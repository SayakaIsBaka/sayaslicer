#pragma once

#include <math.h>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include "notifications.hpp"
#include <SFML/Audio.hpp>
#include <clip/clip.h>
#include "marker.hpp"
#include "settings.hpp"

class BMSEClipboardObject {
public:
	std::string column = "";
	int noteType = 0;
	int position = 0;
	int value = 0;

	double toSamplePosition(double bpm, int sampleRate, int numChannels);
	std::string toString();
	BMSEClipboardObject(std::string column, int noteType, int position, int value);
	BMSEClipboardObject(std::string s);
};

class iBMSCClipboardObject {
public:
	int column = 0;
	double position = 0;
	int value = 0;
	int ln_length = 0;
	bool invisible = false;
	bool mine = false;

	std::string toString();
	iBMSCClipboardObject(int column, double position, int value);
};

class BMSEClipboard {
public:
	std::vector<BMSEClipboardObject> objects;

	static std::string toBMSEClipboardData(MarkerList markers, double bpm, int sampleRate, int numChannels, int startDef);
	static std::string toiBMSCClipboardData(MarkerList markers, double bpm, int sampleRate, int numChannels, int startDef);
	BMSEClipboard(std::string s);
};

void AddMarkersFromBMSEClipboard(BMSEClipboard objs, sf::SoundBuffer& buffer, SlicerSettings& settings);
void ProcessBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings& settings);
void GenerateBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings settings, bool useiBMSC);