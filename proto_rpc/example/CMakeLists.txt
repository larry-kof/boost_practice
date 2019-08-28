cmake_minimum_required(VERSION 3.6)
project(example)

if(NOT ("${BOOST_ROOT_PATH}" STREQUAL ""))
    set(BOOST_ROOT ${BOOST_ROOT_PATH})
endif()
find_package(Boost 1.69 COMPONENTS system REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)

if(USE_SSL)
    add_definitions(-DUSE_SSL)
endif()
set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${PROJECT_BINARY_DIR})
add_custom_command(OUTPUT test.pb.cc test.pb.h
COMMAND protoc
ARGS --cpp_out . -I${CUR_DIR} ${CUR_DIR}/test.proto
DEPENDS ${CUR_DIR}/test.proto)

add_custom_target(example_proto_source ALL DEPENDS test.pb.cc test.pb.h)

set_source_files_properties(${PROJECT_BINARY_DIR}/test.pb.cc PROPERTIES GENERATED 1)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${BUILD_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

link_directories(${Boost_LIBRARY_DIRS} ${Protobuf_LIBRARY_DIRS})
add_executable(client client.cpp ${PROJECT_BINARY_DIR}/test.pb.cc)
add_dependencies(client example_proto_source)
add_dependencies(client BeanRpc)
target_link_libraries(client ${Boost_LIBRARIES} ${Protobuf_LIBRARIES} ${ZLIB_LIBRARIES} BeanRpc)

add_executable(server server.cpp ${PROJECT_BINARY_DIR}/test.pb.cc)
add_dependencies(server example_proto_source)
add_dependencies(server BeanRpc)
target_link_libraries(server ${Boost_LIBRARIES} ${Protobuf_LIBRARIES} ${ZLIB_LIBRARIES} BeanRpc -lz)