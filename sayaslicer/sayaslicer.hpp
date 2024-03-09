#pragma once

#if _WIN32 // Include required headers and pointers for drag and drop on Windows
	#define NOMINMAX
	#include <Windows.h>
	LONG_PTR originalSFMLCallback = 0x0;
	LONG_PTR originalUserData = 0x0;
	LONG_PTR bufferPtr = 0x0;
	LONG_PTR settingsPtr = 0x0;
#endif

#include "bmseclipboard.hpp"
#include "theme.hpp"
#include "custom_widgets.hpp"
#include "settings.hpp"
#include "history.hpp"
#include "audio.hpp"
#include "utils.hpp"
#include "waveform.hpp"
#include "project.hpp"
#include "midi.hpp"
#include "copy_paste.hpp"

#include <IconsFontAwesome6.h>
#include <fonts/fa_solid_900.hpp>
#include <fonts/roboto_medium.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <implot.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>