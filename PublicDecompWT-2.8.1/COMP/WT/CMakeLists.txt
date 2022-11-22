cmake_minimum_required(VERSION 3.18)

project(WT)

set(PROJECT_SOURCES_CPP
	Src/CACCoder.cpp
	Src/CACDecoder.cpp
	Src/CACModel.cpp
	Src/CVLCCoder.cpp
	Src/CVLCDecoder.cpp
	Src/CWBlock.cpp
	Src/CWTCoder.cpp
	Src/CWTDecoder.cpp
	Src/CompressWT.cpp
)

add_library(
    WT STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(WT PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Inc")
target_include_directories(WT PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/Inc")
target_include_directories(WT PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/DISE")
target_link_libraries(WT my_compiler_flags)

