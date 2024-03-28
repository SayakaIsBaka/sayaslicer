#pragma once

#include "settings.hpp"
#include "notifications.hpp"
#include "markdown.hpp"
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

extern const char* kGitHash;

void ShowAbout(SlicerSettings& settings, sf::Texture& logo);
void CheckUpdates(bool silent);