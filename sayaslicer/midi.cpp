#include "midi.hpp"

using namespace i18n::literals;

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

void LoadMidi(sf::SoundBuffer& buffer, SlicerSettings& settings, std::string file) {
    if (!buffer.getSampleCount()) {
        InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
        return;
    }
    if (file.size() == 0) {
        char const* lFilterPatterns[2] = { "*.mid", "*.midi" };
        auto s = tinyfd_openFileDialog("Open MIDI file...", 0, 2, lFilterPatterns, "MIDI file (*.mid, *.midi)", 0);
        if (s)
            file = s;
        else
            return;
    }
    if (file.size() != 0) {
        auto p = std::filesystem::u8path(file);
        std::ifstream inFile(p, std::ios::binary);
        settings.midiFile.read(inFile);
        if (!settings.midiFile.status()) {
            std::cout << "Error reading MIDI file: " << file << std::endl;
        }
        else {
            settings.openMidiModalTemp = true;
        }
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
        settings.markers.clear(true);
    for (int i = 0; i < settings.midiFile[selectedTrack].size(); i++) {
        if (settings.midiFile[selectedTrack][i].isNoteOn()) {
            auto event = settings.midiFile[selectedTrack][i];
            auto tick = relative ? event.tick * (samplesPerBeat / tpq) : event.seconds * sampleRate * nbChannels;
            if (settings.markers.find(tick) == -1.0) {
                settings.markers.push_back(tick);
            }
        }
    }
    InsertNotification({ ImGuiToastType::Success, 3000, "Successfully imported markers from MIDI file!" });
}

void ShowMidiTrackModal(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (settings.openMidiModalTemp) {
        settings.openMidiModalTemp = false;
        ImGui::OpenPopup("Select MIDI track");
    }
    if (ImGui::BeginPopupModal("Select MIDI track", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
        static std::vector<std::string> choices;
        if (choices.size() == 0) {
            choices.push_back("all_tracks"_t.c_str());
            GetTrackNames(settings.midiFile, choices);
        }
        static int choice = 0;
        static bool useMidiBPM = false;
        static bool clearMarkers = true;
        ImGui::Text("%s:", "select_track_import"_t.c_str());
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
        ImGui::Checkbox("clear_existing_markers"_t.c_str(), &clearMarkers);

        if (ImGui::Button("import"_t.c_str())) {
            ImportMidiMarkers(buffer, settings, choice - 1, useMidiBPM, clearMarkers);
            choice = 0;
            choices.clear();
            settings.midiFile.clear();
            settings.updateHistory = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("cancel"_t.c_str())) {
            choice = 0;
            choices.clear();
            settings.midiFile.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}