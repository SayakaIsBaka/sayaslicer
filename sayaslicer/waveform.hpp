#pragma once

#include "settings.hpp"
#include <SFML/Audio.hpp>
#include <implot.h>
#include <cmath>

static const int waveformReso = 48;
static const double minZoom = 6000.0;

void DrawSelection(SlicerSettings& settings);
void DisplayWaveform(sf::SoundBuffer& buffer, SlicerSettings& settings);