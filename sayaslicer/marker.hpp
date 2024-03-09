#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <list>

class Marker {
public:
	double position;
	std::string name;

	Marker(double position, std::string name);
	Marker();

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(position, name);
	}
};

inline bool operator==(const Marker& lhs, const Marker& rhs) {
	return lhs.position == rhs.position && lhs.name == rhs.name;
}

class MarkerList {
private:
	std::list<Marker> markers = {};

public:
	bool operator==(const MarkerList& rhs);
	void push_back(Marker m);
	void push_back(double m);
	void remove(double m);
	void clear(bool clearAll = false);
	size_t size();
	void sort();
	double find(double e);
	Marker get(int _i);
	bool importNames(std::vector<std::string> names);
	std::list<Marker>::iterator begin();
	std::list<Marker>::iterator end();

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(markers);
	}
};