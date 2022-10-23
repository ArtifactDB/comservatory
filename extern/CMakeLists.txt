cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)

FetchContent_Declare(
  byteme
  GIT_REPOSITORY https://github.com/LTLA/byteme
  GIT_TAG 9ae37144bb943c5d66e3c9b15d71257e6281adce
)

FetchContent_MakeAvailable(byteme)
