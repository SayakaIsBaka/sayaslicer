#pragma once

#include "settings.hpp"
#include "sound_buffer.hpp"
#include <implot.h>
#include <cmath>

static const int waveformReso = 48;
static const double minZoom = 6000.0;

void DrawSelection(SlicerSettings& settings);
void DisplayWaveform(SoundBuffer& buffer, SlicerSettings& settings);