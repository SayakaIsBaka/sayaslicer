#include "selection.hpp"

int MarkerSelection::CopyMarkers(MarkerList& markers, bool cut) {
	if (start == -1.0 || end == -1.0)
		return -1;
	bool regionHasMarkers = false;
	for (auto m : markers) {
		if (m.position < start)
			continue;
		if (m.position >= end)
			break;
		if (!regionHasMarkers) {
			regionHasMarkers = true;
			markerClipboard.clear(true);
		}
		markerClipboard.push_back(m);
	}
	if (!regionHasMarkers)
		return 0;
	if (cut) {
		for (auto m : markerClipboard)
			markers.remove(m.position);
	}
	return markerClipboard.size();
}

int MarkerSelection::PasteMarkers(MarkerList& markers, double position) {
	double offset = -1.0;
	for (auto m : markerClipboard) {
		if (offset == -1.0)
			offset = m.position;
		double newPos = m.position - offset + position;
		if (markers.find(newPos) == -1.0) {
			markers.push_back(Marker(newPos, m.name));
		}
	}
	return markerClipboard.size();
}

int MarkerSelection::DeleteSelection(MarkerList& markers) {
	if (start == -1.0 || end == -1.0)
		return -1;
	std::vector<double> toRemove;
	for (auto m : markers) {
		if (m.position < start)
			continue;
		if (m.position >= end)
			break;
		toRemove.push_back(m.position);
	}
	for (auto p : toRemove) {
		markers.remove(p);
	}
	return toRemove.size();
}