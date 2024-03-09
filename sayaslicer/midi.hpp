#pragma once

#include "settings.hpp"
#include <midifile/include/MidiFile.h>
#include <imgui.h>
#include <ImGuiNotify.hpp>
#include <SFML/Audio.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>

void LoadMidi(sf::SoundBuffer& buffer, SlicerSettings& settings);
void ShowMidiTrackModal(sf::SoundBuffer& buffer, SlicerSettings& settings);