#pragma once

#include <ImGuiNotify.hpp>
#include "translations.hpp"

using namespace i18n::literals;

inline void InsertNotification(ImGuiToast toast) {
	ImGuiToast tmpToast = toast;
	switch (tmpToast.getType())
	{
		case ImGuiToastType::Success:
			tmpToast.setTitle("success"_t.c_str());
			break;
		case ImGuiToastType::Warning:
			tmpToast.setTitle("warning"_t.c_str());
			break;
		case ImGuiToastType::Error:
			tmpToast.setTitle("error"_t.c_str());
			break;
		case ImGuiToastType::Info:
			tmpToast.setTitle("info"_t.c_str());
			break;
	}
	ImGui::InsertNotification(tmpToast);
}