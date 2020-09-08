#-------------------------------------------------
#
# Project created by QtCreator 2011-11-28T10:50:15
#
#-------------------------------------------------

QT       += core gui network widgets printsupport
QT       += concurrent xml
unix:TARGET = ../EUMETCastView
else:win32:TARGET = ../../EUMETCastView
TEMPLATE = app
CONFIG += static
CONFIG += c++11
DEFINES += LIBARCHIVE_STATIC
DEFINES += HDF5_DISABLE_VERSION_CHECK=1
DEFINES += OPENGL30


SOURCES += main.cpp \
    mainwindow.cpp \
    options.cpp \
    segmentimage.cpp \
    satellite.cpp \
    globals.cpp \
    avhrrsatellite.cpp \
    segment.cpp \
    segmentgac.cpp \
    segmenthrp.cpp \
    segmentlistmersi.cpp \
    segmentmersi.cpp \
    segmentmetop.cpp \
    segmentnoaa.cpp \
    segmentlist.cpp \
    segmentlistgac.cpp \
    segmentlisthrp.cpp \
    segmentlistmetop.cpp \
    segmentlistnoaa.cpp \
    generalverticalperspective.cpp \
    lambertconformalconic.cpp \
    stereographic.cpp \
    pixgeoconversion.cpp \
    solar.cpp \
    formephem.cpp \
    downloadmanager.cpp \
    formtoolbox.cpp \
    formimage.cpp \
    imagescrollarea.cpp \
    gshhsdata.cpp \
    cylequidist.cpp \
    mapcyl.cpp \
    formmapcyl.cpp \
    globe.cpp \
    geometryengine.cpp \
    octahedron.cpp \
    projextentsgl.cpp \
    satgl.cpp \
    segmentgl.cpp \
    skybox.cpp \
    soc.cpp \
    trackball.cpp \
    sathorizon.cpp \
    dialogpreferences.cpp \
    segmentlistgeostationary.cpp \
    formgeostationary.cpp \
    texturewriter.cpp \
    sgp_math.cpp \
    sgp_obs.cpp \
    sgp_time.cpp \
    qcompressor.cpp \
    segmentviirsm.cpp \
    segmentviirsdnb.cpp \
    segmentlistviirsdnb.cpp \
    segmentlistviirsm.cpp \
    poi.cpp \
    equirectangular.cpp \
    infrascales.cpp \
    infrawidget.cpp \
    forminfrascales.cpp \
    qcustomplot.cpp \
    segmentlistolci.cpp \
    segmentolci.cpp \
    dialogsaveimage.cpp \
    segmenthrpt.cpp \
    segmentlisthrpt.cpp \
    segmentslstr.cpp \
    segmentlistslstr.cpp \
    datahubaccessmanager.cpp \
    segmentdatahub.cpp \
    segmentlistdatahub.cpp \
    msgdataaccess.cpp \
    msgfileaccess.cpp \
    internal.cpp \
    misc_util.cpp \
    nav_util.cpp

HEADERS  += mainwindow.h \
    options.h \
    segmentimage.h \
    satellite.h \
    globals.h \
    avhrrsatellite.h \
    segment.h \
    segmentgac.h \
    segmenthrp.h \
    segmentlistmersi.h \
    segmentmersi.h \
    segmentmetop.h \
    segmentnoaa.h \
    segmentlist.h \
    segmentlistgac.h \
    segmentlisthrp.h \
    segmentlistmetop.h \
    segmentlistnoaa.h \
    generalverticalperspective.h \
    lambertconformalconic.h \
    stereographic.h \
    pixgeoconversion.h \
    formephem.h \
    downloadmanager.h \
    formtoolbox.h \
    formimage.h \
    imagescrollarea.h \
    gshhs.h \
    gshhsdata.h \
    cylequidist.h \
    mapcyl.h \
    formmapcyl.h \
    globe.h \
    geometryengine.h \
    octahedron.h \
    projextentsgl.h \
    satgl.h \
    segmentgl.h \
    skybox.h \
    soc.h \
    trackball.h \
    sathorizon.h \
    dialogpreferences.h \
    segmentlistgeostationary.h \
    formgeostationary.h \
    texturewriter.h \
    sgp4sdp4.h \
    stdafx.h \
    qcompressor.h \
    segmentviirsm.h \
    segmentviirsdnb.h \
    segmentlistviirsdnb.h \
    segmentlistviirsm.h \
    poi.h \
    stb_image.h \
    equirectangular.h \
    infrascales.h \
    infrawidget.h \
    colormaps.h \
    forminfrascales.h \
    qcustomplot.h \
    segmentlistolci.h \
    segmentolci.h \
    dialogsaveimage.h \
    segmenthrpt.h \
    segmentlisthrpt.h \
    segmentslstr.h \
    segmentlistslstr.h \
    datahubaccessmanager.h \
    segmentdatahub.h \
    segmentlistdatahub.h \
    productlist.h \
    msgdataaccess.h \
    msgfileaccess.h \
    nav_util.h \
    internal.h \
    misc_util.h \
    gshhs.h

FORMS    += mainwindow.ui \
    dialogpreferences.ui \
    formephem.ui \
    formmapcyl.ui \
    formtoolbox.ui \
    formgeostationary.ui

