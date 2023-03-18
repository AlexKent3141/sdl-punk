cmake_minimum_required(VERSION 3.19)
project(punk)

option(BUILD_DEMO "Build the demo app" ON)

# Download and build dependencies.
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.0.18)

CPMAddPackage(
  NAME SDL2_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG release-2.20.1)

add_library(punk SHARED
  include/punk.h
  src/punk_internal.h
  src/punk.c
  src/button.c
  src/label.c
  src/checkbox.c
  src/font.h)

if(UNIX)
  target_compile_options(punk
    PRIVATE
      -Wall -Werror -Wpedantic)
endif()

target_include_directories(punk
  PUBLIC
    include/
    "${SDL2_SOURCE_DIR}/include"
    "${SDL2_ttf_SOURCE_DIR}")

target_link_libraries(punk
  PUBLIC
    SDL2
    SDL2_ttf)

if(BUILD_DEMO)
  # Create the demo executables.
  add_executable(punk_demo
    src/demo.c)

  target_link_libraries(punk_demo
    PRIVATE
      punk)

  add_executable(punk_demo_grid
    src/demo_grid.c)

  target_link_libraries(punk_demo_grid
    PRIVATE
      punk)
endif()
