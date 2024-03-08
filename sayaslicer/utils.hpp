#pragma once

std::string GetTempMarkerName(std::string filename, size_t idx) {
    std::string suffix = "_";
    switch (idx > 0 ? (int)log10((double)idx) + 1 : 1) { // Get number of digits
    case 1:
        suffix = "_00";
        break;
    case 2:
        suffix = "_0";
        break;
    }
    return filename + suffix + std::to_string(idx) + ".wav";
}