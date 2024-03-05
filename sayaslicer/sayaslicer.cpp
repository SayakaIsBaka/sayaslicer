// sayaslicer.cpp : définit le point d'entrée de l'application.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#if _WIN32 // Include required headers and pointers for drag and drop on Windows
#define NOMINMAX
#include <Windows.h>
LONG_PTR originalSFMLCallback = 0x0;
LONG_PTR originalUserData = 0x0;
LONG_PTR bufferPtr = 0x0;
LONG_PTR settingsPtr = 0x0;
#endif

#include "sayaslicer.h"
#include "bmseclipboard.hpp"
#include "theme.hpp"
#include "font.hpp"
#include "custom_widgets.hpp"
#include "settings.hpp"
#include <ImGuiNotify.hpp>
#include <IconsFontAwesome6.h>
#include <fa_solid_900.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <implot.h>
#include <clip/clip.h>
#include <cereal/archives/binary.hpp>
#include <midifile/include/MidiFile.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <list>
#include <filesystem>
#include <fstream>

using namespace std;

static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };
static const int waveformReso = 192;

bool OpenAudioFile(sf::SoundBuffer &buffer, SlicerSettings &settings, std::string file = "")
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
            settings.selectedFile = s;
        else
            return false;
    }
    else {
        settings.selectedFile = file;
    }
    
    if (settings.selectedFile.size() > 0) {
        std::cout << "Loading file: " << settings.selectedFile << std::endl;
        bool res = buffer.loadFromFile(settings.selectedFile);
        if (res) {
            std::cout << "Duration: " << buffer.getDuration().asSeconds() << std::endl;
            std::cout << "Channels: " << buffer.getChannelCount() << std::endl;
            std::cout << "Sample rate: " << buffer.getSampleRate() << std::endl;
            std::cout << "Sample count: " << buffer.getSampleCount() << std::endl;
            settings.cursorPos = 0.0; // Reset cursor position to avoid crashing
            if (settings.markers.size() == 0)
                settings.markers.push_back(0.0);
            ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Opened file:\n%s", settings.selectedFile.c_str() });
        }
        else {
            ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Selected file isn't supported!" });
        }
        return res;
    }
    else {
        return false;
    }
}

int MeterFormatter(double value, char* buff, int size, void* data) {
    SlicerSettings settings = *(SlicerSettings*)data;
    const double reducedSamplesPerMeasure = settings.samplesPerSnap * settings.snapping / waveformReso;
    double tmp = fmod(value, reducedSamplesPerMeasure);
    double delta = 0.0001;
    if (tmp <= delta || tmp >= reducedSamplesPerMeasure - delta)
        return snprintf(buff, size, "%d", (int)round(value / reducedSamplesPerMeasure));
    else
        return snprintf(buff, size, "");
}

