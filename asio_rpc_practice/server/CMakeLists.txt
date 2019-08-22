cmake_minimum_required(VERSION 3.6)
project(boost_rpc_server)


find_package(Boost COMPONENTS system REQUIRED)
find_package(Protobuf REQUIRED)


set_source_files_properties(${PROJECT_BINARY_DIR}/../test.pb.cc PROPERTIES GENERATED 1)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR}/../)

set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CUR_DIR} SRC_LIST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety-attributes")
endif()

link_directories(${Boost_LIBRARY_DIRS} ${Protobuf_LIBRARY_DIRS})
add_executable(boost_rpc_server ${SRC_LIST} ${PROJECT_BINARY_DIR}/../test.pb.cc)
add_dependencies(boost_rpc_server protoc_source)
target_link_libraries(boost_rpc_server ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})