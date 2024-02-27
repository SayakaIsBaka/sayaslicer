#pragma once

#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>

class BMSEClipboardObject {
public:
	std::string column = "";
	int noteType = 0;
	int position = 0;
	int value = 0;

	double toSamplePosition(double bpm, int sampleRate, int numChannels) {
		return (60.0 / (double)bpm * ((double)sampleRate * (double)numChannels)) / 192.0 * 4.0 * (double)position;
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