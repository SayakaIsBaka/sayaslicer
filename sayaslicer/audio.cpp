#include "audio.hpp"

using namespace i18n::literals;

bool OpenAudioFile(sf::SoundBuffer& buffer, SlicerSettings& settings, std::string file)
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
        auto size = LoadFileUnicode(settings.selectedFile, bTmp);
        if (size == -1) {
            InsertNotification({ ImGuiToastType::Error, 3000, "file_doesnt_exist"_t.c_str() });
            return false;
        }
        bool res = buffer.loadFromMemory(bTmp.data(), size);

        if (res) {
            std::cout << "Duration: " << buffer.getDuration().asSeconds() << std::endl;
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
    keyStart = keyStart - keyStart % buffer.getChannelCount();
    keyEnd = keyEnd - keyEnd % buffer.getChannelCount();
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
        InsertNotification({ ImGuiToastType::Error, 3000, "load_file_first"_t.c_str() });
        return;
    }
    auto samples = buffer.getSamples();
    settings.markers.sort();
    unsigned long long keyStart = 0;
    unsigned long long keyEnd = 0;
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
            keyEnd = settings.markers.get(i + 1).position + offsetSamples;
        }
        keyEnd = keyEnd - keyEnd % buffer.getChannelCount();
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
        std::filesystem::path u8filename;
        std::filesystem::path finalPath;
        // Write to temp file first because SFML 2.6 doesn't support unicode paths
        if (m.name.empty()) {
            filename = GetTempMarkerName(origFilename.string(), i);
            u8filename = std::filesystem::u8path(GetTempMarkerName(origFilename.u8string(), i));
            finalPath = std::filesystem::u8path(folder.u8string() + GetTempMarkerName(origFilename.u8string(), i));
        }
        else {
            filename = m.name;
            u8filename = std::filesystem::u8path(m.name);
            finalPath = std::filesystem::u8path(folder.u8string() + m.name);
        }

        std::cout << finalPath.u8string() << std::endl;
        if (!file.openFromFile(filename, buffer.getSampleRate(), buffer.getChannelCount())) {
            puts("Error opening temp file for writing");
        }
        file.write(bufOut, bufsize);
        file.close();
        std::filesystem::rename(u8filename, finalPath);
    }
    InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "exported_keysounds_to_folder"_t.c_str(), p.parent_path().u8string().c_str() });
}

unsigned long long FindCrossing(sf::SoundBuffer& buffer, unsigned long long pos, bool searchRight) {
    auto buf = buffer.getSamples();
    auto sampleCount = buffer.getSampleCount();
    auto channelCount = buffer.getChannelCount();
    auto origVal = buf[pos];
    auto prevPos = pos;
    int delta = 500;
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

void ZeroCrossMarkers(sf::SoundBuffer& buffer, SlicerSettings& settings) {
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