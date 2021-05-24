cmake_minimum_required(VERSION 3.17)
project(Utils)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

aux_source_directory(src SOURCE_FILES)

add_library(Utils ${SOURCE_FILES})