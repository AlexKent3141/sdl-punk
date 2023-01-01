cmake_minimum_required(VERSION 3.16.3)
project(punk)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME sdl2
  GIT_REPOSITORY https://github.com/efornara/sdl2
  GIT_TAG release-2.0.18)

add_library(punk
  include/punk.h
  src/punk.c)

target_include_directories(punk
  PUBLIC
    include/
  PRIVATE
    "${sdl2_SOURCE_DIR}/include")

target_link_libraries(punk
  PUBLIC
    sdl2)