void DisplayWaveform(sf::SoundBuffer& buffer, SlicerSettings &settings) {
    double maxDisplayRange = 1500.0;
    double leftMargin = (buffer.getSampleCount() < 400 * waveformReso ? 0 : 400) * waveformReso;

    if (ImPlot::BeginPlot("##lines", ImVec2(-1, 200), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoLegend)) {
        double plotStart = (settings.cursorPos - leftMargin) / waveformReso;
        double plotEnd = plotStart + maxDisplayRange;
        ImPlot::SetupAxisLinks(ImAxis_X1, &plotStart, &plotEnd);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -32768, 32768);

        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks);
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoTickLabels);

        auto sampleCount = buffer.getSampleCount();
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();

        double samplesPerBeat = sampleRate ? 60.0 / (double)settings.bpm * ((double)sampleRate * (double)numChannels) : 1.0;
        settings.samplesPerSnap = samplesPerBeat / (double)settings.snapping * 4.0;
        int beatsToDisplayLeft = (leftMargin / samplesPerBeat) - (int)(leftMargin / samplesPerBeat) % 4 + 4;

        double leftPartSamples = fmod(settings.cursorPos, samplesPerBeat * beatsToDisplayLeft);
        double fixMod = settings.cursorPos > leftMargin && leftPartSamples < leftMargin ? samplesPerBeat * beatsToDisplayLeft : 0.0;
        double startTick = settings.cursorPos - leftPartSamples - fixMod;
        double lastTick = settings.cursorPos + maxDisplayRange * waveformReso;

        std::vector<double> ticks;
        if (sampleCount > 0) {
            for (double i = startTick; i < lastTick; i += settings.samplesPerSnap)
                ticks.push_back(i / waveformReso);
        }
        else {
            ticks.push_back(0.0);
        }
        int nbTicksToDraw = ticks.size();

        ImPlot::SetupAxisFormat(ImAxis_X1, MeterFormatter, &settings);
        ImPlot::SetupAxisTicks(ImAxis_X1, ticks.data(), nbTicksToDraw);

        if (sampleCount > 0) {
            ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -leftMargin, sampleCount / waveformReso - settings.offset);

            // Draw barlines
            for (double j = 0; j < lastTick; j += samplesPerBeat * 4) {
                if (j < settings.cursorPos - leftMargin)
                    continue;
                double tmp = j / waveformReso;
                ImPlot::DragLineX(555, &tmp, ImVec4(1, 1, 1, 0.25), 0.1, ImPlotDragToolFlags_NoInputs);
            }

            // Draw beat lines if snapping is a multiple of 4
            if (settings.snapping % 4 == 0) {
                for (double j = 0; j < lastTick; j += samplesPerBeat) {
                    if (j < settings.cursorPos - leftMargin)
                        continue;
                    double tmp = j / waveformReso;
                    ImPlot::DragLineX(555, &tmp, ImVec4(1, 1, 1, 0.075), 0.05, ImPlotDragToolFlags_NoInputs);
                }
            }

            auto samples = buffer.getSamples();
            ImPlot::SetNextLineStyle(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
            size_t arrLen = maxDisplayRange;
            if (lastTick > sampleCount)
                arrLen = (sampleCount - settings.cursorPos + leftMargin) / waveformReso;
            size_t arrayOffset = (std::max((long long)(settings.cursorPos - leftMargin), (long long)0) / (waveformReso * numChannels)) * (waveformReso * numChannels);
            ImPlot::PlotLine("Waveform", &samples[arrayOffset], arrLen, 1.0, arrayOffset / (waveformReso), 0, settings.offset, waveformReso * numChannels); // Buffer stores samples as [channel1_i, channel2_i, channel1_i+1, etc.]
            
            // Display cursor
            double curDisplayPos = settings.cursorPos / waveformReso;
            ImPlot::DragLineX(555, &curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotLines), 0.5, ImPlotDragToolFlags_NoInputs);
            ImPlot::TagX(curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotLines), true);
            
            for (auto m : settings.markers) {
                if (m.position < settings.cursorPos - leftMargin)
                    continue;
                double mTmp = m.position / waveformReso;
                ImPlot::DragLineX(0, &mTmp, ImVec4(1, 1, 1, 1), 1, ImPlotDragToolFlags_NoInputs);
            }
        }
        ImPlot::EndPlot();
        settings.cursorPos = plotStart * waveformReso + leftMargin;
    }
}

