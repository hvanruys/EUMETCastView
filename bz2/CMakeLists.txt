cmake_minimum_required(VERSION 3.14)

project(bz2)

set(PROJECT_SOURCES_CPP
	huffman.c
	decompress.c
	crctable.c
	compress.c
	bzlib.c
	blocksort.c
	randtable.c 
)

add_library(
    bz2 STATIC
    ${PROJECT_SOURCES_CPP}
)

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR)
cmake_print_variables(CMAKE_SOURCE_DIR)


target_include_directories(bz2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(bz2 my_compiler_flags_1)

