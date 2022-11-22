cmake_minimum_required(VERSION 3.18)

project(COMP)

set(PROJECT_SOURCES_CPP
	Src/CBuffer.cpp
	Src/CImage.cpp
	Src/CQualityMeasures.cpp
)

add_library(
    COMP STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(COMP PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Inc")
target_include_directories(COMP PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/DISE")
target_link_libraries(COMP my_compiler_flags)

