// sayaslicer.cpp : définit le point d'entrée de l'application.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#if _WIN32 // Include required for drag and drop on Windows
#include <Windows.h>
LONG_PTR originalSFMLCallback = 0x0;
LONG_PTR originalUserData = 0x0;
#endif

#include "sayaslicer.h"
#include "bmseclipboard.hpp"
#include "theme.hpp"
#include "font.hpp"
#include "custom_widgets.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <implot.h>
#include <clip/clip.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <list>
#include <filesystem>

using namespace std;

static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };

static int offset = 0;
static int waveformReso = 192;
static double cursorPos = 0.0;
static float bpm = 120.0;
static int snapping = 4;
static int startingKeysound = 1;
static bool useBase62 = false;
static int fadeout = 0;
static int selectedGateThreshold = 0;

static double samplesPerSnap = 0.0;
static std::string selectedFile;
static sf::SoundBuffer buffer;

bool OpenAudioFile(sf::SoundBuffer &buffer, std::string file = "")
{
    if (file.size() == 0) {
        char const* lFilterPatterns[2] = { "*.wav", "*.ogg" };
        char* s = tinyfd_openFileDialog( // there is also a wchar_t version
            "Select audio file...", // title
            0, // optional initial directory
            2, // number of filter patterns
            lFilterPatterns, // char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
            NULL, // optional filter description
            0 // forbid multiple selections
        );
        if (s)
            selectedFile = s;
        else
            return false;
    }
    else {
        selectedFile = file;
    }
    
    if (selectedFile.size() > 0) {
        std::cout << "Loading file: " << selectedFile << std::endl;
        bool res = buffer.loadFromFile(selectedFile);
        if (res) {
            std::cout << "Duration: " << buffer.getDuration().asSeconds() << std::endl;
            std::cout << "Channels: " << buffer.getChannelCount() << std::endl;
            std::cout << "Sample rate: " << buffer.getSampleRate() << std::endl;
            std::cout << "Sample count: " << buffer.getSampleCount() << std::endl;
        }
        return res;
    }
    else {
        return false;
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
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoTickLabels);

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
            ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, sampleCount / waveformReso - offset);

            auto samples = buffer.getSamples();
            ImPlot::SetNextLineStyle(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
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

void PlayKeysound(sf::Sound &sound, sf::SoundBuffer &buffer, sf::SoundBuffer& buffer2, std::list<double> markers, bool jumpToNext) {
    if (buffer.getSampleCount() == 0 || markers.size() == 0)
        return;
    auto samples = buffer.getSamples();
    markers.sort();
    unsigned long long keyStart = 0;
    unsigned long long keyEnd = 0;
    unsigned long long offsetSamples = (long long)offset * (long long)waveformReso;
    int i = 0;
    for (; i < markers.size(); i++) {
        if (i + 1.0 >= markers.size()) {
            keyStart = get(markers, i) + offsetSamples;
            keyEnd = buffer.getSampleCount();
            break;
        }
        else if (cursorPos < get(markers, i + 1)) {
            keyStart = get(markers, i) + offsetSamples;
            keyEnd = get(markers, i + 1) + offsetSamples;
            break;
        }
    }
    printf("playing keysound with range start: %llu, range end: %llu\n", keyStart, keyEnd);
    auto bufsize = (keyEnd - keyStart) + 4 - ((keyEnd - keyStart) % 4); // Buffer size must be a multiple of 4
    buffer2.loadFromSamples(&samples[keyStart], bufsize, buffer.getChannelCount(), buffer.getSampleRate());
    sound.setBuffer(buffer2);
    sound.play();
    if (jumpToNext && keyEnd != buffer.getSampleCount())
        cursorPos = get(markers, i + 1) - (double)offsetSamples;
}

int ApplyNoiseGate(vector<sf::Int16>& buffer, int threshold, int nbChannels) {
    double limit = pow(10.0, (double)threshold / 20.0) * 0x7fff; // dB to amplitude and multiply with max Int16
    auto result = std::find_if(buffer.rbegin(), buffer.rend(),
        [limit](int i) { return abs(i) > limit; });

    auto pos = std::distance(result, buffer.rend());
    if (pos % nbChannels != 0)
        pos = pos + nbChannels - pos % nbChannels;
    buffer.resize(pos);
    return pos;
}

void ApplyFadeout(vector<sf::Int16>& buffer, int fadeTime, unsigned int sampleRate, int nbChannels) {
    size_t fadeoutSampleLen = (size_t)(sampleRate * fadeTime / 1000);
    unsigned int startFadeoutSample = buffer.size() <= fadeoutSampleLen * nbChannels ? 0 : buffer.size() - fadeoutSampleLen * nbChannels - 1;
    double volRatio = 0.0;
    for (size_t i = startFadeoutSample; i < buffer.size(); i += nbChannels) {
        for (size_t j = 0; j < nbChannels && i + j < buffer.size(); j++) {
            buffer[i + j] *= (1 - volRatio * volRatio);
        }
        volRatio += 1.0 / fadeoutSampleLen; 
    }
}

void WriteKeysounds(sf::SoundBuffer& buffer, std::list<double> markers) {
    if (selectedFile.size() == 0)
        return;
    auto samples = buffer.getSamples();
    markers.sort();
    unsigned long long keyStart = 0;
    unsigned long long keyEnd = 0;
    unsigned long long offsetSamples = (long long)offset * (long long)waveformReso;
    std::filesystem::path p = selectedFile;
    p.replace_extension("");
    for (int i = 0; i < markers.size(); i++) {
        keyStart = get(markers, i) + offsetSamples;
        if (i + 1.0 >= markers.size()) {
            keyEnd = buffer.getSampleCount();
        }
        else {
            keyEnd = get(markers, i + 1) + offsetSamples;
        }
        printf("exporting keysound with range start: %llu, range end: %llu\n", keyStart, keyEnd);
        auto bufsize = keyEnd - keyStart;
        sf::OutputSoundFile file;
        char filename[4096];
        auto bufOut = &samples[keyStart];
        vector<sf::Int16> newBuf;
        if (selectedGateThreshold != 0 || fadeout != 0) {
            newBuf.insert(newBuf.end(), &bufOut[0], &bufOut[bufsize]);
            if (selectedGateThreshold != 0)
                bufsize = ApplyNoiseGate(newBuf, gateThresholds[selectedGateThreshold], buffer.getChannelCount());
            if (fadeout != 0)
                ApplyFadeout(newBuf, fadeout, buffer.getSampleRate(), buffer.getChannelCount());
            bufOut = &newBuf[0];
        }
        snprintf(filename, 4096, "%s_%03d.wav", p.string().c_str(), i);
        puts(filename);
        if (!file.openFromFile(filename, buffer.getSampleRate(), buffer.getChannelCount())) {
            puts("Error opening file for writing");
        }
        file.write(bufOut, bufsize);
    }
}

double FindInList(std::list<double> markers, double e) {
    for (double m : markers) {
        if (std::abs(m - e) < 0.000001)
            return m;
    }
    return -1.0;
}

void AddMarkersFromBMSEClipboard(BMSEClipboard objs, sf::SoundBuffer& buffer, std::list<double>& markers) {
    if (buffer.getSampleCount() > 0) {
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();
        for (BMSEClipboardObject o : objs.objects) {
            double m = o.toSamplePosition(bpm, sampleRate, numChannels);
            if (FindInList(markers, m) == -1.0) {
                markers.push_back(m);
            }
        }
    }
}

void ProcessBMSEClipboard(sf::SoundBuffer& buffer, std::list<double>& markers) {
    std::string cb;
    clip::get_text(cb);
    BMSEClipboard objs(cb);
    AddMarkersFromBMSEClipboard(objs, buffer, markers);
}

void GenerateBMSEClipboard(sf::SoundBuffer& buffer, std::list<double> markers) {
    if (buffer.getSampleCount() > 0) {
        auto cb = BMSEClipboard::toBMSEClipboardData(markers, bpm, buffer.getSampleRate(), buffer.getChannelCount(), startingKeysound);
        std::cout << cb << std::endl;
        clip::set_text(cb);
    }
}

void ClearAllMarkers(std::list<double>& markers)
{
    markers.clear();
    //markers.push_back(0.0);
}

void ShowMenuFile(sf::SoundBuffer& buffer, std::list<double> &markers, sf::RenderWindow &window)
{
    if (ImGui::MenuItem("Open", "O")) {
        OpenAudioFile(buffer);
    }
    if (ImGui::MenuItem("Export keysounds", "M")) {
        WriteKeysounds(buffer, markers);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        window.close();
    }
}

void ShowMenuEdit(sf::SoundBuffer& buffer,  std::list<double>& markers)
{
    if (ImGui::MenuItem("Copy BMSE clipboard data", "V")) {
        GenerateBMSEClipboard(buffer, markers);
    }
    if (ImGui::MenuItem("Paste BMSE clipboard data", "B")) {
        ProcessBMSEClipboard(buffer, markers);
    }
    if (ImGui::MenuItem("Clear all markers", "C")) {
        ClearAllMarkers(markers);
    }
}

void ShowMainMenuBar(sf::SoundBuffer& buffer, std::list<double> &markers, sf::RenderWindow &window)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile(buffer, markers, window);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ShowMenuEdit(buffer, markers);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

#if _WIN32 // Modified from https://gist.github.com/FRex/3f7b8d1ad1289a2117553ff3702f04af
LRESULT CALLBACK myCallback(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DROPFILES)
    {
        HDROP hdrop = reinterpret_cast<HDROP>(wParam);

        const UINT filescount = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
        if (filescount != 1) {
            std::cout << "Please drag only one file!" << std::endl;
        }
        else {
            const UINT bufsize = DragQueryFile(hdrop, 0, NULL, 0);
            std::string str;
            str.resize(bufsize + 1);
            if (DragQueryFile(hdrop, 0, &str[0], bufsize + 1))
            {
                std::string stdstr;
                sf::Utf8::fromWide(str.begin(), str.end(), std::back_inserter(stdstr));
                OpenAudioFile(buffer, stdstr);
            }
        }
        DragFinish(hdrop);
    }
    return CallWindowProcW(reinterpret_cast<WNDPROC>(originalSFMLCallback), handle, message, wParam, lParam);
}
#endif

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 450), "sayaslicer");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window, false);
    auto &io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(roboto_compressed_data, roboto_compressed_size, 13);
    io.Fonts->Build();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::SFML::UpdateFontTexture();
    SetupImGuiStyle();
    ImPlot::CreateContext();
    sf::SoundBuffer buffer2;
    sf::Sound sound;
    std::list<double> markers = { 0.0 };

