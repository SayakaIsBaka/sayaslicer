#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <imgui.h>
#include <fstream>
#include "translations.hpp"
#include "notifications.hpp"

#define DEFAULT_FONT_SIZE 14

class UserPreferences {
public:
	bool detectStartingKey = false;
	bool openPreferencesModalTemp = false;
	bool checkForUpdates = false;
	bool updateAvailable = false;
	float fontScale = 1.0f;
	std::string language = "en";

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(detectStartingKey), CEREAL_NVP(language), CEREAL_NVP(checkForUpdates), CEREAL_NVP(fontScale));
	}
};

void ShowPreferencesModal(UserPreferences& pref);
void LoadPreferences(UserPreferences& pref);