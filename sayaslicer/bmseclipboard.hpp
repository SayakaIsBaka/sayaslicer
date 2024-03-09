#pragma once

#include <math.h>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <ImGuiNotify.hpp>
#include <SFML/Audio.hpp>
#include <clip/clip.h>
#include "marker.hpp"
#include "settings.hpp"

class BMSEClipboardObject {
public:
	std::string column = "";
	int noteType = 0;
	int position = 0;
	int value = 0;

	double toSamplePosition(double bpm, int sampleRate, int numChannels) {
		return (60.0 / (double)bpm * ((double)sampleRate * (double)numChannels)) / 192.0 * 4.0 * (double)position;
	}

	std::string toString() {
		char pos[8];
		snprintf(pos, 8, "%07d", position);
		return column + std::to_string(noteType) + std::string(pos) + std::to_string(value);
	}

	BMSEClipboardObject(std::string column, int noteType, int position, int value) {
		this->column = column;
		this->noteType = noteType;
		this->position = position;
		this->value = value;
	}

	BMSEClipboardObject(std::string s) {
		if (s.size() >= 12) {
			column = s.substr(0, 3);
			noteType = std::stoi(s.substr(3, 1));
			position = std::stoi(s.substr(4, 7));
			value = std::stoi(s.substr(11));
		}
	}
};

class BMSEClipboard {
public:
	std::vector<BMSEClipboardObject> objects;

	static std::string toBMSEClipboardData(MarkerList markers, double bpm, int sampleRate, int numChannels, int startDef) {
		std::string res = "BMSE ClipBoard Object Data Format\r\n";
		for (auto m : markers) {
			double samplesPer192th = 60.0 / (double)bpm * ((double)sampleRate * (double)numChannels) / 192.0 * 4.0;
			double position = m.position / samplesPer192th;
			BMSEClipboardObject o("101", 0, round(position), startDef++);
			res = res + o.toString() + "\r\n";
		}
		return res;
	}

	BMSEClipboard(std::string s) {
		s.erase(std::remove(s.begin(), s.end(), '\r'), s.cend()); // Remove \r from the string
		std::stringstream ss(s);
		std::string l;
		auto lines = std::vector<std::string>{};
		while (std::getline(ss, l, '\n')) {
			lines.push_back(l);
		}
		if (lines.size() > 0 && lines[0] == "BMSE ClipBoard Object Data Format") {
			for (int i = 1; i < lines.size(); i++) {
				BMSEClipboardObject bo(lines[i]);
				if (!bo.column.empty())
					objects.push_back(bo);
			}
		}
		else {
			std::cout << "Invalid clipboard data" << std::endl;
		}
	}
};

void AddMarkersFromBMSEClipboard(BMSEClipboard objs, sf::SoundBuffer& buffer, SlicerSettings& settings) {
	if (buffer.getSampleCount() > 0) {
		auto sampleRate = buffer.getSampleRate();
		auto numChannels = buffer.getChannelCount();
		for (BMSEClipboardObject o : objs.objects) {
			double m = o.toSamplePosition(settings.bpm, sampleRate, numChannels);
			if (settings.markers.find(m) == -1.0) {
				settings.markers.push_back(m);
			}
		}
		ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully imported markers from the clipboard!" });
	}
	else {
		ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
	}
}

void ProcessBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings& settings) {
	std::string cb;
	clip::get_text(cb);
	BMSEClipboard objs(cb);
	if (!objs.objects.empty()) {
		AddMarkersFromBMSEClipboard(objs, buffer, settings);
		settings.updateHistory = true;
	}
	else
		ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Clipboard does not contain any BMSE data!" });
}

void GenerateBMSEClipboard(sf::SoundBuffer& buffer, SlicerSettings settings) {
	if (buffer.getSampleCount() > 0) {
		auto cb = BMSEClipboard::toBMSEClipboardData(settings.markers, settings.bpm, buffer.getSampleRate(), buffer.getChannelCount(), settings.startingKeysound);
		std::cout << cb << std::endl;
		clip::set_text(cb);
		ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Copied markers as BMSE clipboard data!" });
	}
	else {
		ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Please load a file first!" });
	}
}