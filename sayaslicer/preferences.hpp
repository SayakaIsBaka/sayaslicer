#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <imgui.h>
#include <fstream>
#include "translations.hpp"

class UserPreferences {
public:
	bool detectStartingKey = false;
	bool openPreferencesModalTemp = false;
	bool checkForUpdates = false;
	std::string language = "en";

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(detectStartingKey), CEREAL_NVP(language), CEREAL_NVP(checkForUpdates));
	}
};

void ShowPreferencesModal(UserPreferences& pref);
void LoadPreferences(UserPreferences& pref);