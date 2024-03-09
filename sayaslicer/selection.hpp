#pragma once

#include "marker.hpp"

enum class SelectionOperation {
	COPY,
	CUT,
	PASTE,
	DEL
};

class MarkerSelection {
public:
	double start = -1.0;
	double end = -1.0;
	bool isSelectMode = false;
	MarkerList markerClipboard;

	int CopyMarkers(MarkerList& markers, bool cut = false);
	int PasteMarkers(MarkerList& markers, double position);
	int DeleteSelection(MarkerList& markers);
};
