cmake_minimum_required(VERSION 3.16.3)
project(punk)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SDL2
  GIT_REPOSITORY https://github.com/efornara/sdl2
  GIT_TAG release-2.0.18)

add_library(punk SHARED
  include/punk.h
  src/punk.c)

target_include_directories(punk
  PUBLIC
    include/
    "${SDL2_SOURCE_DIR}/include")

target_link_libraries(punk
  PUBLIC
    SDL2)

# Create the demo executable.
add_executable(punk_demo
  src/demo.c)

target_link_libraries(punk_demo
  PRIVATE
    punk)
