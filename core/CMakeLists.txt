cmake_minimum_required(VERSION 3.14)

#SET(CMAKE_CXX_FLAGS_DEBUG "-O0")
#SET(CMAKE_C_FLAGS_DEBUG "-O0")

project(EUMETCastView)

set(PROJECT_SOURCES_CPP
    main.cpp
    formmovie.cpp
    mainwindow.cpp
    obliquemercator.cpp
    options.cpp
    segmentimage.cpp
    satellite.cpp
    globals.cpp
    avhrrsatellite.cpp
    segment.cpp
    segmentgac.cpp
    segmenthrp.cpp
    segmentlistmersi.cpp
    segmentmersi.cpp
    segmentmetop.cpp
    segmentnoaa.cpp
    segmentlist.cpp
    segmentlistgac.cpp
    segmentlisthrp.cpp
    segmentlistmetop.cpp
    segmentlistnoaa.cpp
    generalverticalperspective.cpp
    lambertconformalconic.cpp
    stereographic.cpp
    pixgeoconversion.cpp
    solar.cpp
    formephem.cpp
    downloadmanager.cpp
    formtoolbox.cpp
    formimage.cpp
    imagescrollarea.cpp
    gshhsdata.cpp
    cylequidist.cpp
    mapcyl.cpp
    formmapcyl.cpp
    globe.cpp
    geometryengine.cpp
    octahedron.cpp
    projextentsgl.cpp
    satgl.cpp
    segmentgl.cpp
    skybox.cpp
    soc.cpp
    trackball.cpp
    sathorizon.cpp
    dialogpreferences.cpp
    segmentlistgeostationary.cpp
    formgeostationary.cpp
    texturewriter.cpp
    sgp_math.cpp
    sgp_obs.cpp
    sgp_time.cpp
    qcompressor.cpp
    segmentviirsm.cpp
    segmentviirsdnb.cpp
    segmentlistviirsdnb.cpp
    segmentlistviirsm.cpp
    poi.cpp
    equirectangular.cpp
    qcustomplot.cpp
    segmentlistolci.cpp
    segmentolci.cpp
    dialogsaveimage.cpp
    segmenthrpt.cpp
    segmentlisthrpt.cpp
    segmentslstr.cpp
    segmentlistslstr.cpp
    datahubaccessmanager.cpp
    segmentdatahub.cpp
    segmentlistdatahub.cpp
    msgdataaccess.cpp
    msgfileaccess.cpp
    internal.cpp
    misc_util.cpp
    nav_util.cpp
    ColorSpace.cpp
    Conversion.cpp
    moon.cpp
    mainwindow.ui
    dialogpreferences.ui
    formephem.ui
    formmapcyl.ui
    formmovie.ui
    formtoolbox.ui
    formgeostationary.ui
)

#add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)

#INCLUDE_DIRECTORIES (/HDF_Group/HDF5/1.14.0/include)
#set(ENV{PKG_CONFIG_PATH} "/HDF_Group/HDF5/1.14.0/lib/pkgconfig")

#set (FIND_HDF_COMPONENTS CXX C shared)
#find_package (HDF5 NAMES "hdf5" COMPONENTS ${FIND_HDF_COMPONENTS})
#if (HDF5_FOUND)
#  if (HDF5_shared_C_FOUND)
#    set (HDF5_LINK_LIBRARIES ${HDF5_LINK_LIBRARIES} ${HDF5_C_SHARED_LIBRARY})
#  endif ()
#  if (HDF5_shared_CXX_FOUND)
#    set (HDF5_LINK_LIBRARIES ${HDF5_LINK_LIBRARIES} ${HDF5_CXX_SHARED_LIBRARY})
#  endif ()
#else ()
#  message (FATAL_ERROR " HDF5 is Required")
#endif ()
#INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIRS})



pkg_check_modules(HDF5 REQUIRED hdf5)
INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIRS})

pkg_check_modules(NETCDF REQUIRED netcdf)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Xml Core5Compat OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Xml Concurrent Network PrintSupport OpenGL Core5Compat OpenGLWidgets)

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_C_COMPILER Qt5Widgets_VERSION_STRING)
cmake_print_variables(CMAKE_PREFIX_PATH)
cmake_print_variables(HDF5_LINK_LIBRARIES)
cmake_print_variables(NETCDF_LINK_LIBRARIES)
cmake_print_variables(HDF5_INCLUDE_DIRS)
cmake_print_variables(NETCDF_INCLUDE_DIRS)
cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR)

if(UNIX)
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES_CPP} EUMETCastView.qrc shaders.qrc
    )
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES_CPP} EUMETCastView.qrc shaders.qrc darkstyle.qrc)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/aaplus")

add_definitions(-DHDF5_DISABLE_VERSION_CHECK=1 -DOPENGL30)
# sequence of libraries is important !
if(WIN32)
target_link_libraries(${PROJECT_NAME} z pthread Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Xml Qt6::Concurrent Qt6::Network Qt6::PrintSupport Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Core5Compat
    my_compiler_flags_1 meteosatlib QSgp4 bz2 JPEG T4 WT DISE COMP ${HDF5_LINK_LIBRARIES} ${NETCDF_LINK_LIBRARIES} archive freeimage aaplus-static ws2_32)
else()
target_link_libraries(${PROJECT_NAME} z pthread Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Xml Qt6::Concurrent Qt6::Network Qt6::PrintSupport Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Core5Compat
    my_compiler_flags_1 meteosatlib QSgp4 bz2 JPEG T4 WT DISE COMP ${HDF5_LINK_LIBRARIES} ${NETCDF_LINK_LIBRARIES} archive freeimage aaplus-static)
endif()
#set_target_properties(${PROJECT_NAME}
#  PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


