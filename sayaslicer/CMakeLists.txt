# CMakeList.txt : projet CMake pour sayaslicer, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)

# Ajoutez une source à l'exécutable de ce projet.
add_executable (sayaslicer "sayaslicer.cpp" "sayaslicer.h" "bmseclipboard.hpp" "theme.hpp" "custom_widgets.hpp" "settings.hpp" "marker.hpp" "base_convert.hpp" "selection.hpp" "history.hpp" "audio.hpp" "utils.hpp")

# TODO: Ajoutez des tests et installez des cibles si nécessaire.
target_link_options(sayaslicer PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_compile_features(sayaslicer PRIVATE cxx_std_17)
if (APPLE)
    target_link_libraries(sayaslicer PRIVATE "-framework CoreAudio")
    target_link_libraries(sayaslicer PRIVATE "-framework AudioUnit")
    target_link_libraries(sayaslicer PRIVATE "-framework AudioToolbox")
endif()
target_link_libraries(sayaslicer PRIVATE sfml-audio)
target_link_libraries(sayaslicer PRIVATE imgui::imgui)
target_link_libraries(sayaslicer PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(sayaslicer PRIVATE implot::implot)
target_link_libraries(sayaslicer PRIVATE tinyfiledialogs::tinyfiledialogs)
target_link_libraries(sayaslicer PRIVATE clip)
target_link_libraries(sayaslicer PRIVATE cereal::cereal)
target_link_libraries(sayaslicer PRIVATE midifile)