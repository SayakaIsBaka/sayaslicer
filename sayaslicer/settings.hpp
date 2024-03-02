#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>

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

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(offset, cursorPos, bpm, snapping, startingKeysound, useBase62, fadeout, selectedGateThreshold, selectedFile, markers);
	}
};