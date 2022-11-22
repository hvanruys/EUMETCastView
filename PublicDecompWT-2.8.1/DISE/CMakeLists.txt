cmake_minimum_required(VERSION 3.18)

project(DISE)

set(PROJECT_SOURCES_CPP
	CxRITFile.cpp
	CxRITFileHeaderRecords.cpp
	CxRITAnnotation.cpp
	ErrorHandling.cpp
	TimeSpan.cpp
	UTCTime.cpp
)

add_library(
    DISE STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(DISE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(DISE my_compiler_flags)
