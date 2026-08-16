cmake_minimum_required(VERSION 3.18)

project(T4)

set(PROJECT_SOURCES_CPP
	Src/CBitBuffer.cpp
	Src/CompressT4.cpp
	Src/T4Coder.cpp
	Src/T4Codes.cpp
	Src/T4Decoder.cpp
)

add_library(
    T4 STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(T4 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Inc")
target_include_directories(T4 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../Inc")
target_include_directories(T4 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../DISE")
target_link_libraries(T4 my_compiler_flags)

