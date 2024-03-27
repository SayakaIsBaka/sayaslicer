#include "sayaslicer.hpp"

using namespace std;

void ShowMenuFile(sf::SoundBuffer& buffer, SlicerSettings &settings, sf::RenderWindow &window)
{
    if (ImGui::MenuItem("Open project", "Ctrl+O")) {
        OpenProject(buffer, settings);
    }
    if (ImGui::MenuItem("Save project", "Ctrl+S")) {
        SaveProject(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Import audio file", "O")) {
        OpenAudioFile(buffer, settings);
    }
    if (ImGui::MenuItem("Export keysounds", "M")) {
        WriteKeysounds(buffer, settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        window.close();
    }
}

void ShowMenuEdit(sf::SoundBuffer& buffer, SlicerSettings& settings)
{
    if (ImGui::MenuItem("Import slices from MIDI")) {
        LoadMidi(buffer, settings);
    }
    if (ImGui::MenuItem("Import Mid2BMS renamer file")) {
        ImportNamesFromMid2Bms(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Copy BMSE clipboard data", "V")) {
        GenerateBMSEClipboard(buffer, settings, false);
    }
    if (ImGui::MenuItem("Copy iBMSC clipboard data", "Shift+V")) {
        GenerateBMSEClipboard(buffer, settings, true);
    }
    if (ImGui::MenuItem("Paste BMSE clipboard data", "B")) {
        ProcessBMSEClipboard(buffer, settings);
    }
    if (ImGui::MenuItem("Copy keysound list to clipboard", "K")) {
        ExportKeysoundList(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Clear all markers", "C")) {
        settings.markers.clear(false);
        settings.updateHistory = true;
    }
    if (ImGui::MenuItem("Clear all markers (including 0)", "Shift+C")) {
        settings.markers.clear(true);
        settings.updateHistory = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Preferences")) {
        settings.prefs.openPreferencesModalTemp = true;
    }
}

void ShowMainMenuBar(sf::SoundBuffer& buffer, SlicerSettings &settings, sf::RenderWindow &window)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile(buffer, settings, window);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ShowMenuEdit(buffer, settings);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void SetupFonts(ImGuiIO& io) {
    float mainFontSize = 14.0f;
    float iconFontSize = mainFontSize * 2.0f / 3.0f;

    io.Fonts->AddFontFromMemoryCompressedTTF(noto_compressed_data, noto_compressed_size, mainFontSize, 0, io.Fonts->GetGlyphRangesJapanese());

    static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);

    io.Fonts->Build();
    ImGui::SFML::UpdateFontTexture();
}

void SetupDock() {
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    static bool init = true;
    ImGuiID dock_id_left, dock_id_right;
    if (init) {
        init = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_id_left, &dock_id_right);
        ImGui::DockBuilderDockWindow("Settings", dock_id_left);
        ImGui::DockBuilderDockWindow("Waveform", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }
}

void ShowSettingsPanel(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Settings"))
    {
        double minPos = 0;
        double maxPos = buffer.getSampleCount();
        static float zoom = 0;

        ImGui::SeparatorText("General");
        ImGui::SliderFloat("Zoom", &zoom, 0, minZoom - waveformReso * 2.0);
        AddScalarScroll(ImGuiDataType_Float, &zoom, 0, minZoom - waveformReso * 2.0, 200);
        settings.maxDisplayRange = minZoom - zoom;
        ImGui::DragInt("Offset", &settings.offset, 1, 0, 1000);
        AddScalarScroll(ImGuiDataType_S32, &settings.offset, 0, 1000, 10);
        ImGui::DragScalar("Position", ImGuiDataType_Double, &settings.cursorPos, 1, &minPos, &maxPos);
        AddScalarScroll(ImGuiDataType_Double, &settings.cursorPos, minPos, maxPos, 100);
        ImGui::DragFloat("BPM", &settings.bpm, 1, 10, 3500);
        AddScalarScroll(ImGuiDataType_Float, &settings.bpm, 10, 3500, 1);
        ImGui::DragInt("Snapping", &settings.snapping, 1, 1, 192);
        AddScalarScroll(ImGuiDataType_S32, &settings.snapping, 1, 192, 1);
        int base = settings.useBase62 ? 62 : 36;
        int maxKeysound = base * base - 1;
        if (settings.startingKeysound > maxKeysound)
            settings.startingKeysound = maxKeysound;
        DragIntCustomBase("Starting key", &settings.startingKeysound, 1, 1, maxKeysound, base);
        AddScalarScroll(ImGuiDataType_S32, &settings.startingKeysound, 1, maxKeysound, 1);
        ImGui::SetItemTooltip("Decimal value: %d", settings.startingKeysound);
        ImGui::Checkbox("Enable base-62", &settings.useBase62);
        if (ImGui::Button("Zero-cross markers", ImVec2(-FLT_MIN, 0.0f))) {
            ZeroCrossMarkers(buffer, settings);
        }

        ImGui::SeparatorText("Export settings");
        char thres[64];
        if (settings.selectedGateThreshold == 0)
            snprintf(thres, 64, "Disabled");
        else
            snprintf(thres, 64, "%ddB", gateThresholds[settings.selectedGateThreshold]);
        const char* combo_preview_value = thres;
        if (ImGui::BeginCombo("Noise gate", combo_preview_value, 0))
        {
            for (int n = 0; n < IM_ARRAYSIZE(gateThresholds); n++)
            {
                if (n == 0)
                    snprintf(thres, 64, "Disabled");
                else
                    snprintf(thres, 64, "%ddB", gateThresholds[n]);
                const bool is_selected = (settings.selectedGateThreshold == n);
                if (ImGui::Selectable(thres, is_selected))
                    settings.selectedGateThreshold = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        AddScalarScroll(ImGuiDataType_S32, &settings.selectedGateThreshold, 0, IM_ARRAYSIZE(gateThresholds) - 1, 1);
        ImGui::DragInt("Fadeout", &settings.fadeout, 1, 0, 1000, "%dms");
        AddScalarScroll(ImGuiDataType_S32, &settings.fadeout, 0, 1000, 5);

        ImGui::SeparatorText("Process");
        if (ImGui::Button("Export keysounds", ImVec2(-FLT_MIN, 0.0f))) {
            WriteKeysounds(buffer, settings);
        }
    }
    ImGui::End();
}

void ProcessShortcuts(ImGuiIO& io, sf::SoundBuffer& buffer, sf::SoundBuffer& buffer2, sf::Sound& sound, SlicerSettings& settings, History& history) {
    if (!io.WantTextInput && !io.WantCaptureKeyboard && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false)) {
        OpenProject(buffer, settings);
        io.ClearInputKeys(); // Flush Ctrl key (it gets stuck otherwise)
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S), false)) {
        SaveProject(settings);
        io.ClearInputKeys();
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
        if (settings.cursorPos + settings.samplesPerSnap < buffer.getSampleCount()) {
            if (io.KeyShift) {
                settings.cursorPos += settings.samplesPerSnap - fmod(settings.cursorPos, settings.samplesPerSnap);
            }
            else {
                settings.cursorPos += settings.samplesPerSnap;
            }
        }
        if (sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }
    }
    else if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) && settings.cursorPos > 0.0) {
        if (settings.cursorPos - settings.samplesPerSnap < 0.0)
            settings.cursorPos = 0.0;
        else {
            double diff = fmod(settings.cursorPos, settings.samplesPerSnap);
            if (io.KeyShift && diff != 0.0) {
                settings.cursorPos -= diff;
            }
            else {
                settings.cursorPos -= settings.samplesPerSnap;
            }
        }
        if (sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }
    }
    if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) && settings.snapping < 192) {
        settings.snapping += 1;
    }
    else if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)) && settings.snapping > 1) {
        settings.snapping -= 1;
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
        if (io.KeyCtrl) {
            history.Undo(settings, buffer);
        }
        else {
            double e = settings.markers.find(settings.cursorPos);
            if (e != -1.0) {
                settings.markers.remove(e);
            }
            else {
                settings.markers.push_back(settings.cursorPos);
            }
            settings.updateHistory = true;
        }
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y))) {
        history.Redo(settings, buffer);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
        PlayKeysound(sound, buffer, buffer2, settings, false);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P))) {
        PlayKeysound(sound, buffer, buffer2, settings, true);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_M), false)) {
        WriteKeysounds(buffer, settings);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && !io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false)) {
        OpenAudioFile(buffer, settings);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_B), false)) {
        ProcessBMSEClipboard(buffer, settings);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), false)) {
        if (io.KeyCtrl) {
            HandleMarkerCopyPaste(settings, SelectionOperation::PASTE);
        }
        else {
            GenerateBMSEClipboard(buffer, settings, io.KeyShift);
        }
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false)) {
        if (io.KeyCtrl) {
            HandleMarkerCopyPaste(settings, SelectionOperation::COPY);
        }
        else {
            settings.markers.clear(io.KeyShift);
            settings.updateHistory = true;
        }
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X), false)) {
        HandleMarkerCopyPaste(settings, SelectionOperation::CUT);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_K), false)) {
        ExportKeysoundList(settings);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space), false)) {
        ManageSelection(settings);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete), false)) {
        HandleMarkerCopyPaste(settings, SelectionOperation::DEL);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home))) {
        settings.cursorPos = 0.0;
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End))) {
        size_t endPos = settings.samplesPerSnap * (int)(buffer.getSampleCount() / settings.samplesPerSnap);
        settings.cursorPos = endPos - settings.samplesPerSnap * (endPos > 0 && (size_t)(endPos - buffer.getSampleCount()) == 0);
    }
}

