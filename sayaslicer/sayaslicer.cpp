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

using namespace std;

static int offset = 0;
static int waveformReso = 192;
static int cursorPos = 0;
static int bpm = 120;
static int snapping = 4;

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

void DisplayWaveform(sf::SoundBuffer &buffer) {
    static double x1 = 0.2;
    static double x2 = 0.8;
    static ImPlotDragToolFlags flags = ImPlotDragToolFlags_None;
    ImGui::CheckboxFlags("NoCursors", (unsigned int*)&flags, ImPlotDragToolFlags_NoCursors); ImGui::SameLine();
    ImGui::CheckboxFlags("NoFit", (unsigned int*)&flags, ImPlotDragToolFlags_NoFit); ImGui::SameLine();
    ImGui::CheckboxFlags("NoInput", (unsigned int*)&flags, ImPlotDragToolFlags_NoInputs);
    if (ImPlot::BeginPlot("##lines", ImVec2(-1, 0), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxisLimits(ImAxis_Y1, -32768, 32768);
        ImPlot::SetupAxisLimits(ImAxis_X1, cursorPos, cursorPos + 2000, ImPlotCond_Always);
        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock);
        
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 2000, 2000);

        auto sampleCount = buffer.getSampleCount();
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();

        double samplesPerBeat = sampleRate ? 60.0 / bpm * ((double)sampleRate / (double)waveformReso * (double)numChannels) : 1.0;
        double samplesPerSnap = samplesPerBeat / snapping / 4.0;
        double lastTick = sampleCount / waveformReso + samplesPerBeat - fmod((sampleCount / waveformReso), samplesPerBeat);
        double nbTicksToDraw = (lastTick / samplesPerBeat) * snapping / 4.0;
        ImPlot::SetupAxisTicks(ImAxis_X1, 0, lastTick, nbTicksToDraw + 1.0); // Account for last tick

        if (sampleCount > 0) {
            
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, sampleCount / waveformReso);
            
            auto samples = buffer.getSamples();
            ImPlot::PlotLine("Waveform", samples, sampleCount / waveformReso, 1.0, 0, 0, offset, waveformReso * numChannels); // Buffer stores samples as [channel1_i, channel2_i, channel1_i+1, etc.]
            ImPlot::DragLineX(0, &x1, ImVec4(1, 1, 1, 1), 1, flags);
            ImPlot::DragLineX(1, &x2, ImVec4(1, 1, 1, 1), 1, flags);
        }
        ImPlot::EndPlot();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 400), "sayaslicer");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImPlot::CreateContext();

    sf::SoundBuffer buffer;

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
            ImGui::DragInt("Position", &cursorPos, 1, 0, INFINITY);
            ImGui::DragInt("BPM", &bpm, 1, 10, 10000);
            ImGui::DragInt("Snapping", &snapping, 1, 1, 192);
        }
        ImGui::End();

        ImGui::SetNextWindowClass(&window_class);
        ImGui::Begin("Waveform");
        {
            DisplayWaveform(buffer);
        }
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
}