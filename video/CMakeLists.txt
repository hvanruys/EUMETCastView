cmake_minimum_required(VERSION 3.14)

project(EUMETCastVideo)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_SOURCES
	main.cpp
	generalverticalperspective.cpp
	geoseglist.cpp
	globals.cpp
	rssvideo.cpp
	msgdataaccess.cpp
	msgfileaccess.cpp
	gshhsdata.cpp
	pixgeoconversion.cpp
	xmlvideoreader.cpp
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED Core Gui Widgets Xml Concurrent Network PrintSupport)

if(UNIX)
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/QSgp4")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/meteosatlib")

# sequence of libraries is important !
target_link_libraries(${PROJECT_NAME} z pthread Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Widgets Qt5::Xml Qt5::Concurrent Qt5::Network Qt5::PrintSupport
    my_compiler_flags_1 meteosatlib QSgp4 bz2 JPEG T4 WT DISE COMP ${HDF5_LINK_LIBRARIES} ${NETCDF_LINK_LIBRARIES} netcdf archive freeimage)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
