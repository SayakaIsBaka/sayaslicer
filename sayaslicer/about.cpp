#include "about.hpp"

using namespace i18n::literals;
using json = nlohmann::json;

void DownloadUpdate() {
#ifdef _WIN32
    std::string os = "windows";
#elif __APPLE__
    #if __arm64__
    std::string os = "osx-arm64";
    #else
    std::string os = "osx-x64";
    #endif
#else
    std::string os = "linux";
#endif
    std::string filename = "sayaslicer-" + os + ".zip";
    InsertNotification({ ImGuiToastType::Info, 3000, "downloading_update"_t.c_str() });
    cpr::GetCallback([filename](cpr::Response r) {
        try {
            if (r.error || r.status_code != 200)
                throw std::invalid_argument("Error downloading update");
            else {
                std::ofstream of(filename, std::ios::binary);
                of.write(r.text.c_str(), r.text.size());
                of.close();
                InsertNotification({ ImGuiToastType::Success, 3000, "%s:\n%s", "downloaded_update_to"_t.c_str(), (std::filesystem::current_path() / filename).u8string().c_str()});
            }
        }
        catch (...) {
            InsertNotification({ ImGuiToastType::Error, 3000, "error_downloading_update"_t.c_str() });
        }
    }, cpr::Url{ "https://nightly.link/SayakaIsBaka/sayaslicer/workflows/main/master/" + filename });
}

void CheckUpdates(SlicerSettings& settings, bool silent) {
    cpr::GetCallback([&settings, silent](cpr::Response r) {
        try {
            if (r.error || r.status_code != 200)
                throw std::invalid_argument("Error getting runs from GitHub");
            auto resJson = json::parse(r.text);
            std::string latestVerSha;
            for (auto run : resJson["workflow_runs"]) {
                if (run["conclusion"] == "success") {
                    latestVerSha = run["head_sha"];
                    break;
                }
            }
            if (latestVerSha.empty())
                throw std::invalid_argument("Invalid JSON response");
            if (strncmp(latestVerSha.substr(0, 7).c_str(), kGitHash, 7) == 0) {
                if (!silent)
                    InsertNotification({ ImGuiToastType::Success, 3000, "latest_version"_t.c_str() });
            }
            else {
                settings.prefs.updateAvailable = true;
                InsertNotification({ ImGuiToastType::Info, 3000, "update_available"_t.c_str() });
            }
        }
        catch (...) {
            if (!silent)
                InsertNotification({ ImGuiToastType::Error, 3000, "error_update_checking"_t.c_str() });
        }
    }, cpr::Url{ "https://api.github.com/repos/SayakaIsBaka/sayaslicer/actions/runs" });
}

void ShowAbout(SlicerSettings& settings, Texture &logo) {
    if (settings.openAboutModalTemp) {
        settings.openAboutModalTemp = false;
        ImGui::OpenPopup("About");
    }
    if (ImGui::BeginPopupModal("About", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
        {
            ImGui::BeginChild("AboutBanner", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoBackground);
            {
                ImGui::BeginChild("AboutLeft", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoBackground);
                ImGui::Image((ImTextureID)(intptr_t)logo.texture, ImVec2(logo.width, logo.height));
                ImGui::EndChild();
            }
            ImGui::SameLine();
            {
                ImGui::BeginChild("AboutRight", ImVec2(225.0, 55), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoBackground);
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram]);
                ImGui::Text("sayaslicer");
                ImGui::PopStyleColor();
                ImGui::Text("v%s - %s", kGitHash, VERSIONDATE);
                Markdown("[https://github.com/SayakaIsBaka/sayaslicer](https://github.com/SayakaIsBaka/sayaslicer)");
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }

        if (ImGui::CollapsingHeader("help"_t.c_str())) {
            std::string help = "keyboard_shortcuts"_t + u8":\n___\n" + "keyboard_shortcuts_list"_t;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.400000095367432f));
            Markdown(help);
            ImGui::PopStyleVar();
        }

        if (ImGui::CollapsingHeader("credits"_t.c_str())) {
            std::string credits = "used_libraries"_t + u8R"(:
___
  * [Dear ImGui](https://github.com/ocornut/imgui)
  * [SFML](https://github.com/SFML/SFML)
  * [imgui-sfml](https://github.com/SFML/imgui-sfml)
  * [implot](https://github.com/epezent/implot)
  * [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
  * [clip](https://github.com/dacap/clip)
  * [ImGuiNotify](https://github.com/TyomaVader/ImGuiNotify)
  * [cereal](https://github.com/USCiLab/cereal)
  * [midifile](https://github.com/craigsapp/midifile)
  * [i18n_keyval](https://github.com/stefandevai/i18n_keyval)
  * [cpr](https://github.com/libcpr/cpr)
  * [nlohmann_json](https://github.com/nlohmann/json)
  * [imgui_markdown](https://github.com/juliettef/imgui_markdown)
  * [libiconv](https://www.gnu.org/software/libiconv/)
  * [Moonlight (ImGui theme)](https://github.com/Madam-Herta/Moonlight/)

Translations:
___
  * Korean (한국어): [Sobrem](https://twitter.com/SobremMusic)
  * Simplified Chinese (简体中文): [Zris](https://twitter.com/Zrisound)
  * Spanish (Español): [MADizM*rsoff](https://github.com/Marsoff9898)
)";
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.400000095367432f));
            Markdown(credits);
            ImGui::PopStyleVar();
        }

        ImGui::Separator();
        if (ImGui::Button("check_for_updates"_t.c_str())) {
            CheckUpdates(settings, false);
        }
        if (settings.prefs.updateAvailable) {
            ImGui::SameLine();
            if (ImGui::Button("download_update"_t.c_str())) {
                DownloadUpdate();
            }
        }

        if (ImGui::Button("close"_t.c_str())) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}