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
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

include("${CMAKE_SOURCE_DIR}/scripts/CheckGit.cmake")
include_directories("include")
CheckGitSetup()

set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")

# Incluez les sous-projets.
add_subdirectory ("include/clip")
add_subdirectory ("include/midifile")
add_subdirectory ("include/i18n_keyval")
add_subdirectory ("sayaslicer")