void PlayKeysound(sf::Sound &sound, sf::SoundBuffer &buffer, sf::SoundBuffer& buffer2, SlicerSettings &settings, bool jumpToNext) {
    if (buffer.getSampleCount() == 0 || settings.markers.size() == 0)
        return;
    auto samples = buffer.getSamples();
    settings.markers.sort();
    unsigned long long keyStart = 0;
    unsigned long long keyEnd = 0;
    unsigned long long offsetSamples = (long long)settings.offset * (long long)waveformReso;
    int i = 0;
    for (; i < settings.markers.size(); i++) {
        if (i + 1.0 >= settings.markers.size()) {
            keyStart = settings.markers.get(i).position + offsetSamples;
            keyEnd = buffer.getSampleCount();
            break;
        }
        else if ((float)settings.cursorPos < (float)settings.markers.get(i + 1).position) {
            keyStart = settings.markers.get(i).position + offsetSamples;
            keyEnd = settings.markers.get(i + 1).position + offsetSamples;
            break;
        }
    }
    printf("playing keysound with range start: %llu, range end: %llu\n", keyStart, keyEnd);
    auto bufsize = (keyEnd - keyStart) + 4 - ((keyEnd - keyStart) % 4); // Buffer size must be a multiple of 4
    buffer2.loadFromSamples(&samples[keyStart], bufsize, buffer.getChannelCount(), buffer.getSampleRate());
    sound.setBuffer(buffer2);
    sound.play();
    if (jumpToNext && keyEnd != buffer.getSampleCount())
        settings.cursorPos = settings.markers.get(i + 1).position - (double)offsetSamples;
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

std::string GetTempMarkerName(std::string filename, size_t idx) {
    std::string suffix = "_";
    switch (idx > 0 ? (int)log10((double)idx) + 1 : 1) { // Get number of digits
    case 1:
        suffix = "_00";
        break;
    case 2:
        suffix = "_0";
        break;
    }
    return filename + suffix + std::to_string(idx) + ".wav";
}

void WriteKeysounds(sf::SoundBuffer& buffer, SlicerSettings &settings) {
    if (settings.selectedFile.size() == 0) {
        ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
        return;
    }
    auto samples = buffer.getSamples();
    settings.markers.sort();
    unsigned long long keyStart = 0;
    unsigned long long keyEnd = 0;
    unsigned long long offsetSamples = (long long)settings.offset * (long long)waveformReso;
    std::filesystem::path p = settings.selectedFile;
    auto origFilename = p.filename().replace_extension().string();
    auto folder = p.remove_filename();
    for (int i = 0; i < settings.markers.size(); i++) {
        Marker m = settings.markers.get(i);
        keyStart = m.position + offsetSamples;
        if (i + 1.0 >= settings.markers.size()) {
            keyEnd = buffer.getSampleCount();
        }
        else {
            keyEnd = settings.markers.get(i + 1).position + offsetSamples;
        }
        printf("exporting keysound with range start: %llu, range end: %llu\n", keyStart, keyEnd);
        auto bufsize = keyEnd - keyStart;
        sf::OutputSoundFile file;

        if (keyStart > buffer.getSampleCount())
            continue;

        auto bufOut = &samples[keyStart];
        vector<sf::Int16> newBuf;
        if (settings.selectedGateThreshold != 0 || settings.fadeout != 0) {
            newBuf.insert(newBuf.end(), &bufOut[0], &bufOut[bufsize]);
            if (settings.selectedGateThreshold != 0)
                bufsize = ApplyNoiseGate(newBuf, gateThresholds[settings.selectedGateThreshold], buffer.getChannelCount());
            if (settings.fadeout != 0)
                ApplyFadeout(newBuf, settings.fadeout, buffer.getSampleRate(), buffer.getChannelCount());
            bufOut = &newBuf[0];
        }

        std::string filename;
        if (m.name.empty())
            filename = folder.string() + GetTempMarkerName(origFilename, i);
        else
            filename = folder.string() + m.name;
        
        std::cout << filename << std::endl;
        if (!file.openFromFile(filename, buffer.getSampleRate(), buffer.getChannelCount())) {
            puts("Error opening file for writing");
        }
        file.write(bufOut, bufsize);
    }
    ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Exported keysounds to the following folder:\n%s", p.parent_path().string().c_str() });
}

void AddMarkersFromBMSEClipboard(BMSEClipboard objs, sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (buffer.getSampleCount() > 0) {
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();
        for (BMSEClipboardObject o : objs.objects) {
            double m = o.toSamplePosition(settings.bpm, sampleRate, numChannels);
            if (settings.markers.find(m) == -1.0) {
                settings.markers.push_back(m);
            }
        }
        ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully imported markers from the clipboard!" });
    }
    else {
        ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
    }
}

void ProcessBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    std::string cb;
    clip::get_text(cb);
    BMSEClipboard objs(cb);
    if (!objs.objects.empty())
        AddMarkersFromBMSEClipboard(objs, buffer, settings);
    else
        ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Clipboard does not contain any BMSE data!" });
}

void GenerateBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings settings) {
    if (buffer.getSampleCount() > 0) {
        auto cb = BMSEClipboard::toBMSEClipboardData(settings.markers, settings.bpm, buffer.getSampleRate(), buffer.getChannelCount(), settings.startingKeysound);
        std::cout << cb << std::endl;
        clip::set_text(cb);
        ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Copied markers as BMSE clipboard data!" });
    }
    else {
        ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
    }
}

void SaveProject(SlicerSettings settings) {
    char const* aFilterPatterns[1] = { "*.syp" };
    char* s = tinyfd_saveFileDialog("Save project file...", 0, 1, aFilterPatterns, "sayaslicer Project (*.syp)");
    if (s) {
        std::ofstream outFile(s);
        cereal::BinaryOutputArchive oarchive(outFile);
        oarchive(settings);
        ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Saved project to:\n%s", s });
    }
}

void OpenProject(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    char const* lFilterPatterns[1] = { "*.syp" };
    char* s = tinyfd_openFileDialog("Open project file...", 0, 1, lFilterPatterns, "sayaslicer Project (*.syp)", 0);
    if (s) {
        std::ifstream inFile(s);
        cereal::BinaryInputArchive iarchive(inFile);
        iarchive(settings);
        if (std::filesystem::exists(settings.selectedFile)) {
            OpenAudioFile(buffer, settings, settings.selectedFile);
        }
        else {
            OpenAudioFile(buffer, settings);
        }
        ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Opened project:\n%s", s });
    }
}

void GetTrackNames(smf::MidiFile midifile, std::vector<std::string>& out) {
    int tracks = midifile.getTrackCount();
    for (int i = 0; i < tracks; i++) {
        bool foundName = false;
        for (int j = 0; j < midifile[i].getEventCount(); j++) {
            if (midifile[i][j].isTrackName()) {
                std::string trackName = midifile[i][j].getMetaContent();
                if (!trackName.empty()) {
                    out.push_back(trackName);
                    foundName = true;
                    break;
                }
            }
        }
        if (!foundName)
            out.push_back("Track " + std::to_string(i));
    }
}

void LoadMidi(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (!buffer.getSampleCount()) {
        ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
        return;
    }
    char const* lFilterPatterns[2] = { "*.mid", "*.midi"};
    char* s = tinyfd_openFileDialog("Open MIDI file...", 0, 2, lFilterPatterns, "MIDI file (*.mid, *.midi)", 0);
    if (s) {
        settings.midiFile.read(s);
        if (!settings.midiFile.status()) {
            std::cout << "Error reading MIDI file: " << s << std::endl;
        }
        else {
            ImGuiID popup_id = ImHashStr("MidiPopup");
            ImGui::PushOverrideID(popup_id);
            ImGui::OpenPopup("Select MIDI track");
            ImGui::PopID();
        }
    }
}

void ImportNamesFromMid2Bms(SlicerSettings& settings) {
    char const* lFilterPatterns[1] = { "*.txt" };
    char* s = tinyfd_openFileDialog("Open renamer array file...", "text5_renamer_array.txt", 1, lFilterPatterns, "Text file (*.txt)", 0);
    if (s) {
        std::ifstream f(s);
        std::vector<std::string> names;
        if (f.is_open() && f.good()) {
            std::string line;
            for (int i = 0; i < 3 && f.good(); i++) {
                f >> line;
            }
            while (f.good()) {
                f >> line;
                if (line == "//") {
                    for (int i = 0; i < 3 && f.good(); i++) {
                        f >> line;
                    }
                }
                else {
                    names.push_back(line);
                }
            }
        }
        if (settings.markers.importNames(names)) {
            ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully imported marker names!" });
        }
        else {
            ImGui::InsertNotification({ ImGuiToastType::Warning, 3000, "Imported marker names but the number of names did not perfectly match the number of markers" });
        }
    }
}

