#include "preferences.hpp"

const char* settingsFile = "settings.json";

void SavePreferences(UserPreferences pref) {
	std::ofstream outFile(settingsFile);
	cereal::JSONOutputArchive oarchive(outFile);
	oarchive(cereal::make_nvp("settings", pref));
}

void LoadPreferences(UserPreferences& pref) {
	std::ifstream inFile(settingsFile);
	if (inFile.is_open()) {
		cereal::JSONInputArchive iarchive(inFile);
		iarchive(pref);
	}
}

void ShowPreferencesModal(UserPreferences& pref) {
	if (pref.openPreferencesModalTemp) {
		pref.openPreferencesModalTemp = false;
		ImGui::OpenPopup("Preferences");
	}
	if (ImGui::BeginPopupModal("Preferences", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::Checkbox("Auto-detect starting keysound", &pref.detectStartingKey);
		if (ImGui::Button("Save")) {
			SavePreferences(pref);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}