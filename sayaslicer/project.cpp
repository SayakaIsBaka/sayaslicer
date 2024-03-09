#include "project.hpp"

void SaveProject(SlicerSettings settings) {
    char const* aFilterPatterns[1] = { "*.syp" };
    char* s = tinyfd_saveFileDialog("Save project file...", 0, 1, aFilterPatterns, "sayaslicer Project (*.syp)");
    if (s) {
        std::ofstream outFile(s, std::ios::binary);
        cereal::BinaryOutputArchive oarchive(outFile);
        oarchive(settings);
        ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Saved project to:\n%s", s });
    }
}

void OpenProject(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    char const* lFilterPatterns[1] = { "*.syp" };
    char* s = tinyfd_openFileDialog("Open project file...", 0, 1, lFilterPatterns, "sayaslicer Project (*.syp)", 0);
    if (s) {
        std::ifstream inFile(s, std::ios::binary);
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