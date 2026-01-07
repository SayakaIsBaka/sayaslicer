cmake_minimum_required (VERSION 3.10)

string(TIMESTAMP VERSIONDATE "%Y-%m-%d")

if (APPLE)
    set(CMAKE_MACOSX_BUNDLE ON)
endif()

add_executable (sayaslicer "sayaslicer.cpp" "sayaslicer.hpp" "bmseclipboard.hpp" "theme.hpp" "custom_widgets.hpp" "settings.hpp" "marker.hpp" "base_convert.hpp" "selection.hpp" "history.hpp" "audio.hpp" "utils.hpp" "waveform.hpp" "project.hpp" "midi.hpp" "copy_paste.hpp" "audio.cpp" "base_convert.cpp" "utils.cpp" "bmseclipboard.cpp" "copy_paste.cpp" "custom_widgets.cpp" "history.cpp" "marker.cpp" "midi.cpp" "project.cpp" "selection.cpp" "theme.cpp" "waveform.cpp" "preferences.hpp" "preferences.cpp" "translations.hpp" "translations.cpp" "notifications.hpp" "about.cpp" "markdown.hpp" "markdown.cpp" "console.hpp" "console.cpp" "dropmanager.hpp" "sound_buffer.cpp")
target_compile_definitions(sayaslicer PRIVATE VERSIONDATE="${VERSIONDATE}")

target_link_libraries(sayaslicer PRIVATE git_version)

target_link_options(sayaslicer PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_compile_features(sayaslicer PRIVATE cxx_std_17)
if (APPLE)
    target_link_libraries(sayaslicer PRIVATE glfw OpenGL::GL portaudio_static)
elseif (WIN32)
    target_link_libraries(sayaslicer PRIVATE glfw opengl32.lib portaudio)
else()
	target_link_libraries(sayaslicer PRIVATE glfw OpenGL portaudio_static)
endif()
target_link_libraries(sayaslicer PRIVATE imgui::imgui)
target_link_libraries(sayaslicer PRIVATE implot::implot)
target_link_libraries(sayaslicer PRIVATE tinyfiledialogs::tinyfiledialogs)
target_link_libraries(sayaslicer PRIVATE clip)
target_link_libraries(sayaslicer PRIVATE cereal::cereal)
target_link_libraries(sayaslicer PRIVATE midifile)
target_link_libraries(sayaslicer PRIVATE i18n_keyval)
target_link_libraries(sayaslicer PRIVATE cpr::cpr)
target_link_libraries(sayaslicer PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(sayaslicer PRIVATE Iconv::Iconv)
target_link_libraries(sayaslicer PRIVATE SndFile::sndfile)

if (APPLE)
    set_target_properties(sayaslicer PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_BUNDLE_NAME sayaslicer
        MACOSX_BUNDLE_BUNDLE_VERSION ${VERSIONDATE}
        MACOSX_BUNDLE_GUI_IDENTIFIER io.lovelyrad.sayaslicer
        MAXOSX_BUNDLE_ICON_FILE sayaslicer.icns
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSIONDATE}
    )
    install(TARGETS sayaslicer BUNDLE DESTINATION .)
    file(COPY "../lang" DESTINATION sayaslicer.app/Contents/Resources)
    file(COPY "../assets/sayaslicer.icns" DESTINATION sayaslicer.app/Contents/Resources)
else()
    # Copy translations to dest folder
    file(COPY "../lang" DESTINATION .)
endif()