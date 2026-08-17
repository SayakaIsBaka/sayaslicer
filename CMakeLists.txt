# CMakeList.txt : fichier projet CMake de niveau supérieur, effectuez une configuration globale
# et incluez les sous-projets ici.
#
cmake_minimum_required (VERSION 3.10)

project ("sayaslicer")

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(Iconv)

include("${CMAKE_SOURCE_DIR}/scripts/CheckGit.cmake")
include_directories("include")
CheckGitSetup()

set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")

include(FetchContent)
FetchContent_Declare(
    miniaudio
    URL https://github.com/mackron/miniaudio/archive/refs/tags/0.11.25.zip
    URL_MD5 09736fdb2c05f393a69ac3029af81a48
)
FetchContent_MakeAvailable(miniaudio)
target_compile_definitions(miniaudio PUBLIC
    MA_NO_DECODING
    MA_NO_ENCODING
    MA_NO_WAV
    MA_NO_FLAC
    MA_NO_MP3
    MA_NO_RESOURCE_MANAGER
    MA_NO_GENERATION
)

# Incluez les sous-projets.
add_subdirectory ("include/clip")
add_subdirectory ("include/midifile")
add_subdirectory ("include/i18n_keyval")
add_subdirectory ("sayaslicer")