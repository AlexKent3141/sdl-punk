cmake_minimum_required(VERSION 3.16.3)
project(punk)

add_library(punk
  include/punk.h
  src/punk.c)

target_include_directories(punk
  PUBLIC
    include/)
