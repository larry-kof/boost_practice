cmake_minimum_required(VERSION 3.6)

option(USE_SSL "whether to use ssl" ON)
add_subdirectory(lib)
add_subdirectory(example)