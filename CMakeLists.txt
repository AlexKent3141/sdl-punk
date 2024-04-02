cmake_minimum_required(VERSION 3.19)
project(punk)

option(BUILD_DEMO "Build the demo app" OFF)
option(SUPPORT_IMAGES "Link to SDL_image and support image-based widgets" ON)

if(BUILD_DEMO AND NOT SUPPORT_IMAGES)
  message("Error: cannot build the demo without image support")
  return()
endif()

# Download and build dependencies.
include(cmake/GetCPM.cmake)

# Set up some options for SDL2 dependencies.
set(SDL2TTF_SAMPLES OFF)
set(SDL2IMAGE_SAMPLES OFF)

CPMAddPackage("gh:libsdl-org/SDL#release-2.0.18")
CPMAddPackage("gh:libsdl-org/SDL_ttf#release-2.20.1")

add_library(punk SHARED)

target_sources(punk
  PRIVATE
  "include/punk.h"
  "src/punk_internal.h"
  "src/punk.c"
  "src/button.c"
  "src/label.c"
  "src/checkbox.c"
  "src/textbox.c"
  "src/font.h")

if(UNIX)
  target_compile_options(punk
    PRIVATE
      -Wall -Werror -Wpedantic -fvisibility=hidden)
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

if(SUPPORT_IMAGES)
  add_compile_definitions(WITH_IMAGES)

  CPMAddPackage("gh:libsdl-org/SDL_image#release-2.6.3")

  target_sources(punk
    PRIVATE
      "src/image.c"
      "src/image_button.c")

  target_link_libraries(punk
    PUBLIC
      SDL2_image)
endif()

if(BUILD_DEMO)
  # Create the demo executables.
  add_executable(punk_demo
    "src/demo.c")

  target_link_libraries(punk_demo
    PRIVATE
      punk)

  add_executable(punk_demo_grid
    "src/demo_grid.c")

  target_link_libraries(punk_demo_grid
    PRIVATE
      punk)

  # Need access to the res folder.
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/res" DESTINATION ${CMAKE_BINARY_DIR})
endif()
