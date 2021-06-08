cmake_minimum_required(VERSION 3.17)
project(MySql)

set(CMAKE_CXX_STANDARD 14)

include_directories(./include ..)

aux_source_directory(./src SOURCE_FILES)

add_library(MySql ${SOURCE_FILES})

target_link_libraries(MySql mysqlclient)