void DisplayMarkersTable(SlicerSettings& settings) {
    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 8);
    if (ImGui::BeginTable("markerstable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, outer_size))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        if (settings.markers.size() == 0) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::Text("No markers set...");
        }
        else {
            settings.markers.sort();
            double toRemove = -1.0;
            auto p = std::filesystem::u8path(settings.selectedFile);
            auto filename = p.filename().replace_extension().u8string();
            size_t idx = 0;
            for (auto& m : settings.markers) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                char bufId[64];
                size_t keysoundId = idx + settings.startingKeysound;
                ToBaseString(bufId, IM_ARRAYSIZE(bufId), &keysoundId, settings.useBase62 ? 62 : 36);
                ImGui::Text("%s", bufId);
                ImGui::TableNextColumn();

                char buf[64];
                snprintf(buf, 64, "%f", m.position);
                ImGui::Selectable(buf);
                if (ImGui::IsItemClicked(0)) {
                    settings.cursorPos = m.position;
                }
                else if (ImGui::IsItemClicked(1)) {
                    toRemove = m.position;
                }
                ImGui::TableNextColumn();

                bool edited = false;
                if (m.name.size() == 0) {
                    auto tmpName = GetTempMarkerName(filename, idx);
                    edited = SelectableInput(std::to_string(idx).c_str(), false, ImGuiSelectableFlags_None, &m.name[0], 4096, &tmpName[0]);
                }
                else {
                    edited = SelectableInput(std::to_string(idx).c_str(), false, ImGuiSelectableFlags_None, &m.name[0], 4096);
                }
                if (edited)
                    settings.updateHistory = true;
                m.name = &m.name[0]; // Awesome hack to update the string structure with the proper length and everything
                idx++;
            }
            if (toRemove != -1) {
                settings.markers.remove(toRemove);
                settings.updateHistory = true;
            }
        }
        ImGui::EndTable();
    }
}

