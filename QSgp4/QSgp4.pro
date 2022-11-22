#-------------------------------------------------
#
# Project created by QtCreator 2014-10-05T17:48:03
#
#-------------------------------------------------

QT       += core

TARGET = QSgp4
TEMPLATE = lib
CONFIG += staticlib



SOURCES += qsgp4.cpp \
    qeci.cpp \
    qobserver.cpp \
    qsgp4date.cpp \
    qtle.cpp \
    Matrices.cpp

HEADERS += qsgp4.h \
    qeci.h \
    qgeodetic.h \
    qobserver.h \
    qsgp4date.h \
    qsgp4globals.h \
    qsgp4utilities.h \
    qtle.h \
    qtopocentric.h \
    Vectors.h \
    qgeocentric.h \
    Matrices.h

CONFIG(release, debug|release) {
    #This is a release build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/release/qsgp4
    else:win32:TARGET = ../../../libs/win64_mingw64/release/qsgp4
} else {
    #This is a debug build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/debug/qsgp4
    else:win32:TARGET = ../../../libs/win64_mingw64/debug/qsgp4
}


OTHER_FILES += \
    README.md
