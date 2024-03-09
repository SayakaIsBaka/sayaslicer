#include "history.hpp"

HistoryItem::HistoryItem(SlicerSettings item) {
	this->item = item;
	timestamp = std::chrono::high_resolution_clock::now();
}

HistoryItem History::get(int _i) {
	auto it = items.begin();
	for (int i = 0; i < _i; i++) {
		++it;
	}
	return *it;
}



void History::UpdateSettings(SlicerSettings& dst, SlicerSettings src, sf::SoundBuffer& buffer) {
	bool fileChange = dst.selectedFile != src.selectedFile;
	dst.markers = src.markers;
	dst.startingKeysound = src.startingKeysound;
	dst.useBase62 = src.useBase62;
	dst.fadeout = src.fadeout;
	dst.selectedGateThreshold = src.selectedGateThreshold;
	dst.selectedFile = src.selectedFile;
	if (fileChange)
		OpenAudioFile(buffer, dst, dst.selectedFile);
}

bool History::IsSameSettings(SlicerSettings a, SlicerSettings b) {
	return a.markers == b.markers &&
		a.startingKeysound == b.startingKeysound &&
		a.useBase62 == b.useBase62 &&
		a.fadeout == b.fadeout &&
		a.selectedGateThreshold == b.selectedGateThreshold &&
		a.selectedFile == b.selectedFile;
}

bool History::AddItem(SlicerSettings item) {
	if (items.size() > 0) {
		auto timestamp = std::chrono::high_resolution_clock::now();
		auto curHistory = get(curPos);
		auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - curHistory.timestamp).count();
		if (IsSameSettings(item, curHistory.item))
			return false;
		if (curPos != 0) {
			while (curPos >= 1) {
				items.pop_front();
				curPos--;
			}
		}
		if (deltaMs < 50) {
			items.pop_front(); // Probably a drag element or something so we replace the most recent item with the new values
		}
		if (items.size() >= maxSize)
			items.pop_back();
	}
	items.push_front(HistoryItem(item));
	return true;
}

bool History::Undo(SlicerSettings& item, sf::SoundBuffer& buffer) {
	if (curPos >= maxSize - 1 || curPos >= items.size() - 1 || items.size() == 0)
		return false;
	SlicerSettings newItem = get(++curPos).item;
	UpdateSettings(item, newItem, buffer);
	return true;
}

bool History::Redo(SlicerSettings& item, sf::SoundBuffer& buffer) {
	if (curPos <= 0 || items.size() < 2)
		return false;
	SlicerSettings newItem = get(--curPos).item;
	UpdateSettings(item, newItem, buffer);
	return true;
}