#pragma once

#include "settings.hpp"
#include <SFML/Audio.hpp>
#include <implot.h>

static const int waveformReso = 192;

void DrawSelection(SlicerSettings& settings);
void DisplayWaveform(sf::SoundBuffer& buffer, SlicerSettings& settings);