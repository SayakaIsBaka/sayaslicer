#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "base_convert.hpp"
#include <imgui.h>
#include <imgui_internal.h>

static const float DRAG_MOUSE_THRESHOLD_FACTOR = 0.50f;

bool SelectableInput(const char* str_id, bool selected, ImGuiSelectableFlags flags, char* buf, size_t buf_size, char* display_text = nullptr);
bool DragIntCustomBase(const char* label, int* v, float v_speed = (1.0F), int v_min = 0, int v_max = 0, int base = 36, ImGuiSliderFlags flags = 0);
bool AddScalarScroll(ImGuiDataType data_type, void* v, float v_min, float v_max, float scrollStep = 10);