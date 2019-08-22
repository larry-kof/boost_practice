cmake_minimum_required(VERSION 3.6)
project(RpcLibrary)

find_package(Boost 1.69 COMPONENTS System REQUIRED)
find_package(Protobuf REQUIRED)

set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${PROJECT_BINARY_DIR})
add_custom_command(OUTPUT rpc.pb.cc rpc.pb.h
COMMAND protoc
ARGS --cpp_out . -I${CUR_DIR} ${CUR_DIR}/rpc.proto
DEPENDS ${CUR_DIR}/rpc.proto)

add_custom_target(proto_source ALL DEPENDS rpc.pb.cc rpc.pb.h)

set_source_files_properties(${PROJECT_BINARY_DIR}/rpc.pb.cc PROPERTIES GENERATED 1)

aux_source_directory(${CUR_DIR} SRC_LIST)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${BUILD_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wwrite-strings -Wshadow -Wno-unused-parameter -Wno-sign-conversion -std=c++11")

link_directories(${Boost_LIBRARY_DIRS} ${Protobuf_LIBRARY_DIRS})
add_library(BeanRpc ${SRC_LIST} ${PROJECT_BINARY_DIR}/rpc.pb.cc)
add_dependencies(BeanRpc proto_source)
target_link_libraries(BeanRpc ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})