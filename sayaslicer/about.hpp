#pragma once

#include "settings.hpp"
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>

extern const char* kGitHash;

void ShowAbout(SlicerSettings& settings, sf::Texture& logo);