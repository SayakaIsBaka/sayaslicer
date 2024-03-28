#pragma once

#if _WIN32 // Include required headers and pointers for drag and drop on Windows
	#include <Windows.h>
	#include <shellapi.h>
#endif

#include <imgui.h>
#include <imgui_markdown/imgui_markdown.h>
#include <string>

void Markdown(const std::string& markdown_);
