// sayaslicer.cpp : définit le point d'entrée de l'application.
//

#include "sayaslicer.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <implot.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <list>

using namespace std;

static int offset = 0;
static int waveformReso = 192;
static double cursorPos = 0.0;
static float bpm = 120.0;
static int snapping = 4;
static double samplesPerSnap = 0.0;

bool OpenAudioFile(sf::SoundBuffer &buffer)
{
    char const* lFilterPatterns[2] = { "*.wav", "*.ogg" };
    char const* selection = tinyfd_openFileDialog( // there is also a wchar_t version
        "Select file", // title
        0, // optional initial directory
        2, // number of filter patterns
        lFilterPatterns, // char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
        NULL, // optional filter description
        0 // forbid multiple selections
    );
    
    if (selection && strlen(selection) > 0) {
        puts(selection);
        bool res = buffer.loadFromFile(selection);
        std::cout << "duration: " << buffer.getDuration().asSeconds() << std::endl;
        std::cout << "channels: " << buffer.getChannelCount() << std::endl;
        std::cout << "sample rate: " << buffer.getSampleRate() << std::endl;
        std::cout << "sample count: " << buffer.getSampleCount() << std::endl;
        return res;
    }
    else return false;
}

void ShowExampleMenuFile(sf::SoundBuffer &buffer)
{
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {
        if (!OpenAudioFile(buffer))
            ;
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile(buffer);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Border);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

void ShowExampleAppMainMenuBar(sf::SoundBuffer &buffer)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowExampleMenuFile(buffer);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

int MeterFormatter(double value, char* buff, int size, void* data) {
    const int unit = *(const int*)data;
    return snprintf(buff, size, "%g/%d", value / 250 + 1, unit);
}

void DisplayWaveform(sf::SoundBuffer& buffer, std::list<double> &markers) {
    if (ImPlot::BeginPlot("##lines", ImVec2(-1, 200), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxisLimits(ImAxis_Y1, -32768, 32768);
        ImPlot::SetupAxisLimits(ImAxis_X1, cursorPos / waveformReso, cursorPos / waveformReso + 2000.0, ImPlotCond_Always);
        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks);
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground);

        //ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 2000, 2000);

        auto sampleCount = buffer.getSampleCount();
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();

        double samplesPerBeat = sampleRate ? 60.0 / (double)bpm * ((double)sampleRate * (double)numChannels) : 1.0;
        samplesPerSnap = samplesPerBeat / (double)snapping * 4.0;
        double lastTick = sampleCount + samplesPerBeat - fmod((sampleCount), samplesPerBeat);
        double remainder = fmod(lastTick, samplesPerBeat * 4);
        int extraBeats = 0;
        if ((int)remainder != 0) {
            lastTick = lastTick - remainder + samplesPerBeat * 4;
        }
        int nbTicksToDraw = (lastTick / samplesPerBeat) * snapping / 4;
        //printf("samplesPerBeat: %f\n", samplesPerBeat);
        //printf("lastTick: %f\n", lastTick);
        //printf("nbTicksToDraw: %d\n", nbTicksToDraw);
        ImPlot::SetupAxisTicks(ImAxis_X1, 0, lastTick / waveformReso, nbTicksToDraw + 1); // Account for last tick

        if (sampleCount > 0) {
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, sampleCount / waveformReso - offset);
            
            auto samples = buffer.getSamples();
            ImPlot::PlotLine("Waveform", samples, sampleCount / waveformReso, 1.0, 0, 0, offset, waveformReso * numChannels); // Buffer stores samples as [channel1_i, channel2_i, channel1_i+1, etc.]
            for (double m : markers) {
                double mTmp = m / waveformReso;
                ImPlot::DragLineX(0, &mTmp, ImVec4(1, 1, 1, 1), 1, ImPlotDragToolFlags_NoInputs);
            }
        }
        ImPlot::EndPlot();
    }
}

double get(std::list<double> _list, int _i) {
    std::list<double>::iterator it = _list.begin();
    for (int i = 0; i < _i; i++) {
        ++it;
    }
    return *it;
}

void PlayKeysound(sf::Sound &sound, sf::SoundBuffer &buffer, sf::SoundBuffer& buffer2, std::list<double> markers) {
    auto samples = buffer.getSamples();
    markers.sort();
    unsigned long keyStart = 0;
    unsigned long keyEnd = 0;
    for (int i = 0; i < markers.size(); i++) {
        if (i + 1.0 >= markers.size()) {
            keyStart = get(markers, i);
            keyEnd = buffer.getSampleCount();
            break;
        }
        else if (cursorPos < get(markers, i + 1)) {
            keyStart = get(markers, i);
            keyEnd = get(markers, i + 1);
            break;
        }
    }
    printf("range start: %d, range end: %d\n", keyStart, keyEnd);
    auto bufsize = keyEnd - keyStart;
    buffer2.loadFromSamples(&samples[keyStart], bufsize, buffer.getChannelCount(), buffer.getSampleRate());
    sound.setBuffer(buffer2);
    sound.play();
    if (keyEnd != buffer.getSampleCount())
        cursorPos = keyEnd;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 421), "sayaslicer");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImPlot::CreateContext();

    sf::SoundBuffer buffer;
    sf::SoundBuffer buffer2;
    sf::Sound sound;
    std::list<double> markers = { 0.0 };

    window.resetGLStates();
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        static bool init = true;
        ImGuiID dock_id_left, dock_id_right;
        if (init) {
            init = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_id_left, &dock_id_right);
            ImGui::DockBuilderDockWindow("Settings", dock_id_left);
            ImGui::DockBuilderDockWindow("Waveform", dock_id_right);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ShowExampleAppMainMenuBar(buffer);

        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);
        ImGui::Begin("Settings");
        {
            ImGui::SeparatorText("General");
            ImGui::DragInt("Offset", &offset, 1, 0, 1000);
            ImGui::DragScalar("Position", ImGuiDataType_Double, &cursorPos, 1, 0, 0);
            ImGui::DragFloat("BPM", &bpm, 1, 10, 10000);
            ImGui::DragInt("Snapping", &snapping, 1, 1, 192);
        }
        ImGui::End();

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
            cursorPos += samplesPerSnap;
        }
        else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && cursorPos > 0.0) {
            if (cursorPos - samplesPerSnap < 0.0)
                cursorPos = 0.0;
            else
                cursorPos -= samplesPerSnap;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && snapping < 192) {
            snapping += 1;
        }
        else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && snapping > 1) {
            snapping -= 1;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
            if (std::find(markers.begin(), markers.end(), cursorPos) != markers.end()) {
                markers.remove(cursorPos);
            }
            else {
                markers.push_back(cursorPos);
            }
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P))) {
            PlayKeysound(sound, buffer, buffer2, markers);
        }

        ImGui::SetNextWindowClass(&window_class);
        ImGui::Begin("Waveform");
        {
            ImGui::SeparatorText("Waveform");
            DisplayWaveform(buffer, markers);

            ImGui::SeparatorText("Markers");
            ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 8);
            if (ImGui::BeginTable("markerstable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, outer_size))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Marker", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                if (markers.size() == 0) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("No markers set...");
                }
                else {
                    markers.sort();
                    double toRemove = -1.0;
                    for (double m : markers) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        char buf[64];
                        sprintf(buf, "%f", m);
                        ImGui::Selectable(buf);
                        if (ImGui::IsItemClicked(0)) {
                            cursorPos = m;
                        }
                        else if (ImGui::IsItemClicked(1)) {
                            toRemove = m;
                        }
                    }
                    if (toRemove != -1) {
                        markers.remove(toRemove);
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
}