#include "translations.hpp"

void InitTranslations(std::string locale) {
	i18n::set_locale(locale);
	i18n::initialize_translator(translations);
}

std::vector<std::string> GetLanguages() {
	std::vector<std::string> keys;
	keys.reserve(translations.size());
	for (auto kv : translations) {
		keys.push_back(kv.first);
	}
	return keys;
}

std::vector<std::string> GetLanguagesPretty() {
	std::vector<std::string> keys;
	keys.reserve(translations.size());
	for (auto kv : translations) {
		keys.push_back(kv.second["_lang"]);
	}
	return keys;
}