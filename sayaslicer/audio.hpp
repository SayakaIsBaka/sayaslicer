#pragma once

#include "settings.hpp"
#include "utils.hpp"
#include "waveform.hpp"
#include "notifications.hpp"
#include "sound_buffer.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>

static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };

bool OpenAudioFile(SoundBuffer& buffer, SlicerSettings& settings, std::string file = "");
void PlayKeysound(SoundBuffer& buffer, SlicerSettings& settings, bool jumpToNext);
void WriteKeysounds(SoundBuffer& buffer, SlicerSettings& settings);
void ZeroCrossMarkers(SoundBuffer& buffer, SlicerSettings& settings);