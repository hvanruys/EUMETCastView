cmake_minimum_required(VERSION 3.14)

project(EUMETCastVideo)

# This file is used two ways:
#
#   * as a subdirectory of the top-level EUMETCastView project, which is how
#     the shipped bin/EUMETCastVideo is built, and
#   * as a top-level project of its own, so QtCreator can configure, build and
#     debug just this executable.
#
# So nothing here may refer to CMAKE_SOURCE_DIR: in the second case that is
# video/, not the root of the tree, and the includes for meteosatlib and QSgp4
# resolve to directories that do not exist - which is what the missing
# MSG_data_image.h, qsgp4globals.h and Vectors.h were. EUMETCASTVIEW_ROOT is
# derived from this directory instead, so it is right either way.
get_filename_component(EUMETCASTVIEW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(EUMETCASTVIDEO_STANDALONE ON)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Standing in for the parent when there is no parent: the same libraries it
# would have configured, plus my_compiler_flags_1, which bz2, meteosatlib and
# QSgp4 link. Done before the AUTOMOC/AUTOUIC settings below so those apply to
# this executable only, exactly as in the full build.
if(EUMETCASTVIDEO_STANDALONE)
    include(${EUMETCASTVIEW_ROOT}/cmake/compilerflags.cmake)

    # One set of archives per build tree, never shared - see the comment in the
    # top-level CMakeLists.txt for what sharing them costs. The executable is
    # deliberately left in this build tree rather than sent to bin/, so a debug
    # build here cannot replace the bin/EUMETCastVideo that the main
    # application spawns as "./EUMETCastVideo".
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)

    # Out-of-tree sources, so each needs an explicit binary directory.
    add_subdirectory(${EUMETCASTVIEW_ROOT}/bz2 bz2)
    add_subdirectory(${EUMETCASTVIEW_ROOT}/PublicDecompWT-2.8.1 PublicDecompWT-2.8.1)
    add_subdirectory(${EUMETCASTVIEW_ROOT}/meteosatlib meteosatlib)
    add_subdirectory(${EUMETCASTVIEW_ROOT}/QSgp4 QSgp4)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_SOURCES
	main.cpp
	generalverticalperspective.cpp
	generalverticalperspective.h
	globals.cpp
	globals.h
	msgdataaccess.cpp
	msgdataaccess.h
	msgfileaccess.cpp
	msgfileaccess.h
	gshhsdata.cpp
	gshhsdata.h
	pixgeoconversion.cpp
	pixgeoconversion.h
	xmlvideoreader.cpp
	xmlvideoreader.h
	jsonvideoreader.cpp
	jsonvideoreader.h
	videomaker.cpp
	videomaker.h
)

find_package(Qt6 REQUIRED Core Gui Widgets Xml Concurrent Network PrintSupport)

# videomaker.cpp includes <hdf5.h> and <netcdf.h>. In the full build these used
# to be found only because core/ happens to be configured first and
# pkg_check_modules writes its results to the cache, which made them visible
# here by accident. Ask for them ourselves so the standalone build works too.
find_package(PkgConfig REQUIRED)
pkg_check_modules(HDF5 REQUIRED hdf5)
pkg_check_modules(NETCDF REQUIRED netcdf)

if(UNIX)
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${EUMETCASTVIEW_ROOT}/QSgp4")
target_include_directories(${PROJECT_NAME} PUBLIC "${EUMETCASTVIEW_ROOT}/meteosatlib")
target_include_directories(${PROJECT_NAME} PUBLIC ${HDF5_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS})

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR)
cmake_print_variables(EUMETCASTVIEW_ROOT)
cmake_print_variables(EUMETCASTVIDEO_STANDALONE)

# sequence of libraries is important !
target_link_libraries(${PROJECT_NAME} z pthread Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Widgets Qt6::Xml Qt6::Concurrent Qt6::Network Qt6::PrintSupport
    my_compiler_flags_1 meteosatlib QSgp4 bz2 JPEG T4 WT DISE COMP ${HDF5_LINK_LIBRARIES} ${NETCDF_LINK_LIBRARIES} netcdf archive freeimage)