void ShowMenuFile(sf::SoundBuffer& buffer, SlicerSettings &settings, sf::RenderWindow &window)
{
    if (ImGui::MenuItem("Open project", "Ctrl+O")) {
        OpenProject(buffer, settings);
    }
    if (ImGui::MenuItem("Save project", "Ctrl+S")) {
        SaveProject(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Import audio file", "O")) {
        OpenAudioFile(buffer, settings);
    }
    if (ImGui::MenuItem("Export keysounds", "M")) {
        WriteKeysounds(buffer, settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        window.close();
    }
}

void ShowMenuEdit(sf::SoundBuffer& buffer, SlicerSettings& settings)
{
    if (ImGui::MenuItem("Import slices from MIDI")) {
        LoadMidi(buffer, settings);
    }
    if (ImGui::MenuItem("Import Mid2BMS renamer file")) {
        ImportNamesFromMid2Bms(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Copy BMSE clipboard data", "V")) {
        GenerateBMSEClipboard(buffer, settings);
    }
    if (ImGui::MenuItem("Paste BMSE clipboard data", "B")) {
        ProcessBMSEClipboard(buffer, settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Clear all markers", "C")) {
        settings.markers.clear();
    }
}

void ShowMainMenuBar(sf::SoundBuffer& buffer, SlicerSettings &settings, sf::RenderWindow &window)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile(buffer, settings, window);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ShowMenuEdit(buffer, settings);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

bool HasBPMData(SlicerSettings& settings, int selectedTrack) {
    for (int i = 0; i < settings.midiFile[selectedTrack].size(); i++) {
        if (settings.midiFile[selectedTrack][i].isTempo())
            return true;
    }
    return false;
}

void ImportMidiMarkers(sf::SoundBuffer& buffer, SlicerSettings& settings, int track, bool useMidiBPM, bool clearMarkers) {
    int selectedTrack = 0;
    if (track == -1) {
        settings.midiFile.joinTracks();
    }
    else {
        selectedTrack = track;
    }

    auto samplesPerBeat = settings.samplesPerSnap * settings.snapping / 4.0;
    auto tpq = settings.midiFile.getTicksPerQuarterNote();
    auto sampleRate = buffer.getSampleRate();
    auto nbChannels = buffer.getChannelCount();
    settings.midiFile.absoluteTicks();

    bool relative = !useMidiBPM || !HasBPMData(settings, 0);
    if (!relative) {
        settings.midiFile.doTimeAnalysis();
    }
    if (clearMarkers)
        settings.markers.clear();
    for (int i = 0; i < settings.midiFile[selectedTrack].size(); i++) {
        if (settings.midiFile[selectedTrack][i].isNoteOn()) {
            auto event = settings.midiFile[selectedTrack][i];
            auto tick = relative ? event.tick * (samplesPerBeat / tpq) : event.seconds * sampleRate * nbChannels;
            if (settings.markers.find(tick) == -1.0) {
                    settings.markers.push_back(tick);
            }
        }
    }
    ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully imported markers from MIDI file!" });
}

void ShowMidiTrackModal(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    ImGuiID popup_id = ImHashStr("MidiPopup");
    ImGui::PushOverrideID(popup_id);
    if (ImGui::BeginPopupModal("Select MIDI track", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
        static std::vector<std::string> choices;
        if (choices.size() == 0) {
            choices.push_back("All tracks");
            GetTrackNames(settings.midiFile, choices);
        }
        static int choice = 0;
        static bool useMidiBPM = false;
        static bool clearMarkers = true;
        ImGui::Text("Select the track to import:");
        const char* combo_preview_value = choices[choice].c_str();
        if (ImGui::BeginCombo("##miditrack", combo_preview_value)) {
            for (int n = 0; n < choices.size(); n++)
            {
                const bool is_selected = (choice == n);
                if (ImGui::Selectable(choices[n].c_str(), is_selected))
                    choice = n;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        //ImGui::Checkbox("Use BPM from the MIDI file", &useMidiBPM);
        ImGui::Checkbox("Clear existing markers", &clearMarkers);

        if (ImGui::Button("Import")) {
            ImportMidiMarkers(buffer, settings, choice - 1, useMidiBPM, clearMarkers);
            choice = 0;
            choices.clear();
            settings.midiFile.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            choice = 0;
            choices.clear();
            settings.midiFile.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void SetupFonts(ImGuiIO& io) {
    float mainFontSize = 13.0f;
    float iconFontSize = mainFontSize * 2.0f / 3.0f;

    io.Fonts->AddFontFromMemoryCompressedTTF(roboto_compressed_data, roboto_compressed_size, mainFontSize);

    static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);

    io.Fonts->Build();
    ImGui::SFML::UpdateFontTexture();
}

void SetupDock() {
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
}

void ShowSettingsPanel(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Settings"))
    {
        double minPos = 0;
        double maxPos = buffer.getSampleCount();

        ImGui::SeparatorText("General");
        ImGui::DragInt("Offset", &settings.offset, 1, 0, 1000);
        ImGui::DragScalar("Position", ImGuiDataType_Double, &settings.cursorPos, 1, &minPos, &maxPos);
        ImGui::DragFloat("BPM", &settings.bpm, 1, 10, 10000);
        ImGui::DragInt("Snapping", &settings.snapping, 1, 1, 192);
        int base = settings.useBase62 ? 62 : 36;
        int maxKeysound = base * base - 1;
        if (settings.startingKeysound > maxKeysound)
            settings.startingKeysound = maxKeysound;
        DragIntCustomBase("Starting key", &settings.startingKeysound, 1, 1, maxKeysound, base);
        ImGui::SetItemTooltip("Decimal value: %d", settings.startingKeysound);
        ImGui::Checkbox("Enable base-62", &settings.useBase62);

        ImGui::SeparatorText("Export settings");
        char thres[64];
        if (settings.selectedGateThreshold == 0)
            snprintf(thres, 64, "Disabled");
        else
            snprintf(thres, 64, "%ddB", gateThresholds[settings.selectedGateThreshold]);
        const char* combo_preview_value = thres;
        if (ImGui::BeginCombo("Noise gate", combo_preview_value, 0))
        {
            for (int n = 0; n < IM_ARRAYSIZE(gateThresholds); n++)
            {
                if (n == 0)
                    snprintf(thres, 64, "Disabled");
                else
                    snprintf(thres, 64, "%ddB", gateThresholds[n]);
                const bool is_selected = (settings.selectedGateThreshold == n);
                if (ImGui::Selectable(thres, is_selected))
                    settings.selectedGateThreshold = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::DragInt("Fadeout", &settings.fadeout, 1, 0, 1000, "%dms");

        ImGui::SeparatorText("Process");
        if (ImGui::Button("Export keysounds", ImVec2(-FLT_MIN, 0.0f))) {
            WriteKeysounds(buffer, settings);
        }
    }
    ImGui::End();
}

void ProcessShortcuts(ImGuiIO& io, sf::SoundBuffer& buffer, sf::SoundBuffer& buffer2, sf::Sound& sound, SlicerSettings& settings) {
    if (!io.WantTextInput && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false)) {
        OpenProject(buffer, settings);
        io.ClearInputKeys(); // Flush Ctrl key (it gets stuck otherwise)
    }
    if (!io.WantTextInput && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S), false)) {
        SaveProject(settings);
        io.ClearInputKeys();
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
        if (settings.cursorPos + settings.samplesPerSnap < buffer.getSampleCount()) {
            if (io.KeyShift) {
                settings.cursorPos += settings.samplesPerSnap - fmod(settings.cursorPos, settings.samplesPerSnap);
            }
            else {
                settings.cursorPos += settings.samplesPerSnap;
            }
        }
        if (sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }
    }
    else if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && settings.cursorPos > 0.0) {
        if (settings.cursorPos - settings.samplesPerSnap < 0.0)
            settings.cursorPos = 0.0;
        else {
            double diff = fmod(settings.cursorPos, settings.samplesPerSnap);
            if (io.KeyShift && diff != 0.0) {
                settings.cursorPos -= diff;
            }
            else {
                settings.cursorPos -= settings.samplesPerSnap;
            }
        }
        if (sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && settings.snapping < 192) {
        settings.snapping += 1;
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && settings.snapping > 1) {
        settings.snapping -= 1;
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
        double e = settings.markers.find(settings.cursorPos);
        if (e != -1.0) {
            settings.markers.remove(e);
        }
        else {
            settings.markers.push_back(settings.cursorPos);
        }
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
        PlayKeysound(sound, buffer, buffer2, settings, false);
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P))) {
        PlayKeysound(sound, buffer, buffer2, settings, true);
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_M), false)) {
        WriteKeysounds(buffer, settings);
    }
    if (!io.WantTextInput && !io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false)) {
        OpenAudioFile(buffer, settings);
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_B), false)) {
        ProcessBMSEClipboard(buffer, settings);
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), false)) {
        GenerateBMSEClipboard(buffer, settings);
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false)) {
        settings.markers.clear();
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home))) {
        settings.cursorPos = 0.0;
    }
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End))) {
        size_t endPos = settings.samplesPerSnap * (int)(buffer.getSampleCount() / settings.samplesPerSnap);
        settings.cursorPos = endPos - settings.samplesPerSnap * (endPos > 0 && (size_t)(endPos - buffer.getSampleCount()) == 0);
    }
}

