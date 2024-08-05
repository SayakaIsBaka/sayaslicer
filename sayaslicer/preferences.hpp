#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <imgui.h>
#include <fstream>
#include "translations.hpp"
#include "notifications.hpp"

class UserPreferences {
public:
	bool detectStartingKey = false;
	bool openPreferencesModalTemp = false;
	bool checkForUpdates = false;
	bool updateAvailable = false;
	int fontSize = 14;
	std::string language = "en";

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(detectStartingKey), CEREAL_NVP(language), CEREAL_NVP(checkForUpdates), CEREAL_NVP(fontSize));
	}
};

void ShowPreferencesModal(UserPreferences& pref);
void LoadPreferences(UserPreferences& pref);