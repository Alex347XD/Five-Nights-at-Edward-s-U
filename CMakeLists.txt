cmake_minimum_required(VERSION 3.16)
project(FNaE_Native C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(FNaE_Native
    src/main.c
    src/fnae_core.c
    src/visuals.c
)

target_include_directories(FNaE_Native PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(FNaE_Native PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
)