cmake_minimum_required(VERSION 3.18)

project(JPEG)

set(PROJECT_SOURCES_CPP
	Src/CHOptim.cpp
	Src/CHcodec.cpp
	Src/CHufftables.cpp
	Src/CJBlock.cpp
	Src/CJPEGDecoder.cpp
	Src/CJPEGLossLessCoder.cpp
	Src/CJPEGLossyCoder.cpp
	Src/CQuantizationTable.cpp
	Src/CompressJPEG.cpp
)

add_library(
    JPEG STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(JPEG PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Inc")
target_include_directories(JPEG PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/Inc")
target_include_directories(JPEG PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/DISE")
target_link_libraries(JPEG my_compiler_flags)

