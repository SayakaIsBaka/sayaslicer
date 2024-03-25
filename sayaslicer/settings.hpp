#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <midifile/include/MidiFile.h>
#include <filesystem>
#include "marker.hpp"
#include "selection.hpp"

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
	MarkerSelection selection;
	bool updateHistory = false;
	bool openMidiModalTemp = false;
	float maxDisplayRange = 1.0;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(offset, cursorPos, bpm, snapping, startingKeysound, useBase62, fadeout, selectedGateThreshold, selectedFile, markers);
	}
};