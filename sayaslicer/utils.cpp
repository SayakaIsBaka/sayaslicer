#include "utils.hpp"

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

std::filesystem::path GetBmsFilePath(std::string audioFile, bool enforceOneFile) {
    const std::string bmsExtensions[4] = { ".bms", ".bme", ".bml", ".pms" };
    auto p = std::filesystem::u8path(audioFile);

    std::filesystem::path selectedPath;
    for (const auto& entry : std::filesystem::directory_iterator(p.parent_path())) {
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); }); // To lowercase
        if (std::find(std::begin(bmsExtensions), std::end(bmsExtensions), ext) != std::end(bmsExtensions)) {
            if (enforceOneFile && !selectedPath.empty())
                throw std::invalid_argument("More than one BMS file found");
            selectedPath = entry.path();
            if (!enforceOneFile)
                break; // Only select the first BMS file found
        }
    }
    return selectedPath;
}

std::string ConvertToShiftJIS(std::string s) {
    auto t = iconv_open("SHIFT-JIS", "UTF-8");
    std::string output;
    output.reserve(s.size() * 2); // Better be safe than sorry
    memset(&output[0], 0, s.size() * 2);
    size_t inLeft = s.size();
    size_t outLeft = s.size() * 2;
    auto tmpIn = &s[0];
    auto tmpOut = &output[0];
    iconv(t, &tmpIn, &inLeft, &tmpOut, &outLeft);
    iconv_close(t);
    output = &output[0];
    return output;
}

void ExportKeysoundList(SlicerSettings settings, bool writeToFile) {
    if (settings.selectedFile.empty()) {
        InsertNotification({ ImGuiToastType::Error, 3000, "load_file_first"_t.c_str() });
        return;
    }
    size_t idx = 0;
    std::string res = "";
    auto p = std::filesystem::u8path(settings.selectedFile);
    auto filename = p.filename().replace_extension().u8string();
    for (auto m : settings.markers) {
        char kId[64];
        size_t keysoundId = idx + settings.startingKeysound;
        ToBaseString(kId, IM_ARRAYSIZE(kId), &keysoundId, settings.useBase62 ? 62 : 36);
        std::string keysoundName = m.name;
        if (keysoundName.empty())
            keysoundName = GetTempMarkerName(filename, idx);
        res = res + "#WAV" + std::string(kId) + " " + keysoundName + '\n';
        idx++;
    }
    if (writeToFile) {
        try {
            auto bmsFile = GetBmsFilePath(settings.selectedFile, true);
            if (bmsFile.empty()) {
                InsertNotification({ ImGuiToastType::Error, 3000, "no_suitable_bms_found"_t.c_str() });
                return;
            }
            std::ofstream outFile(bmsFile, std::ios_base::app);
            outFile << std::endl << ConvertToShiftJIS(res);
            InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s"_t.c_str(), "appended_keysound_list_to_file"_t.c_str(), bmsFile.u8string().c_str()});
        }
        catch (std::invalid_argument) {
            InsertNotification({ ImGuiToastType::Error, 3000, "more_than_one_bms_found_in_folder"_t.c_str() });
        }
    }
    else {
        clip::set_text(res);
        InsertNotification({ ImGuiToastType::Success, 3000, "copied_keysound_list_clipboard"_t.c_str() });
    }
}

void ImportNamesFromMid2Bms(SlicerSettings& settings, std::string file) {
    if (file.size() == 0) {
        char const* lFilterPatterns[1] = { "*.txt" };
        char* s = tinyfd_openFileDialog("Open renamer array file...", "text5_renamer_array.txt", 1, lFilterPatterns, "Text file (*.txt)", 0);
        if (s)
            file = s;
        else
            return;
    }
    if (file.size() != 0) {
        auto p = std::filesystem::u8path(file);
        std::ifstream f(p);
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
            settings.updateHistory = true;
            InsertNotification({ ImGuiToastType::Success, 3000, "marker_names_import_success"_t.c_str() });
        }
        else {
            InsertNotification({ ImGuiToastType::Warning, 3000, "marker_names_import_warning"_t.c_str() });
        }
    }
}

long long LoadFileUnicode(std::string path, std::vector<char>& buf) {
    auto fp = std::filesystem::u8path(path);
    if (!std::filesystem::exists(fp))
        return -1;
    std::ifstream file(fp, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    buf.clear();
    buf.resize(size);
    file.read(buf.data(), size);
    return size;
}

void GetStartingKeysoundFromBMS(SlicerSettings& settings) {
    auto selectedPath = GetBmsFilePath(settings.selectedFile, false);
    if (selectedPath.empty()) // No BMS file found in the folder
        return;

    std::ifstream inBms(selectedPath);
    std::list<std::string> wavs;
    std::string l;
    settings.useBase62 = false;

    while (std::getline(inBms, l)) {
        if (l.rfind("#BASE", 0) == 0) {
            std::istringstream iss(l);
            int base;
            std::string cmd;
            if (!(iss >> cmd >> base)) { break; }
            settings.useBase62 = base == 62;
        }
        if (l.rfind("#WAV", 0) == 0)
            wavs.push_back(l.substr(4, 2));
    }
    wavs.sort();
    int lastKeysound = FromBaseToDec(wavs.back().c_str(), settings.useBase62 ? 62 : 36);
    settings.startingKeysound = lastKeysound + 1;
}