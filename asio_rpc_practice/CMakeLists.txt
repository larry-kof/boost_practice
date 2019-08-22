cmake_minimum_required(VERSION 3.6)

add_custom_command(OUTPUT test.pb.cc test.pb.h
COMMAND protoc 
ARGS --cpp_out . -I${CMAKE_CURRENT_SOURCE_DIR}/common ${CMAKE_CURRENT_SOURCE_DIR}/common/test.proto
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/common/test.proto
COMMENT "generate test.pb.cc")

add_custom_target(protoc_source ALL DEPENDS test.pb.cc)

add_subdirectory(server)
add_subdirectory(client)