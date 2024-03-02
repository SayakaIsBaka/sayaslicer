#pragma once

class SlicerSettings {
public:
	int offset = 0;
	double cursorPos = 0.0;
	float bpm = 120.0;
	int snapping = 4;
	int startingKeysound = 1;
	bool useBase62 = false;
	int fadeout = 0;
	int selectedGateThreshold = 0;
	std::string selectedFile;
	std::list<double> markers = { 0.0 };
	double samplesPerSnap = 0.0;
};