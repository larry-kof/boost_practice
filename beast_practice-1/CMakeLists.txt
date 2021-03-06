cmake_minimum_required(VERSION 3.6)
project(beast_server)

find_package(Boost COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory("${CUR_DIR}/beast_practice" SRC_LIST)

message(STATUS ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")
endif()

link_directories(${Boost_LIBRARY_DIRS})
add_executable(beast_server ${SRC_LIST})
target_link_libraries(beast_server ${Boost_LIBRARIES})