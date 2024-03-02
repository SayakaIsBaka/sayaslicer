# CMakeList.txt : fichier projet CMake de niveau supérieur, effectuez une configuration globale
# et incluez les sous-projets ici.
#
cmake_minimum_required (VERSION 3.8)

project ("sayaslicer")

find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)

include_directories("include")

set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")

# Incluez les sous-projets.
add_subdirectory ("include/clip")
add_subdirectory ("include/midifile")
add_subdirectory ("sayaslicer")