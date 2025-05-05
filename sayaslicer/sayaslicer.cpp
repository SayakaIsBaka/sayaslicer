#include "sayaslicer.hpp"

#if _WIN32
    #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") // Hide console on Windows
#else
    #include <unistd.h> // For usleep
#endif

using namespace std;
using namespace i18n::literals;

void ShowMenuFile(SoundBuffer& buffer, SlicerSettings &settings, GLFWwindow* window)
{
    if (ImGui::MenuItem("open_project"_t.c_str(), "Ctrl+O")) {
        OpenProject(buffer, settings);
    }
    if (ImGui::MenuItem("save_project"_t.c_str(), "Ctrl+S")) {
        SaveProject(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("import_audio_file"_t.c_str(), "O")) {
        OpenAudioFile(buffer, settings);
    }
    if (ImGui::MenuItem("export_keysounds"_t.c_str(), "M")) {
        WriteKeysounds(buffer, settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("quit"_t.c_str(), "Alt+F4")) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void ShowMenuEdit(SoundBuffer& buffer, SlicerSettings& settings)
{
    if (ImGui::MenuItem("import_midi"_t.c_str())) {
        LoadMidi(buffer, settings);
    }
    if (ImGui::MenuItem("import_mid2bms"_t.c_str())) {
        ImportNamesFromMid2Bms(settings);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("export_bmse"_t.c_str(), "V")) {
        GenerateBMSEClipboard(buffer, settings, false);
    }
    if (ImGui::MenuItem("export_ibmsc"_t.c_str(), "Shift+V")) {
        GenerateBMSEClipboard(buffer, settings, true);
    }
    if (ImGui::MenuItem("import_bmse"_t.c_str(), "B")) {
        ProcessBMSEClipboard(buffer, settings);
    }
    if (ImGui::MenuItem("export_keysound_list"_t.c_str(), "K")) {
        ExportKeysoundList(settings, false);
    }
    if (ImGui::MenuItem("append_keysound_list_bms"_t.c_str(), "Shift+K")) {
        ExportKeysoundList(settings, true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("clear_all_markers"_t.c_str(), "C")) {
        settings.markers.clear(false);
        settings.updateHistory = true;
    }
    if (ImGui::MenuItem("clear_all_markers_with_0"_t.c_str(), "Shift+C")) {
        settings.markers.clear(true);
        settings.updateHistory = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("preferences"_t.c_str())) {
        settings.prefs.openPreferencesModalTemp = true;
    }
}

void ShowMainMenuBar(SoundBuffer& buffer, SlicerSettings &settings, GLFWwindow* window)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("file"_t.c_str()))
        {
            ShowMenuFile(buffer, settings, window);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("edit"_t.c_str()))
        {
            ShowMenuEdit(buffer, settings);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("help_menu"_t.c_str()))
        {
            if (ImGui::MenuItem("about"_t.c_str())) {
                settings.openAboutModalTemp = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void SetupFonts(ImGuiIO& io, int fontSize) {
    float mainFontSize = (float)fontSize;
    float iconFontSize = mainFontSize * 2.0f / 3.0f;

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText(u8"←→↑↓");
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.BuildRanges(&ranges);
    io.Fonts->AddFontFromMemoryCompressedTTF(noto_compressed_data, noto_compressed_size, mainFontSize, 0, ranges.Data);

    ImFontConfig krConfig;
    krConfig.MergeMode = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(notokr_compressed_data, notokr_compressed_size, mainFontSize, &krConfig, io.Fonts->GetGlyphRangesKorean());

    ImFontConfig scConfig;
    scConfig.MergeMode = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(notosc_compressed_data, notosc_compressed_size, mainFontSize, &scConfig, io.Fonts->GetGlyphRangesChineseFull());

    static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);

    io.Fonts->Build();
}

void SetupDock() {
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    static bool init = true;
    ImGuiID dock_id_left, dock_id_right;
    if (init && ImGui::GetCurrentContext()->SettingsIniData.size() == 0) {
        init = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_id_left, &dock_id_right);
        ImGui::DockBuilderDockWindow("Settings", dock_id_left);
        ImGui::DockBuilderDockWindow("Waveform", dock_id_right);
        ImGui::DockBuilderDockWindow("Console", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }
}

void ShowSettingsPanel(SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Settings"))
    {
        double minPos = 0;
        double maxPos = buffer.getSampleCount();
        static float zoom = 0;

        ImGui::SeparatorText("general"_t.c_str());
        ImGui::SliderFloat("zoom"_t.c_str(), &zoom, 0, minZoom - waveformReso * 2.0);
        AddScalarScroll(ImGuiDataType_Float, &zoom, 0, minZoom - waveformReso * 2.0, 200);
        settings.maxDisplayRange = minZoom - zoom;
        ImGui::DragInt("offset"_t.c_str(), &settings.offset, 1, 0, 1000);
        AddScalarScroll(ImGuiDataType_S32, &settings.offset, 0, 1000, 10);
        ImGui::DragScalar("position"_t.c_str(), ImGuiDataType_Double, &settings.cursorPos, 1, &minPos, &maxPos);
        AddScalarScroll(ImGuiDataType_Double, &settings.cursorPos, minPos, maxPos, 100);
        ImGui::DragFloat("bpm"_t.c_str(), &settings.bpm, 1, 10, 3500);
        AddScalarScroll(ImGuiDataType_Float, &settings.bpm, 10, 3500, 1);
        ImGui::DragInt("snapping"_t.c_str(), &settings.snapping, 1, 1, 192);
        AddScalarScroll(ImGuiDataType_S32, &settings.snapping, 1, 192, 1);
        int base = settings.useBase62 ? 62 : 36;
        int maxKeysound = base * base - 1;
        if (settings.startingKeysound > maxKeysound)
            settings.startingKeysound = maxKeysound;
        if (settings.startingKeysound <= 0)
            settings.startingKeysound = 1;
        DragIntCustomBase("starting_key"_t.c_str(), &settings.startingKeysound, 1, 1, maxKeysound, base);
        AddScalarScroll(ImGuiDataType_S32, &settings.startingKeysound, 1, maxKeysound, 1);
        ImGui::SetItemTooltip("%s: %d", "decimal_value"_t.c_str(), settings.startingKeysound);
        ImGui::Checkbox("enable_b62"_t.c_str(), &settings.useBase62);
        if (ImGui::Button("zerocross_markers"_t.c_str(), ImVec2(-FLT_MIN, 0.0f))) {
            ZeroCrossMarkers(buffer, settings);
        }

        ImGui::SeparatorText("export_settings"_t.c_str());
        char thres[64];
        if (settings.selectedGateThreshold == 0)
            snprintf(thres, 64, "disabled"_t.c_str());
        else
            snprintf(thres, 64, "%ddB", gateThresholds[settings.selectedGateThreshold]);
        const char* combo_preview_value = thres;
        if (ImGui::BeginCombo("noise_gate"_t.c_str(), combo_preview_value, 0))
        {
            for (int n = 0; n < IM_ARRAYSIZE(gateThresholds); n++)
            {
                if (n == 0)
                    snprintf(thres, 64, "disabled"_t.c_str());
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
        ImGui::DragInt("fadeout"_t.c_str(), &settings.fadeout, 1, 0, 1000, "%dms");
        AddScalarScroll(ImGuiDataType_S32, &settings.fadeout, 0, 1000, 5);
        ImGui::DragInt("keysound_end_offset"_t.c_str(), &settings.keysoundOffsetEnd, 1, -1000, 1000, "%dms");
        AddScalarScroll(ImGuiDataType_S32, &settings.keysoundOffsetEnd, -1000, 1000, 5);

        ImGui::SeparatorText("process"_t.c_str());
        if (ImGui::Button("export_keysounds"_t.c_str(), ImVec2(-FLT_MIN, 0.0f))) {
            WriteKeysounds(buffer, settings);
        }
    }
    ImGui::End();
}

void ProcessShortcuts(ImGuiIO& io, SoundBuffer& buffer, SlicerSettings& settings, History& history) {
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
        if (buffer.isPlaying()) {
            buffer.stop();
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
        if (buffer.isPlaying()) {
            buffer.stop();
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
        PlayKeysound(buffer, settings, false);
    }
    if (!io.WantTextInput && !io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P))) {
        PlayKeysound(buffer, settings, true);
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
        ExportKeysoundList(settings, io.KeyShift);
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
        ImGui::TableSetupColumn("id"_t.c_str(), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("position"_t.c_str(), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("name"_t.c_str(), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        if (settings.markers.size() == 0) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::Text("no_markers_set"_t.c_str());
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

void ShowWaveform(SoundBuffer& buffer, SlicerSettings& settings) {
    if (ImGui::Begin("Waveform"))
    {
        ImGui::SeparatorText("waveform"_t.c_str());
        DisplayWaveform(buffer, settings);

        ImGui::SeparatorText("markers"_t.c_str());
        DisplayMarkersTable(settings);
    }
    ImGui::End();
}

void ShowConsole(ConsoleLog &console) {
    if (ImGui::Begin("Console"))
    {
        ImGui::SeparatorText("console"_t.c_str());
        console.Draw();
    }
    ImGui::End();
}

void HandleDragDropDispatch(SoundBuffer& buffer, SlicerSettings& settings, std::string path) {
    auto ext = path.substr(path.find_last_of(".") + 1);
    if (strcmp(ext.c_str(), "mid") == 0 || strcmp(ext.c_str(), "midi") == 0)
        LoadMidi(buffer, settings, path);
    else if (strcmp(ext.c_str(), "syp") == 0)
        OpenProject(buffer, settings, path);
    else if (strcmp(ext.c_str(), "txt") == 0)
        ImportNamesFromMid2Bms(settings, path);
    else
        OpenAudioFile(buffer, settings, path);
}

// Copied from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-opengl-users
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#if defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow* window = glfwCreateWindow(870, 477, "sayaslicer", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    ImGui::CreateContext();
    auto &io = ImGui::GetIO();

    if (Pa_Initialize() != paNoError) {
        throw std::runtime_error("Error initializing PortAudio");
    }

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    SetupImGuiStyle();
    ImPlot::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SoundBuffer buffer;
    Texture logo;
    bool ret = LoadTextureFromMemory(logoArr, IM_ARRAYSIZE(logoArr), &logo.texture, &logo.width, &logo.height);
    IM_ASSERT(ret);
    SlicerSettings settings;
    History history;
    ConsoleLog consoleLog;
    settings.maxDisplayRange = minZoom;

    std::stringstream logBuffer;
    std::streambuf* oldCout = std::cout.rdbuf(logBuffer.rdbuf());
    std::streambuf* oldCerr = std::cerr.rdbuf(logBuffer.rdbuf());

    LoadPreferences(settings.prefs);
    InitTranslations(settings.prefs.language);

    SetupFonts(io, settings.prefs.fontSize);

#if _WIN32
    OleInitialize(NULL);
    HWND handle = glfwGetWin32Window(window);
    DropManager dm;
    RegisterDragDrop(handle, &dm);

    SetConsoleOutputCP(CP_UTF8);
#endif

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;

    if (settings.prefs.checkForUpdates)
        CheckUpdates(settings, true);
    int frame = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
#ifdef _WIN32
            ::Sleep(10);
#else
            usleep(10 * 1000);
#endif
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        SetupDock();
        ShowMainMenuBar(buffer, settings, window);

#if _WIN32
        if (dm.droppedFile.size() != 0) {
            HandleDragDropDispatch(buffer, settings, dm.droppedFile);
            dm.droppedFile = "";
        }
#endif

        ImGui::SetNextWindowClass(&window_class);
#if _WIN32
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, dm.isDragged);
#endif
        ShowSettingsPanel(buffer, settings);
#if _WIN32
        ImGui::PopItemFlag();
#endif

        ProcessShortcuts(io, buffer, settings, history);

        ImGui::SetNextWindowClass(&window_class);
        if (frame < 2) { // Issue #5289, needed to set tab focus to waveform on init
            if (frame == 1)
                ImGui::SetNextWindowFocus();
            frame += 1;
        }
        ShowWaveform(buffer, settings);

        ShowConsole(consoleLog);

        ShowMidiTrackModal(buffer, settings);
        ShowPreferencesModal(settings.prefs);
        ShowAbout(settings, logo);

        if (settings.updateHistory) {
            settings.updateHistory = false;
            history.AddItem(settings);
        }

        if (!logBuffer.str().empty()) {
            auto time = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(time);
            consoleLog.AddLog("[+] %s", std::ctime(&time_t));
            consoleLog.AddLog("%s\n", logBuffer.str().c_str());
            logBuffer.str(""); // Clear stream
        }

        ImGui::RenderNotifications();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

#if _WIN32
    RevokeDragDrop(handle);
    OleUninitialize();
#endif

    if (Pa_Terminate() != paNoError) {
        throw std::runtime_error("Error terminating PortAudio");
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}