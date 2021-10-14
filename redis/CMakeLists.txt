cmake_minimum_required(VERSION 3.16)
project(Redis)

set(CMAKE_CXX_STANDARD 14)

include_directories(./include ..)

aux_source_directory(./src SOURCE_FILES)

add_library(Redis ${SOURCE_FILES})

target_link_libraries(Redis hiredis)