void DisplayMarkersTable(SlicerSettings& settings) {
    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 8);
    if (ImGui::BeginTable("markerstable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, outer_size))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        if (settings.markers.size() == 0) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("No markers set...");
        }
        else {
            settings.markers.sort();
            double toRemove = -1.0;
            std::filesystem::path p = settings.selectedFile;
            auto filename = p.filename().replace_extension().string();
            size_t idx = 0;
            for (auto& m : settings.markers) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                char buf[64];
                snprintf(buf, 64, "%f", m.position);
                ImGui::Selectable(buf);
                if (ImGui::IsItemClicked(0)) {
                    settings.cursorPos = m.position;
                }
                else if (ImGui::IsItemClicked(1)) {
                    toRemove = m.position;
                }
                ImGui::TableNextColumn();

                if (m.name.size() == 0) {
                    auto tmpName = GetTempMarkerName(filename, idx);
                    SelectableInput(std::to_string(idx).c_str(), false, ImGuiSelectableFlags_None, &m.name[0], 4096, &tmpName[0]);
                }
                else {
                    SelectableInput(std::to_string(idx).c_str(), false, ImGuiSelectableFlags_None, &m.name[0], 4096);
                }
                m.name = &m.name[0]; // Awesome hack to update the string structure with the proper length and everything
                idx++;
            }
            if (toRemove != -1) {
                settings.markers.remove(toRemove);
            }
        }
        ImGui::EndTable();
    }
}

