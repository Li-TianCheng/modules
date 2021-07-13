cmake_minimum_required(VERSION 3.16)
project(Modules)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(mem_pool)

add_subdirectory(event_system)

add_subdirectory(time_system)

add_subdirectory(task_system)

add_subdirectory(my_pthread)

add_subdirectory(utils)

add_subdirectory(net)

add_subdirectory(http)

add_subdirectory(my_sql)

add_subdirectory(log)

add_library(Modules Modules.h)

target_link_libraries(Modules Log Http MySql Net Utils TaskSystem TimeSystem EventSystem MemPool MyPthread)