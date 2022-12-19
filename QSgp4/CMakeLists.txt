cmake_minimum_required(VERSION 3.5)

project(QSgp4)

set(PROJECT_SOURCES_CPP
	qsgp4.cpp
	qeci.cpp
	qobserver.cpp
	qsgp4date.cpp
	qtle.cpp
        qsun.cpp
        sgp_math.cpp
        sgp_time.cpp
	Matrices.cpp
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Gui REQUIRED)
add_library(
    QSgp4 STATIC
    ${PROJECT_SOURCES_CPP}
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/core")

target_link_libraries(QSgp4 my_compiler_flags_1 Qt5::Gui)

