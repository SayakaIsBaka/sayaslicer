#include "audio.hpp"

using namespace i18n::literals;

bool OpenAudioFile(SoundBuffer& buffer, SlicerSettings& settings, std::string file)
{
    auto prevFile = settings.selectedFile;
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

        std::vector<char> bTmp;
        int res = buffer.loadFromFile(settings.selectedFile);
        if (res == -1) {
            InsertNotification({ ImGuiToastType::Error, 3000, "file_doesnt_exist"_t.c_str() });
            return false;
        }

        if (res == 0) {
            std::cout << "Duration: " << buffer.getDuration() << std::endl;
            std::cout << "Channels: " << buffer.getChannelCount() << std::endl;
            std::cout << "Sample rate: " << buffer.getSampleRate() << std::endl;
            std::cout << "Sample count: " << buffer.getSampleCount() << std::endl;
            settings.cursorPos = 0.0; // Reset cursor position to avoid crashing
            if (settings.markers.size() == 0)
                settings.markers.push_back(0.0);
            if (settings.prefs.detectStartingKey)
                GetStartingKeysoundFromBMS(settings);
            settings.updateHistory = true;
            InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "opened_file"_t.c_str(), settings.selectedFile.c_str() });
        }
        else {
            settings.selectedFile = prevFile;
            InsertNotification({ ImGuiToastType::Error, 3000, "selected_file_not_supported"_t.c_str() });
        }
        return res;
    }
    else {
        return false;
    }
}

void PlayKeysound(SoundBuffer& buffer, SlicerSettings& settings, bool jumpToNext) {
    if (buffer.getSampleCount() == 0 || settings.markers.size() == 0)
        return;
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
    keyStart = keyStart - keyStart % buffer.getChannelCount();
    keyEnd = keyEnd - keyEnd % buffer.getChannelCount();
    std::cout << "Playing keysound with range start: " << keyStart << ", range end: " << keyEnd << std::endl;
    auto bufsize = keyEnd - keyStart;
    buffer.play(keyStart, bufsize);

    if (jumpToNext && keyEnd != buffer.getSampleCount())
        settings.cursorPos = settings.markers.get(i + 1).position - (double)offsetSamples;
}

int ApplyNoiseGate(std::vector<float>& buffer, int threshold, int nbChannels) {
    double limit = pow(10.0, (double)threshold / 20.0); // dB to amplitude
    auto result = std::find_if(buffer.rbegin(), buffer.rend(),
        [limit](float i) { return abs(i) > limit; });

    auto pos = std::distance(result, buffer.rend());
    if (pos % nbChannels != 0)
        pos = pos + nbChannels - pos % nbChannels;
    buffer.resize(pos);
    return pos;
}

void ApplyFadeout(std::vector<float>& buffer, int fadeTime, unsigned int sampleRate, int nbChannels) {
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

void WriteKeysounds(SoundBuffer& buffer, SlicerSettings& settings) {
    if (settings.selectedFile.size() == 0) {
        InsertNotification({ ImGuiToastType::Error, 3000, "load_file_first"_t.c_str() });
        return;
    }
    bool hasError = false;
    auto& samples = buffer.getSamples();
    settings.markers.sort();
    unsigned long long keyStart = 0;
    long long keyEnd = 0;
    unsigned long long offsetSamples = (long long)settings.offset * (long long)waveformReso;
    auto p = std::filesystem::u8path(settings.selectedFile);
    auto origFilename = p.filename().replace_extension();
    auto folder = p.remove_filename();
    for (int i = 0; i < settings.markers.size(); i++) {
        Marker m = settings.markers.get(i);
        keyStart = m.position + offsetSamples;
        keyStart = keyStart - keyStart % buffer.getChannelCount();
        if (i + 1.0 >= settings.markers.size()) {
            keyEnd = buffer.getSampleCount();
        }
        else {
            keyEnd = std::min(settings.markers.get(i + 1).position + offsetSamples + ((long long)buffer.getSampleRate() * settings.keysoundOffsetEnd / 1000), (double)buffer.getSampleCount());
        }
        keyEnd = keyEnd - keyEnd % buffer.getChannelCount();
        if (keyEnd < (long long)keyStart) {
            std::cout << "WARNING: Keysound range end is lower than range start, exported file will be silent!" << std::endl;
            hasError = true;
            keyEnd = keyStart;
        }
        std::cout << "Exporting keysound with range start: " << keyStart << ", range end: " << keyEnd << std::endl;
        auto bufsize = keyEnd - keyStart;

        if (keyStart > buffer.getSampleCount())
            continue;

        auto bufOut = &samples[keyStart];
        std::vector<float> newBuf;
        if (settings.selectedGateThreshold != 0 || settings.fadeout != 0) {
            newBuf.insert(newBuf.end(), &bufOut[0], &bufOut[bufsize]);
            if (settings.selectedGateThreshold != 0)
                bufsize = ApplyNoiseGate(newBuf, gateThresholds[settings.selectedGateThreshold], buffer.getChannelCount());
            if (settings.fadeout != 0)
                ApplyFadeout(newBuf, settings.fadeout, buffer.getSampleRate(), buffer.getChannelCount());
            bufOut = &newBuf[0];
        }

        std::filesystem::path finalPath;
        if (m.name.empty()) {
            finalPath = std::filesystem::u8path(folder.u8string() + GetTempMarkerName(origFilename.u8string(), i));
        }
        else {
            finalPath = std::filesystem::u8path(folder.u8string() + m.name);
        }

        std::cout << finalPath.u8string() << std::endl;
        
        if (!SoundBuffer::writeFile(finalPath, buffer.getSampleRate(), buffer.getChannelCount(), bufOut, bufsize)) {
            std::cerr << "Error writing audio file" << std::endl;
        }
    }
    if (hasError)
        InsertNotification({ ImGuiToastType::Warning, 3000, "keysound_export_error"_t.c_str() });
    InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "exported_keysounds_to_folder"_t.c_str(), p.parent_path().u8string().c_str() });
}

unsigned long long FindCrossing(SoundBuffer& buffer, unsigned long long pos, bool searchRight) {
    auto& buf = buffer.getSamples();
    auto sampleCount = buffer.getSampleCount();
    auto channelCount = buffer.getChannelCount();
    auto origVal = buf[pos];
    auto prevPos = pos;
    auto delta = 0.015f;
    if (pos == 0 || (-delta <= origVal && origVal <= delta))
        return pos;
    while (pos >= 0 && pos < sampleCount && buf[pos] != 0 && ((origVal < 0) == (buf[pos] < 0))) {
        if (buf[pos] >= -delta && buf[pos] <= delta)
            break;
        prevPos = pos;
        searchRight ? pos += channelCount : pos -= channelCount;
    }
    return std::abs(buf[prevPos]) > std::abs(buf[pos]) ? prevPos : pos; // Start marker on the sample right after the crossing and not the other way around
}

void ZeroCrossMarkers(SoundBuffer& buffer, SlicerSettings& settings) {
    for (auto& m : settings.markers) {
        unsigned long long p = m.position;
        auto pos = p - p % buffer.getChannelCount();
        auto crossL = FindCrossing(buffer, pos, false);
        auto crossR = FindCrossing(buffer, pos, true);
        if (pos - crossL <= crossR - pos)
            m.position = crossL;
        else
            m.position = crossR;
    }
    settings.updateHistory = true;
    InsertNotification({ ImGuiToastType::Success, 3000, "moved_markers_zerocrossing"_t.c_str() });
}