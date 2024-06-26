#include "translations.hpp"

using json = nlohmann::json;

void LoadTranslations(std::string path) {
	translations = {};
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().extension() == ".json") {
			try {
				std::ifstream f(entry.path());
				auto lang = entry.path().filename().replace_extension("").string();
				json data = json::parse(f);
				for (auto& [key, val] : data.items()) {
					if (val.is_array()) {
						std::string s = "";
						for (auto l : val)
							if (l.is_string())
								s += l.template get<std::string>() + "\n";
						data[key] = s;
					}
				}
				translations[lang] = data.get<std::unordered_map<std::string, std::string>>();
			}
			catch (std::exception e) {
				std::cerr << "Error loading the following language file, skipping: " << entry.path().filename().string() << std::endl;
			}
		}
	}
}

void InitTranslations(std::string locale) {
	try {
		LoadTranslations("lang");
	}
	catch (std::exception e) {
		std::cerr << "Error loading language files, make sure the lang folder exists and is in the same folder as sayaslicer" << std::endl;
	}
	try {
		i18n::set_locale(locale);
		i18n::initialize_translator(translations);
	}
	catch (std::exception e) {
		if (translations.find("en") == translations.end()) {
			tinyfd_messageBox("Error", "Error loading English language file, please redownload sayaslicer and make sure the lang folder is in the same folder as the executable.", "ok", "error", 1);
			abort();
		}
		else {
			std::cerr << "Locale " << locale << " not found, falling back to english" << std::endl;
			InitTranslations("en");
		}
	}
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

std::string GetLangPrettyFromId(std::string id) {
	if (translations.find(id) != translations.end())
		return translations[id]["_lang"];
	return "";
}