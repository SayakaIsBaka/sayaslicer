#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <list>

class Marker {
public:
	double position;
	std::string name;

	Marker(double position, std::string name) {
		this->position = position;
		this->name = name;
		this->name.reserve(4096);
	}

	Marker() {
		position = 0.0;
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(position, name);
	}
};

class MarkerList {
private:
	std::list<Marker> markers = {};

public:
	void push_back(Marker m) {
		markers.push_back(m);
	}

	void push_back(double m) {
		markers.push_back(Marker(m, ""));
	}

	void remove(double m) {
		markers.remove_if([m](Marker marker) { return marker.position == m; });
	}

	void clear() {
		markers.clear();
	}

	size_t size() {
		return markers.size();
	}

	void sort() {
		auto sortLambda =
			[](const Marker& a, const Marker& b) -> bool
		{
			return a.position < b.position;
		};
		markers.sort(sortLambda);
	}

	auto begin() {
		return markers.begin();
	}

	auto end() {
		return markers.end();
	}

	double find(double e) {
		for (auto m : markers) {
			if (std::abs(m.position - e) < 0.000001)
				return m.position;
		}
		return -1.0;
	}

	Marker get(int _i) {
		auto it = markers.begin();
		for (int i = 0; i < _i; i++) {
			++it;
		}
		return *it;
	}

	bool importNames(std::vector<std::string> names) {
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

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(markers);
	}
};