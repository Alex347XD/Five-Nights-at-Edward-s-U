cmake_minimum_required(VERSION 3.16)
project(FNaE_Native C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

add_executable(FNaE_Native
    src/main.c
    src/fnae_core.c
    src/headless.c
    src/visuals.c
    src/audio.c
    src/save.c
)

target_include_directories(FNaE_Native PRIVATE src)
target_link_libraries(FNaE_Native PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
)

# Self-contained run folder: build/FNaE/FNaE_Native.exe + assets + DLLs,
# so the game can be launched from there instead of the repo root.
set_target_properties(FNaE_Native PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/FNaE"
)

# Assets are loaded via relative paths (e.g. "assets/images/179.png"),
# so mirror the repo's assets/ tree next to the executable.
add_custom_command(TARGET FNaE_Native POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:FNaE_Native>/assets"
    COMMENT "Copying assets to FNaE run folder"
)

# Bundle the full transitive DLL closure (SDL2 + libpng/libjpeg/...).
# SEARCH_DIRS is derived from the imported targets -- no hardcoded
# MSYS2 absolute paths, keeping this file portable.
add_custom_command(TARGET FNaE_Native POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        "-DTARGET_FILE=$<TARGET_FILE:FNaE_Native>"
        "-DTARGET_DIR=$<TARGET_FILE_DIR:FNaE_Native>"
        "-DSEARCH_DIRS=$<TARGET_FILE_DIR:SDL2::SDL2>;$<TARGET_FILE_DIR:SDL2_image::SDL2_image>;$<TARGET_FILE_DIR:SDL2_mixer::SDL2_mixer>"
        -P "${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeDeps.cmake"
    COMMENT "Copying runtime DLLs to FNaE run folder"
)

# Headless screenshot cleanup: wipes generated images under the tracked
# screenshots/ dir (keeping .gitkeep), legacy CWD screenshots from before
# the screenshots/ convention, and the bundle's screenshots/ output.
add_custom_target(clean-screenshots
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_SCREENSHOTS_DIR=${CMAKE_SOURCE_DIR}/screenshots"
        "-DBUNDLE_SCREENSHOTS_DIR=${CMAKE_BINARY_DIR}/FNaE/screenshots"
        "-DLEGACY_FILES=${CMAKE_SOURCE_DIR}/headless.png;${CMAKE_SOURCE_DIR}/title.png;${CMAKE_BINARY_DIR}/FNaE/headless.png;${CMAKE_BINARY_DIR}/FNaE/title.png"
        -P "${CMAKE_SOURCE_DIR}/cmake/CleanScreenshots.cmake"
    COMMENT "Removing generated headless screenshots"
)
