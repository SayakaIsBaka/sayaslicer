#include "preferences.hpp"

using namespace i18n::literals;

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
		try {
			iarchive(pref);
		}
		catch (cereal::Exception) {} // Do not crash if upgrading from an old config file
	}
}

void ShowPreferencesModal(UserPreferences& pref) {
	if (pref.openPreferencesModalTemp) {
		pref.openPreferencesModalTemp = false;
		ImGui::OpenPopup("Preferences");
	}

	static UserPreferences pTmp = pref;
	pTmp.updateAvailable = pref.updateAvailable; // Necessary since pTmp is initialized before updateAvailable is properly updated
	if (ImGui::BeginPopupModal("Preferences", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::Checkbox("autodetect_starting_keysound"_t.c_str(), &pTmp.detectStartingKey);
		ImGui::Checkbox("check_for_updates_startup"_t.c_str(), &pTmp.checkForUpdates);
		auto langs = GetLanguages();
		auto langsPretty = GetLanguagesPretty();
		auto combo_preview_value = GetLangPrettyFromId(pTmp.language);
		if (combo_preview_value.empty())
			pTmp.language = pref.language = "en"; // Fallback to english
		ImGui::Text("%s:", "language"_t.c_str());
		if (ImGui::BeginCombo("##language_select", combo_preview_value.c_str())) {
			for (int n = 0; n < langsPretty.size(); n++)
			{
				const bool is_selected = (GetLangPrettyFromId(pTmp.language) == langsPretty[n]);
				if (ImGui::Selectable(langsPretty[n].c_str(), is_selected)) {
					pTmp.language = langs[n];
					i18n::set_locale(pTmp.language);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("save"_t.c_str())) {
			pref = pTmp;
			pTmp = pref;
			i18n::set_locale(pref.language);
			SavePreferences(pref);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("cancel"_t.c_str())) {
			pTmp = pref;
			i18n::set_locale(pref.language);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}