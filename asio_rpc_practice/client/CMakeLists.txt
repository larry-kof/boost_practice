cmake_minimum_required(VERSION 3.6)
project(boost_rpc_client)

set(BOOST_ROOT "/usr/local/")

find_package(Boost COMPONENTS system REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})

set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CUR_DIR} SRC_LIST)

message(STATUS ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")
endif()

link_directories(${Boost_LIBRARY_DIRS} ${Protobuf_LIBRARY_DIRS})
add_executable(boost_rpc_client ${SRC_LIST}
                ${CUR_DIR}/../common/test.pb.cc)
target_link_libraries(boost_rpc_client ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})