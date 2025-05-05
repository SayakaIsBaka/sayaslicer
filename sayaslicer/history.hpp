#pragma once

#include "settings.hpp"
#include "audio.hpp"
#include <chrono>
#include <list>

class HistoryItem {
public:
	SlicerSettings item;
	std::chrono::high_resolution_clock::time_point timestamp;

	HistoryItem(SlicerSettings item);
};

class History {
private:
	std::list<HistoryItem> items; // Most recent at front, oldest at back
	const int maxSize = 100;
	size_t curPos = 0;

	HistoryItem get(int _i);
	void UpdateSettings(SlicerSettings& dst, SlicerSettings src, SoundBuffer& buffer);
	bool IsSameSettings(SlicerSettings a, SlicerSettings b);

public:
	bool AddItem(SlicerSettings item);
	bool Undo(SlicerSettings& item, SoundBuffer& buffer);
	bool Redo(SlicerSettings& item, SoundBuffer& buffer);
};