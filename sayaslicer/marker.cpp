#include "marker.hpp"

Marker::Marker(double position, std::string name) {
	this->position = position;
	this->name = name;
	this->name.reserve(4096);
}

Marker::Marker() {
	position = 0.0;
}



bool MarkerList::operator==(const MarkerList& rhs) {
	return markers == rhs.markers;
}

void MarkerList::push_back(Marker m) {
	markers.push_back(m);
}

void MarkerList::push_back(double m) {
	markers.push_back(Marker(m, ""));
}

void MarkerList::remove(double m) {
	markers.remove_if([m](Marker marker) { return marker.position == m; });
}

void MarkerList::clear(bool clearAll) {
	markers.clear();
	if (!clearAll)
		markers.push_back(Marker(0.0, ""));
}

size_t MarkerList::size() {
	return markers.size();
}

void MarkerList::sort() {
	auto sortLambda =
		[](const Marker& a, const Marker& b) -> bool
	{
		return a.position < b.position;
	};
	markers.sort(sortLambda);
}

std::list<Marker>::iterator MarkerList::begin() {
	return markers.begin();
}

std::list<Marker>::iterator MarkerList::end() {
	return markers.end();
}

double MarkerList::find(double e) {
	for (auto m : markers) {
		if (std::abs(m.position - e) < 0.000001)
			return m.position;
	}
	return -1.0;
}

Marker MarkerList::get(int _i) {
	auto it = markers.begin();
	for (int i = 0; i < _i; i++) {
		++it;
	}
	return *it;
}

bool MarkerList::importNames(std::vector<std::string> names) {
	bool perfectMatch = false;
	if (names.size() == markers.size())
		perfectMatch = true;
	size_t limit = names.size() < markers.size() ? names.size() : markers.size();
	size_t i = 0;
	for (auto& marker : markers) {
		marker.name = names[i];
		if (++i >= limit)
			break;
	}
	return perfectMatch;
}