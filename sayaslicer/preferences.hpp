#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <imgui.h>
#include <fstream>

class UserPreferences {
public:
	bool detectStartingKey = false;
	bool openPreferencesModalTemp = false;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(detectStartingKey));
	}
};

void ShowPreferencesModal(UserPreferences& pref);
void LoadPreferences(UserPreferences& pref);