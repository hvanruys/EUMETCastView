#-------------------------------------------------
#
# The qmake project for EUMETCastVideo, kept in step with core/core.pro: the
# two executables have to end up in the same directory, because EUMETCastView
# spawns this one from the directory it is running from.
#
# CMakeLists.txt in this directory is the maintained build. This file is here
# for QtCreator on Windows, where core.pro is opened the same way; the
# top-level EUMETCastView.pro cannot be used for either, it names a
# PublicDecompWT-2.8.1 subproject that has no .pro file.
#
#-------------------------------------------------
QT       += core gui network widgets printsupport
QT       += concurrent xml

unix:TARGET = ../../deployEUMETCastView/EUMETCastVideo
win32:TARGET = ../../needed/EUMETCastVideo

TEMPLATE = app

# console, so that the qDebug() output of a run started by hand is readable.
# Nothing pops up when EUMETCastView spawns it: QProcess passes
# CREATE_NO_WINDOW whenever the parent has no console itself.
CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += HDF5_DISABLE_VERSION_CHECK=1

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    generalverticalperspective.cpp \
    globals.cpp \
    jsonvideoreader.cpp \
    msgdataaccess.cpp \
    msgfileaccess.cpp \
    gshhsdata.cpp \
    pixgeoconversion.cpp \
    videomaker.cpp \
    xmlvideoreader.cpp

HEADERS += \
    generalverticalperspective.h \
    globals.h \
    gshhs.h \
    gshhsdata.h \
    jsonvideoreader.h \
    msgdataaccess.h \
    msgfileaccess.h \
    pixgeoconversion.h \
    videomaker.h \
    xmlvideoreader.h

unix:INCLUDEPATH += /usr/local/hdf5/include ../bz2 ../meteosatlib ../QSgp4
win32:INCLUDEPATH += ../bz2 ../meteosatlib ../QSgp4 D:\msys64\ucrt64\include

unix:LIBS += -lpthread -lz -lfreeimage
unix:LIBS += -L$$_PRO_FILE_PWD_/../libs/linux_gplusplus/release -lmeteosat -lqsgp4 -lbz2 -lhdf5_serial -larchive
unix:LIBS += -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT-2.8.1/DISE -lDISE
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT-2.8.1/COMP/JPEG/Src -lJPEG
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT-2.8.1/COMP/WT/Src -lWT
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT-2.8.1/COMP/T4/Src -lT4
unix:LIBS += -L$$_PRO_FILE_PWD_/../PublicDecompWT-2.8.1/COMP/Src -lCOMP
win32:LIBS += -L$$PWD/../../libs/win64_mingw64/release -lmeteosat -lDISE -lWT -lT4 -lJPEG -lCOMP -lqsgp4 -lbz2
win32:LIBS += -L"D:/msys64/ucrt64/lib/" -lz -lhdf5.dll -lnetcdf.dll -larchive.dll -lfreeimage.dll

CONFIG(release, debug|release): DEFINES += NDEBUG

DISTFILES += \
    README.md
