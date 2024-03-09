#pragma once

#include "settings.hpp"
#include "utils.hpp"
#include <ImGuiNotify.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>

static const int waveformReso = 192;
static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };

bool OpenAudioFile(sf::SoundBuffer& buffer, SlicerSettings& settings, std::string file = "")
{
    if (file.size() == 0) {
        char const* lFilterPatterns[2] = { "*.wav", "*.ogg" };
        char* s = tinyfd_openFileDialog("Select audio file...", 0, 2, lFilterPatterns, "Audio files (*.wav, *.ogg)", 0);
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
            settings.updateHistory = true;
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

void PlayKeysound(sf::Sound& sound, sf::SoundBuffer& buffer, sf::SoundBuffer& buffer2, SlicerSettings& settings, bool jumpToNext) {
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

int ApplyNoiseGate(std::vector<sf::Int16>& buffer, int threshold, int nbChannels) {
    double limit = pow(10.0, (double)threshold / 20.0) * 0x7fff; // dB to amplitude and multiply with max Int16
    auto result = std::find_if(buffer.rbegin(), buffer.rend(),
        [limit](int i) { return abs(i) > limit; });

    auto pos = std::distance(result, buffer.rend());
    if (pos % nbChannels != 0)
        pos = pos + nbChannels - pos % nbChannels;
    buffer.resize(pos);
    return pos;
}

void ApplyFadeout(std::vector<sf::Int16>& buffer, int fadeTime, unsigned int sampleRate, int nbChannels) {
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

void WriteKeysounds(sf::SoundBuffer& buffer, SlicerSettings& settings) {
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
        std::vector<sf::Int16> newBuf;
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