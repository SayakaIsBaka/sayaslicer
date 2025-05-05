#pragma once

#if _WIN32 // Include required headers for drag and drop on Windows
	#define NOMINMAX
	#include <Windows.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <list>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <portaudio.h>

#if _WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
	#define GLFW_NATIVE_INCLUDE_NONE
	#include <GLFW/glfw3native.h>
#endif