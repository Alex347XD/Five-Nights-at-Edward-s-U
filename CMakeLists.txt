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

    # wuhbtool requires the content dir to exist at configure time; the
    # staging target below fills it on every build (and the bundle cleans
    # it afterwards). Nothing else is staged: deliverables are just
    # FNaE_U.wuhb + FNaE_U.zip (plus the .elf/.rpx link products).
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FNaE/content")
    # Stage the assets tree wuhbtool bundles (FNaE/content/assets) on EVERY
    # build, ordered before the .wuhb via target dependency. A link
    # POST_BUILD cannot be relied on: FNaE/ is cleaned after each build,
    # so an up-to-date .rpx would leave wuhbtool with nothing to bundle.
    add_custom_target(FNaE_wuhb_content ALL
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE/content/assets"
        COMMENT "Staging Wii U content (assets for .wuhb)"
    )
    wut_create_wuhb(FNaE_Native
        NAME "Five Nights at Edward's U"
        SHORTNAME "FNaE U"
        AUTHOR "Alex347_XD"
        # 1. Point this to the STAGED directory so it carries the internal /assets subfolder layout
        CONTENT "${CMAKE_CURRENT_BINARY_DIR}/FNaE/content" 
        # 2. Make sure these target the clean, uncompressed PNG files directly
        ICON "${CMAKE_SOURCE_DIR}/assets/wiiu/iconTex.png"
        TV_SPLASH "${CMAKE_SOURCE_DIR}/assets/wiiu/bootTvTex.png"
        DRC_SPLASH "${CMAKE_SOURCE_DIR}/assets/wiiu/bootDrcTex.png"
    )
    add_dependencies(FNaE_Native_wuhb FNaE_wuhb_content)
    # Deliverables: FNaE_U.wuhb (Aroma single file) + FNaE_U.zip (HBL SD
    # drop-in: wiiu/apps/FNaE_U/). The wiiu/ staging dir is zipped first,
    # then removed along with the other intermediates, so only the two
    # files (plus the .elf/.rpx link products) remain in build-wiiu/.
    add_custom_target(FNaE_U_bundle ALL
        DEPENDS FNaE_Native_wuhb
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_U"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/wiiu"
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_CURRENT_BINARY_DIR}/wiiu/apps/FNaE_U"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_Native.wuhb"
            "${CMAKE_CURRENT_BINARY_DIR}/wiiu/apps/FNaE_U/FNaE_U.wuhb"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_Native.wuhb"
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_U.wuhb"
        COMMAND "${CMAKE_COMMAND}" -E rm -f
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_Native.wuhb"
        COMMAND "${CMAKE_COMMAND}" -E tar "cf" "FNaE_U.zip" "--format=zip" "--" "wiiu"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/content"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/wiiu"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/FNaE_WUP"
        COMMENT "Staging FNaE_U.wuhb + FNaE_U.zip, cleaning intermediates"
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

# Assets are loaded via relative paths (e.g. "assets/images/227.png"),
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
