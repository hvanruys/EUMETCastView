cmake_minimum_required(VERSION 3.5)

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
    infrascales.cpp
    infrawidget.cpp
    forminfrascales.cpp
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
    mainwindow.ui
    dialogpreferences.ui
    formephem.ui
    formmapcyl.ui
    formmovie.ui
    formtoolbox.ui
    formgeostationary.ui
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED netcdf)
pkg_check_modules(HDF5 REQUIRED hdf5)

find_package(Qt5 REQUIRED Core Gui Widgets Xml Concurrent Network PrintSupport)

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(CMAKE_C_COMPILER Qt5Widgets_VERSION_STRING)
cmake_print_variables(CMAKE_PREFIX_PATH)
cmake_print_variables(UI_RESOURCES_RCC)

cmake_print_properties(TARGETS Qt5::Widgets PROPERTIES LOCATION)
cmake_print_properties(TARGETS Qt5::Xml PROPERTIES LOCATION)

if(UNIX)
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES_CPP} EUMETCastView.qrc shaders.qrc)
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES_CPP} EUMETCastView.qrc shaders.qrc)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_definitions(-DHDF5_DISABLE_VERSION_CHECK=1 -DOPENGL30)
# sequence of libraries is important !
target_link_libraries(${PROJECT_NAME} z pthread Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Widgets Qt5::Xml Qt5::Concurrent Qt5::Network Qt5::PrintSupport
    my_compiler_flags_1 meteosatlib QSgp4 bz2 JPEG T4 WT DISE COMP ${HDF5_LINK_LIBRARIES} ${NETCDF_LINK_LIBRARIES} archive freeimage)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