void ShowWaveform(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Waveform"))
    {
        ImGui::SeparatorText("Waveform");
        DisplayWaveform(buffer, settings);

        ImGui::SeparatorText("Markers");
        DisplayMarkersTable(settings);
    }
    ImGui::End();
}

#if _WIN32 // Modified from https://gist.github.com/FRex/3f7b8d1ad1289a2117553ff3702f04af
LRESULT CALLBACK myCallback(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DROPFILES)
    {
        HDROP hdrop = reinterpret_cast<HDROP>(wParam);

        const UINT filescount = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
        if (filescount != 1) {
            std::cout << "Please drag only one file!" << std::endl;
        }
        else {
            const UINT bufsize = DragQueryFileW(hdrop, 0, NULL, 0);
            std::wstring str;
            str.resize(bufsize + 1);
            if (DragQueryFileW(hdrop, 0, &str[0], bufsize + 1))
            {
                std::string stdstr;
                sf::Utf8::fromWide(str.begin(), str.end(), std::back_inserter(stdstr));
                auto ext = stdstr.substr(stdstr.find_last_of(".") + 1);
                if (strcmp(ext.c_str(), "mid") == 0 || strcmp(ext.c_str(), "midi") == 0)
                    LoadMidi(*(sf::SoundBuffer*)bufferPtr, *(SlicerSettings*)settingsPtr, stdstr);
                else if (strcmp(ext.c_str(), "syp") == 0)
                    OpenProject(*(sf::SoundBuffer*)bufferPtr, *(SlicerSettings*)settingsPtr, stdstr);
                else if (strcmp(ext.c_str(), "txt") == 0)
                    ImportNamesFromMid2Bms(*(SlicerSettings*)settingsPtr, stdstr);
                else
                    OpenAudioFile(*(sf::SoundBuffer*)bufferPtr, *(SlicerSettings*)settingsPtr, stdstr);
            }
        }
        DragFinish(hdrop);
    }
    return CallWindowProcW(reinterpret_cast<WNDPROC>(originalSFMLCallback), handle, message, wParam, lParam);
}
#endif

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 456), "sayaslicer");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window, false);
    auto &io = ImGui::GetIO();

    SetupFonts(io);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    SetupImGuiStyle();
    ImPlot::CreateContext();

    sf::SoundBuffer buffer;
    sf::SoundBuffer buffer2; // For keysound playing
    sf::Sound sound;
    SlicerSettings settings;
    History history;
    settings.maxDisplayRange = minZoom;

    LoadPreferences(settings.prefs);

#if _WIN32
    HWND handle = window.getSystemHandle();
    DragAcceptFiles(handle, TRUE);
    originalSFMLCallback = SetWindowLongPtrW(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(myCallback));
    bufferPtr = (LONG_PTR)&buffer;
    settingsPtr = (LONG_PTR)&settings;

    SetConsoleOutputCP(CP_UTF8);
#endif

    window.resetGLStates();
    sf::Clock deltaClock;

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        SetupDock();
        ShowMainMenuBar(buffer, settings, window);

        ImGui::SetNextWindowClass(&window_class);
        ShowSettingsPanel(buffer, settings);

        ProcessShortcuts(io, buffer, buffer2, sound, settings, history);

        ImGui::SetNextWindowClass(&window_class);
        ShowWaveform(buffer, settings);

        ShowMidiTrackModal(buffer, settings);
        ShowPreferencesModal(settings.prefs);

        if (settings.updateHistory) {
            settings.updateHistory = false;
            history.AddItem(settings);
        }

        ImGui::RenderNotifications();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

#if _WIN32
    DragAcceptFiles(handle, FALSE);
#endif

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
}