cmake_minimum_required(VERSION 3.9)

project(join_server)

add_library(sqlite3 SHARED sqlite/sqlite3.c sqlite/sqlite3.h)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads)

include_directories(${Boost_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(join_server main.cpp async_server.h dbadapter.h dbadapter.cpp)

target_link_libraries(join_server
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        sqlite3 )