void ShowWaveform(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Waveform"))
    {
        ImGui::SeparatorText("Waveform");
        DisplayWaveform(buffer, settings);

        ImGui::SeparatorText("Markers");
        DisplayMarkersTable(settings);
    }
    ImGui::End();
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
                OpenAudioFile(*(sf::SoundBuffer*)bufferPtr, *(SlicerSettings*)settingsPtr, stdstr);
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

    SetupFonts(io);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    SetupImGuiStyle();
    ImPlot::CreateContext();

    sf::SoundBuffer buffer;
    sf::SoundBuffer buffer2; // For keysound playing
    sf::Sound sound;
    SlicerSettings settings;

#if _WIN32
    HWND handle = window.getSystemHandle();
    DragAcceptFiles(handle, TRUE);
    originalSFMLCallback = SetWindowLongPtrW(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(myCallback));
    bufferPtr = (LONG_PTR)&buffer;
    settingsPtr = (LONG_PTR)&settings;
#endif

    window.resetGLStates();
    sf::Clock deltaClock;

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        SetupDock();
        ShowMainMenuBar(buffer, settings, window);

        ImGui::SetNextWindowClass(&window_class);
        ShowSettingsPanel(buffer, settings);

        ProcessShortcuts(io, buffer, buffer2, sound, settings);

        ImGui::SetNextWindowClass(&window_class);
        ShowWaveform(buffer, settings);

        ShowMidiTrackModal(buffer, settings);

        ImGui::RenderNotifications();

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