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
# Wii U portlibs ship static libraries only: the shared SDL2_image /
# SDL2_mixer imported targets either don't exist (mixer) or disagree with
# the static SDL2 (COMPATIBLE_INTERFACE_BOOL SDL2_SHARED), so link the
# -static targets there. Desktop keeps the shared targets.
if(WIIU)
    set(FNAE_SDL_IMAGE_LIB SDL2_image::SDL2_image-static)
    set(FNAE_SDL_MIXER_LIB SDL2_mixer::SDL2_mixer-static)
    # The wiiu sdl2_mixer config mangles -lstdc++ into -lstdc (its -l regex
    # drops the "++"), which the linker can't find. Swap in the real lib.
    get_target_property(_fnae_mixer_libs SDL2_mixer::SDL2_mixer-static INTERFACE_LINK_LIBRARIES)
    if(_fnae_mixer_libs)
        list(REMOVE_ITEM _fnae_mixer_libs stdc)
        list(APPEND _fnae_mixer_libs stdc++)
        set_target_properties(SDL2_mixer::SDL2_mixer-static PROPERTIES INTERFACE_LINK_LIBRARIES "${_fnae_mixer_libs}")
    endif()
else()
    set(FNAE_SDL_IMAGE_LIB SDL2_image::SDL2_image)
    set(FNAE_SDL_MIXER_LIB SDL2_mixer::SDL2_mixer)
endif()
target_link_libraries(FNaE_Native PRIVATE
    ${FNAE_SDL_IMAGE_LIB}
    ${FNAE_SDL_MIXER_LIB}
    SDL2::SDL2
)
# NOTE: static-link order matters (dependents first, SDL2 last); shared
# desktop builds don't care, so one order serves both.

# Wii U (WUT/CafeOS toolchain): ELF -> .rpx plus an Aroma .wuhb with the
# assets/ tree bundled as content, so a single file carries the game.
# Configure with:
#   /opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake -S . -B build-wiiu
if(WIIU)
    wut_create_rpx(FNaE_Native)

    set(NUSPACKER_JAR "${CMAKE_SOURCE_DIR}/NUSPacker/NUSPacker.jar")
    set(NUSPACKER_OUT "${CMAKE_CURRENT_BINARY_DIR}/FNaE_WUP")

    # wuhbtool reads the content dir at build time, so make sure it exists
    # at configure time (it is filled by the staging step below, which is
    # ordered before the .wuhb via the RPX target dependency).
    # Unpacked-title folder bundle: build-wiiu/FNaE holds ONLY code/,
    # content/, and meta/ — nothing loose at the root. Cemu boots
    # code/main.rpx; the game's vol/content asset probe (src/wiiu.h)
    # resolves content/assets no matter the working directory, so a bare
    # .rpx elsewhere can no longer die on missing assets.
    add_custom_command(TARGET FNaE_Native POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_Native.rpx"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/code/main.rpx"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/app.xml"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/code/app.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/cos.xml"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/code/cos.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/content/assets"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/meta.xml"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/meta/meta.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/iconTex.tga"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/meta/iconTex.tga"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/bootTvTex.tga"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/meta/bootTvTex.tga"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/assets/wiiu/bootDrcTex.tga"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/meta/bootDrcTex.tga"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory "${NUSPACKER_OUT}"
        COMMAND java -jar "${NUSPACKER_JAR}"
            -in "${CMAKE_CURRENT_BINARY_DIR}/FNaE"
            -out "${NUSPACKER_OUT}"
            -encryptKeyWith "D7B00402659BA2ABD2CB0DB27FA2B656"
        COMMENT "Staging Wii U folder bundle (FNaE/{code,content,meta})"
    )
    wut_create_wuhb(FNaE_Native
        NAME "Five Nights at Edward's U"
        SHORTNAME "FNaE U"
        AUTHOR "Alex347_XD"
        CONTENT "${CMAKE_CURRENT_BINARY_DIR}/FNaE"
    )
else()
# Self-contained run folder: build/FNaE/FNaE_Native.exe + assets + DLLs,
# so the game can be launched from there instead of the repo root.
set_target_properties(FNaE_Native PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/FNaE"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/FNaE"
)
endif()

# Assets are loaded via relative paths (e.g. "assets/images/179.png"),
# so mirror the repo's assets/ tree next to the executable.
# On Wii U the assets ride inside the .wuhb content (staged above) instead.
if(NOT WIIU)
add_custom_command(TARGET FNaE_Native POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:FNaE_Native>/assets"
    COMMENT "Copying assets to FNaE run folder"
)

# Bundle the full transitive DLL closure (SDL2 + libpng/libjpeg/...).
# SEARCH_DIRS is derived from the imported targets -- no hardcoded
# MSYS2 absolute paths, keeping this file portable.
# Wii U links the portlibs statically, so there is nothing to bundle.
add_custom_command(TARGET FNaE_Native POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        "-DTARGET_FILE=$<TARGET_FILE:FNaE_Native>"
        "-DTARGET_DIR=$<TARGET_FILE_DIR:FNaE_Native>"
        "-DSEARCH_DIRS=$<TARGET_FILE_DIR:SDL2::SDL2>;$<TARGET_FILE_DIR:SDL2_image::SDL2_image>;$<TARGET_FILE_DIR:SDL2_mixer::SDL2_mixer>"
        -P "${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeDeps.cmake"
    COMMENT "Copying runtime DLLs to FNaE run folder"
)
endif()

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
