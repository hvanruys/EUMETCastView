#-------------------------------------------------
#
# Project created by QtCreator 2015-01-08T14:40:34
#
#-------------------------------------------------

QT       -= core gui

TARGET = SOIL
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    SOIL.cpp \
    image_DXT.cpp \
    image_helper.cpp \
    stb_image_aug.cpp

HEADERS += \
    SOIL.h \
    glext.h \
    image_DXT.h \
    image_helper.h \
    stbi_DDS_aug.h \
    stbi_DDS_aug_c.h \
    stb_image_aug.h

CONFIG(release, debug|release) {
    #This is a release build
    unix:TARGET = ../../libs/linux_gplusplus/release/SOIL
    else:win32:TARGET = ../../../libs/win64_MSVC2012/release/SOIL
} else {
    #This is a debug build
    unix:TARGET = ../../libs/linux_gplusplus/debug/SOIL
    else:win32:TARGET = ../../../libs/win64_MSVC2012/debug/SOIL
}

