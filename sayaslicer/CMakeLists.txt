# CMakeList.txt : projet CMake pour sayaslicer, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)

# Ajoutez une source à l'exécutable de ce projet.
add_executable (sayaslicer "sayaslicer.cpp" "sayaslicer.h")

# TODO: Ajoutez des tests et installez des cibles si nécessaire.
target_link_options(sayaslicer PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(sayaslicer PRIVATE sfml-main sfml-audio)
target_link_libraries(sayaslicer PRIVATE imgui::imgui)
target_link_libraries(sayaslicer PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(sayaslicer PRIVATE implot::implot)
target_link_libraries(sayaslicer PRIVATE tinyfiledialogs::tinyfiledialogs)