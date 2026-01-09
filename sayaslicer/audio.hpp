#pragma once

#include "settings.hpp"
#include "utils.hpp"
#include "waveform.hpp"
#include "notifications.hpp"
#include "sound_buffer.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <tcb/span.hpp>

static const int gateThresholds[] = { 0, -24, -30, -36, -42, -48, -54, -60, -66, -72 };

bool OpenAudioFile(SoundBuffer& buffer, SlicerSettings& settings, std::string file = "");
void PlayKeysound(SoundBuffer& buffer, SlicerSettings& settings, bool jumpToNext);
void WriteKeysounds(SoundBuffer& buffer, SlicerSettings& settings);
void ZeroCrossMarkers(SoundBuffer& buffer, SlicerSettings& settings);
void ApplyFadein(tcb::span<float> buffer, int fadeTime, unsigned int sampleRate, int nbChannels);
void ApplyFadeout(tcb::span<float> buffer, int fadeTime, unsigned int sampleRate, int nbChannels);
int ApplyNoiseGate(tcb::span<float> buffer, int threshold, int nbChannels);