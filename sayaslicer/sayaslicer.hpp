#pragma once

#if _WIN32 // Include required headers for drag and drop on Windows
	#define NOMINMAX
	#include <Windows.h>
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
#include "preferences.hpp"
#include "translations.hpp"
#include "about.hpp"
#include "console.hpp"
#include "dropmanager.hpp"
#include "sound_buffer.hpp"

#include <IconsFontAwesome6.h>
#include <fonts/fa_solid_900.hpp>
#include <fonts/noto_medium.hpp>
#include <fonts/notokr_medium.hpp>
#include <fonts/notosc_medium.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <implot.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <portaudio.h>