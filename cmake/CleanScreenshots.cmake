# Removes generated headless screenshots, preserving .gitkeep files.
#
# Expected -D variables:
#   SOURCE_SCREENSHOTS_DIR : repo screenshots/ dir (tracked, .gitkeep kept)
#   BUNDLE_SCREENSHOTS_DIR : build/FNaE/screenshots/ dir (inside git-ignored
#                            build tree, removed entirely if present)
#   LEGACY_FILES           : ";"-separated list of legacy screenshot files
#                            from before the screenshots/ convention

if(NOT DEFINED SOURCE_SCREENSHOTS_DIR OR NOT DEFINED BUNDLE_SCREENSHOTS_DIR)
  message(FATAL_ERROR "CleanScreenshots.cmake requires SOURCE_SCREENSHOTS_DIR and BUNDLE_SCREENSHOTS_DIR")
endif()

file(GLOB _generated
  "${SOURCE_SCREENSHOTS_DIR}/*.png"
  "${SOURCE_SCREENSHOTS_DIR}/*.bmp"
)
foreach(_f IN LISTS _generated)
  get_filename_component(_name "${_f}" NAME)
  if(_name STREQUAL ".gitkeep")
    continue()
  endif()
  file(REMOVE "${_f}")
  message(STATUS "Removed ${_f}")
endforeach()

foreach(_f IN LISTS LEGACY_FILES)
  if(EXISTS "${_f}" AND NOT IS_DIRECTORY "${_f}")
    file(REMOVE "${_f}")
    message(STATUS "Removed ${_f}")
  endif()
endforeach()

if(EXISTS "${BUNDLE_SCREENSHOTS_DIR}")
  file(REMOVE_RECURSE "${BUNDLE_SCREENSHOTS_DIR}")
  message(STATUS "Removed ${BUNDLE_SCREENSHOTS_DIR}")
endif()
