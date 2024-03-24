#pragma once

#include "settings.hpp"
#include "utils.hpp"
#include "waveform.hpp"
#include <ImGuiNotify.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>

static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };

bool OpenAudioFile(sf::SoundBuffer& buffer, SlicerSettings& settings, std::string file = "");
void PlayKeysound(sf::Sound& sound, sf::SoundBuffer& buffer, sf::SoundBuffer& buffer2, SlicerSettings& settings, bool jumpToNext);
void WriteKeysounds(sf::SoundBuffer& buffer, SlicerSettings& settings);
void ZeroCrossMarkers(sf::SoundBuffer& buffer, SlicerSettings& settings);