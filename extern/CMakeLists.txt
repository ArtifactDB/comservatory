cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)

FetchContent_Declare(
  byteme
  GIT_REPOSITORY https://github.com/LTLA/byteme
  GIT_TAG e95bc33c1a6677110582d6ac1c369de1fb286e8d
)

FetchContent_MakeAvailable(byteme)
