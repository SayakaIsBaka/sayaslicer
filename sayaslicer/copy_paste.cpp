#include "copy_paste.hpp"

void ManageSelection(SlicerSettings& settings) {
    if (!settings.selection.isSelectMode)
        settings.selection.start = settings.cursorPos;
    else {
        settings.selection.end = settings.cursorPos;
        if (settings.selection.start == settings.selection.end) {
            settings.selection.start = settings.selection.end = -1.0;
        }
        else if (settings.selection.end < settings.selection.start) {
            std::swap(settings.selection.start, settings.selection.end);
        }
    }
    settings.selection.isSelectMode = !settings.selection.isSelectMode;
}

void HandleMarkerCopyPaste(SlicerSettings& settings, SelectionOperation op) {
    if (op == SelectionOperation::COPY || op == SelectionOperation::CUT) {
        bool cut = op == SelectionOperation::CUT;
        int copiedMarkers = settings.selection.CopyMarkers(settings.markers, cut);
        if (copiedMarkers == -1)
            InsertNotification({ ImGuiToastType::Error, 3000, "select_region_first"_t.c_str() });
        else if (copiedMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "selected_region_no_markers"_t.c_str() });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "%s", cut ? i18n::t("cut_markers", copiedMarkers).c_str() : i18n::t("copied_markers", copiedMarkers).c_str() });
            settings.updateHistory = cut;
        }
    }
    else if (op == SelectionOperation::PASTE) {
        int pastedMarkers = settings.selection.PasteMarkers(settings.markers, settings.cursorPos);
        if (pastedMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "clipboard_empty"_t.c_str() });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "%s", i18n::t("pasted_markers", pastedMarkers).c_str() });
            settings.updateHistory = true;
        }

    }
    else if (op == SelectionOperation::DEL) {
        int delMarkers = settings.selection.DeleteSelection(settings.markers);
        if (delMarkers == -1)
            InsertNotification({ ImGuiToastType::Error, 3000, "select_region_first"_t.c_str() });
        else if (delMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "selected_region_no_markers"_t.c_str() });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "%s", i18n::t("deleted_markers", delMarkers).c_str() });
            settings.updateHistory = true;
        }
    }
}