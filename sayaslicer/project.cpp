#include "project.hpp"

void SaveProject(SlicerSettings settings) {
    char const* aFilterPatterns[1] = { "*.syp" };
    char* s = tinyfd_saveFileDialog("Save project file...", 0, 1, aFilterPatterns, "sayaslicer Project (*.syp)");
    if (s) {
        auto p = std::filesystem::u8path(s);
        std::ofstream outFile(p, std::ios::binary);
        cereal::BinaryOutputArchive oarchive(outFile);
        oarchive(settings);
        InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "saved_project_to"_t.c_str(), s});
    }
}

void OpenProject(SoundBuffer& buffer, SlicerSettings& settings, std::string file) {
    if (file.size() == 0) {
        char const* lFilterPatterns[1] = { "*.syp" };
        char* s = tinyfd_openFileDialog("Open project file...", 0, 1, lFilterPatterns, "sayaslicer Project (*.syp)", 0);
        if (s)
            file = s;
        else
            return;
    }
    if (file.size() != 0) {
        auto p = std::filesystem::u8path(file);
        std::ifstream inFile(p, std::ios::binary);
        cereal::BinaryInputArchive iarchive(inFile);
        try {
            iarchive(settings);
        } catch (std::exception) {}
        if (std::filesystem::exists(std::filesystem::u8path(settings.selectedFile))) {
            OpenAudioFile(buffer, settings, settings.selectedFile);
        }
        else {
            OpenAudioFile(buffer, settings);
        }
        InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "opened_project"_t.c_str(), file.c_str()});
    }
}