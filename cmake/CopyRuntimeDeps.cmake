# Copies runtime DLL dependencies next to the built executable.
#
# Expected -D variables (passed from CMakeLists.txt POST_BUILD step):
#   TARGET_FILE : full path to FNaE_Native.exe
#   TARGET_DIR  : directory to copy DLLs into (build/FNaE)
#   SEARCH_DIRS : ";"-separated list of extra directories to search
#                 (SDL2 / SDL2_image / SDL2_mixer bin dirs, no hardcoded MSYS2 paths)
#
# Uses file(GET_RUNTIME_DEPENDENCIES) so the full transitive closure
# (libpng, libjpeg, libwebp, libavif, mpg123, vorbis, ...) is collected,
# not just the direct SDL2/SDL2_image/SDL2_mixer DLLs.

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

if(NOT DEFINED TARGET_FILE OR NOT DEFINED TARGET_DIR)
  message(FATAL_ERROR "CopyRuntimeDeps.cmake requires TARGET_FILE and TARGET_DIR")
endif()

# SEARCH_DIRS arrives as a semicolon-separated string; keep it a list.
set(_search_dirs ${SEARCH_DIRS})

file(GET_RUNTIME_DEPENDENCIES
  EXECUTABLES "${TARGET_FILE}"
  RESOLVED_DEPENDENCIES_VAR _resolved
  UNRESOLVED_DEPENDENCIES_VAR _unresolved
  DIRECTORIES ${_search_dirs}
  PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
  POST_EXCLUDE_REGEXES ".*[Ss]ystem32.*"
)

if(_unresolved)
  message(WARNING "Unresolved runtime dependencies: ${_unresolved}")
endif()

foreach(_dep IN LISTS _resolved)
  # Only bundle DLLs (skip static system libs that may resolve to .lib/.a).
  if(NOT _dep MATCHES "\\.[Dd][Ll][Ll]$")
    continue()
  endif()
  get_filename_component(_name "${_dep}" NAME)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_dep}" "${TARGET_DIR}/${_name}"
    RESULT_VARIABLE _copy_result
  )
  if(NOT _copy_result EQUAL 0)
    message(WARNING "Failed to copy ${_dep} to ${TARGET_DIR}/${_name}")
  endif()
endforeach()
