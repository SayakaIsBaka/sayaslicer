#pragma once

#include "audio.hpp"
#include "settings.hpp"
#include <ImGuiNotify.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <cereal/archives/binary.hpp>

void SaveProject(SlicerSettings settings);
void OpenProject(sf::SoundBuffer& buffer, SlicerSettings& settings, std::string path = "");