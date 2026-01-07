#pragma once

#include "settings.hpp"
#include "notifications.hpp"
#include "markdown.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "texture.hpp"

#ifdef __APPLE__
#include <unistd.h>
#include <pwd.h>
#endif

extern const char* kGitHash;

void ShowAbout(SlicerSettings& settings, Texture& logo);
void CheckUpdates(SlicerSettings& settings, bool silent);