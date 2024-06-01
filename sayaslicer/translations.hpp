#pragma once

#include <i18n_keyval/src/i18n_keyval/i18n.hpp>
#include <i18n_keyval/src/i18n_keyval/translators/basic.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

static i18n::translations translations = {};

void InitTranslations(std::string locale);
std::vector<std::string> GetLanguages();
std::vector<std::string> GetLanguagesPretty();
std::string GetLangPrettyFromId(std::string id);