#if _WIN32
    HWND handle = window.getSystemHandle();
    DragAcceptFiles(handle, TRUE);
    originalSFMLCallback = SetWindowLongPtrW(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(myCallback));
#endif

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

        ShowMainMenuBar(buffer, markers, window);

        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);
        if (ImGui::Begin("Settings"))
        {
            ImGui::SeparatorText("General");
            ImGui::DragInt("Offset", &offset, 1, 0, 1000);
            ImGui::DragScalar("Position", ImGuiDataType_Double, &cursorPos, 1, 0, 0);
            ImGui::DragFloat("BPM", &bpm, 1, 10, 10000);
            ImGui::DragInt("Snapping", &snapping, 1, 1, 192);
            int base = useBase62 ? 62 : 36;
            int maxKeysound = base * base - 1;
            if (startingKeysound > maxKeysound)
                startingKeysound = maxKeysound;
            DragIntCustomBase("Starting key", &startingKeysound, 1, 1, maxKeysound, base);
            ImGui::SetItemTooltip("Decimal value: %d", startingKeysound);
            ImGui::Checkbox("Enable base-62", &useBase62);

            ImGui::SeparatorText("Export settings");
            char thres[64];
            if (selectedGateThreshold == 0)
                snprintf(thres, 64, "Disabled");
            else
                snprintf(thres, 64, "%ddB", gateThresholds[selectedGateThreshold]);
            const char* combo_preview_value = thres;
            if (ImGui::BeginCombo("Noise gate", combo_preview_value, 0))
            {
                for (int n = 0; n < IM_ARRAYSIZE(gateThresholds); n++)
                {
                    if (n == 0)
                        snprintf(thres, 64, "Disabled");
                    else
                        snprintf(thres, 64, "%ddB", gateThresholds[n]);
                    const bool is_selected = (selectedGateThreshold == n);
                    if (ImGui::Selectable(thres, is_selected))
                        selectedGateThreshold = n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::DragInt("Fadeout", &fadeout, 1, 0, 1000, "%dms");

            ImGui::SeparatorText("Process");
            if (ImGui::Button("Export keysounds", ImVec2(-FLT_MIN, 0.0f))) {
                WriteKeysounds(buffer, markers);
            }
        }
        ImGui::End();

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
            if (cursorPos + samplesPerSnap < buffer.getSampleCount())
                cursorPos += samplesPerSnap;
            if (sound.getStatus() == sf::Sound::Playing) {
                sound.stop();
            }
        }
        else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && cursorPos > 0.0) {
            if (cursorPos - samplesPerSnap < 0.0)
                cursorPos = 0.0;
            else
                cursorPos -= samplesPerSnap;
            if (sound.getStatus() == sf::Sound::Playing) {
                sound.stop();
            }
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && snapping < 192) {
            snapping += 1;
        }
        else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && snapping > 1) {
            snapping -= 1;
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
            double e = FindInList(markers, cursorPos);
            if (e != -1.0) {
                markers.remove(e);
            }
            else {
                markers.push_back(cursorPos);
            }
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
            PlayKeysound(sound, buffer, buffer2, markers, false);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P))) {
            PlayKeysound(sound, buffer, buffer2, markers, true);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_M), false)) {
            WriteKeysounds(buffer, markers);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false)) {
            OpenAudioFile(buffer);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_B), false)) {
            ProcessBMSEClipboard(buffer, markers);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), false)) {
            GenerateBMSEClipboard(buffer, markers);
        }
        if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false)) {
            ClearAllMarkers(markers);
        }

        ImGui::SetNextWindowClass(&window_class);
        if (ImGui::Begin("Waveform"))
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
                        snprintf(buf, 64, "%f", m);
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

#if _WIN32
    DragAcceptFiles(handle, FALSE);
#endif

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
}