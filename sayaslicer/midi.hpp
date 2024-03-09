#pragma once

#include "settings.hpp"
#include <midifile/include/MidiFile.h>
#include <imgui.h>
#include <ImGuiNotify.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>

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
    char const* lFilterPatterns[2] = { "*.mid", "*.midi" };
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
            settings.updateHistory = true;
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