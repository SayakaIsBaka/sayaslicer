#pragma once

#include <imgui.h>
#include <iostream>
#include <i18n_keyval/src/i18n_keyval/i18n.hpp>

// Mostly copied from the demo code
class ConsoleLog
{
    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;
    bool AutoScroll;

public:
    ConsoleLog();
    void Clear();
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void Draw();
};