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
            InsertNotification({ ImGuiToastType::Error, 3000, "Please select a region first!" });
        else if (copiedMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "Selected region does not contain any markers!" });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "%s %d %s!", cut ? "Cut" : "Copied", copiedMarkers, copiedMarkers <= 1 ? "marker" : "markers" });
            settings.updateHistory = cut;
        }
    }
    else if (op == SelectionOperation::PASTE) {
        int pastedMarkers = settings.selection.PasteMarkers(settings.markers, settings.cursorPos);
        if (pastedMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "Clipboard is empty!" });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "Pasted %d %s!", pastedMarkers, pastedMarkers <= 1 ? "marker" : "markers" });
            settings.updateHistory = true;
        }

    }
    else if (op == SelectionOperation::DEL) {
        int delMarkers = settings.selection.DeleteSelection(settings.markers);
        if (delMarkers == -1)
            InsertNotification({ ImGuiToastType::Error, 3000, "Please select a region first!" });
        else if (delMarkers == 0)
            InsertNotification({ ImGuiToastType::Error, 3000, "Selected region does not contain any markers!" });
        else {
            InsertNotification({ ImGuiToastType::Success, 3000, "Deleted %d %s!", delMarkers, delMarkers <= 1 ? "marker" : "markers" });
            settings.updateHistory = true;
        }
    }
}