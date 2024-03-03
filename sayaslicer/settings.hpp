#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <midifile/include/MidiFile.h>
#include <filesystem>
#include "marker.hpp"

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
	MarkerList markers;
	double samplesPerSnap = 0.0;
	smf::MidiFile midiFile;

	void AddMarker(double position, std::string name = "")
	{
		if (name.empty()) {
			std::filesystem::path p = selectedFile;
			auto filename = p.filename().replace_extension().string();
			auto idx = markers.size();
			std::string suffix = "_";
			switch (idx > 0 ? (int)log10((double)idx) + 1 : 1) { // Get number of digits
			case 1:
				suffix = "_00";
				break;
			case 2:
				suffix = "_0";
				break;
			}
			name = filename + suffix + std::to_string(idx) + ".wav";
		}
		Marker m(position, name);
		markers.push_back(m);
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(offset, cursorPos, bpm, snapping, startingKeysound, useBase62, fadeout, selectedGateThreshold, selectedFile, markers);
	}
};