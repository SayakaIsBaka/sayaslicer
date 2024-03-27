#pragma once

#include <ImGuiNotify.hpp>
#include <filesystem>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <clip/clip.h>
#include <sstream>
#include "settings.hpp"
#include "base_convert.hpp"

std::string GetTempMarkerName(std::string filename, size_t idx);
void ExportKeysoundList(SlicerSettings settings);
void ImportNamesFromMid2Bms(SlicerSettings& settings, std::string file = "");
long long LoadFileUnicode(std::string path, std::vector<char>& buf);
void GetStartingKeysoundFromBMS(SlicerSettings& settings);