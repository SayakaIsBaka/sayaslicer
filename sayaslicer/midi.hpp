#pragma once

#include "settings.hpp"
#include <midifile/include/MidiFile.h>
#include <imgui.h>
#include "notifications.hpp"
#include "sound_buffer.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>

void LoadMidi(SoundBuffer& buffer, SlicerSettings& settings, std::string file = "");
void ShowMidiTrackModal(SoundBuffer& buffer, SlicerSettings& settings);