cmake_minimum_required(VERSION 3.6)
project(tcp_server)

set(BOOST_ROOT "/usr/local/")

find_package(Boost COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CUR_DIR} SRC_LIST)

message(STATUS ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")
endif()

link_directories(${Boost_LIBRARY_DIRS})
add_executable(tcp_server ${SRC_LIST})
target_link_libraries(tcp_server ${Boost_LIBRARIES} pthread)