RESOURCES += \
    EUMETCastView.qrc \
    shaders.qrc

#QMAKE_CXXFLAGS += -std=c++0x -Wno-trigraphs
unix:QMAKE_CXXFLAGS += -Wno-trigraphs
unix:QMAKE_LFLAGS += -no-pie
#win32:QMAKE_LFLAGS += /NODEFAULTLIB:MSVCRT

#/home/hugo/TAR_LIBS/libarchive-master/libarchive
unix:INCLUDEPATH += /usr/include/GL /usr/include/freetype2 /usr/local/hdf5/include ../bz2 ../zlib128-dll/include ../meteosatlib  ../QSgp4
win32:INCLUDEPATH += "D:/msys64/mingw64/include" ../bz2 ../meteosatlib ../QSgp4
#else:win32:INCLUDEPATH += "C:/Users/Windows7/libarchive-3.2.2-new/libarchive-3.2.2/libarchive" \
#                ../bz2 ../zlib128-dll/include ../meteosatlib ../QSgp4 \
#                "C:/Program Files/netCDF 4.4.1/include" "C:/Program Files/HDF_Group/HDF5/1.8.16/include"



CONFIG(release, debug|release) {
#This is a release build
unix:LIBS += -lpthread -lz -lfreeimage
unix:LIBS += -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/release -lmeteosat -lqsgp4 -lbz2 -lhdf5_serial -larchive
unix:LIBS += -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/DISE -lDISE
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/JPEG/Src -lJPEG
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/WT/Src -lWT
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/T4/Src -lT4
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/Src -lCOMP
win32:LIBS += -L$$PWD/../../libs/win64_mingw64/release -lmeteosat -lDOSE -lWT -lT4 -lJPEG -lCOMP -lqsgp4 -lbz2
win32:LIBS += -L"D:/msys64/mingw64/lib/" -lszip -lz -lhdf5.dll -lnetcdf.dll -larchive.dll -lfreeimage.dll
}
else
{
#This is a debug build
unix:LIBS += -lpthread -lz -lfreeimage
unix:LIBS += -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/debug -lmeteosat -lqsgp4 -lbz2 -lhdf5_serial -larchive
unix:LIBS += -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/DISE -lDISE
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/JPEG/Src -lJPEG
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/WT/Src -lWT
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/T4/Src -lT4
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT_2.7.2-master/COMP/Src -lCOMP
win32:LIBS += -L$$PWD/../../libs/win64_mingw64/debug -lmeteosat -lDOSE -lWT -lT4 -lJPEG -lCOMP -lqsgp4 -lbz2
win32:LIBS += -L"D:/msys64/mingw64/lib/" -lszip -lz -lhdf5.dll -lnetcdf.dll -larchive.dll -lfreeimage.dll
}

#CONFIG(release, debug|release) {
#This is a release build
#    unix:LIBS += -lpthread -lz -lfreeimage \
#        -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/release -lmeteosat -lDISE -lJPEG -lWT -lT4 -lCOMP -lqsgp4 -lpng -lbz2 -lhdf5_serial -larchive \
#        -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
#        #-L/usr/local/hdf5/lib -lhdf5
#    unix:LIBS += -lpthread -lz -lfreeimage \
#        -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/release -lmeteosat -lDISE -lJPEG -lWT -lT4 -lCOMP -lqsgp4 -lbz2 -lhdf5_serial -larchive \
#        -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
#        #-L/usr/local/hdf5/lib -lhdf5
#    else:win32:
#        LIBS += -L$$PWD/../../libs/win64_mingw64/release -lmeteosat -ldise -ljpeg -lwt -lt4 -lcomp -lqsgp4 -lbz2
#        LIBS += -L"C:/msys64/mingw64/lib/" -lszip -lz -lhdf5.dll -lnetcdf.dll -larchive.dll -lfreeimage.dll
#} #else {
#This is a debug build
#unix:LIBS += -lpthread -lz -lfreeimage \
#    -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/debug -lmeteosat -lDISE -lJPEG -lWT -lT4 -lCOMP -lqsgp4 -lpng -lbz2 -lhdf5_serial -larchive \
#    -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
    #-L/usr/local/hdf5/lib -lhdf5
#else:win32:LIBS += -lfreeimage \
#        -L"C:/Program Files/HDF_Group/HDF5/1.8.16/lib/" libszip.lib libzlib.lib libhdf5.lib \
#        "C:/Program Files/netCDF 4.4.1/lib/netcdf.lib" \
#        -L$$PWD/../../libs/win64_MSVC2012/debug -lmeteosat -lDISE -lJPEG -lWT -lT4 -lCOMP -lqsgp4 -lbz2 \
#        "C:/Users/Windows7/libarchive-3.2.2-new/libarchive-3.2.2/x64/Release/archive_static.lib" \
#        Advapi32.lib
#}

CONFIG(release, debug|release): DEFINES += NDEBUG

DISTFILES += \
    EUMETCastView.ini \
    images/NE2_50M_SR_W_4096.jpg \
    images/Topography.jpg \
    images/ulukai/corona_bk.png \
    images/ulukai/corona_dn.png \
    images/ulukai/corona_ft.png \
    images/ulukai/corona_lf.png \
    images/ulukai/corona_rt.png \
    images/ulukai/corona_up.png \
    images/ulukai/readme.txt
