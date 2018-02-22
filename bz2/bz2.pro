TEMPLATE = lib
CONFIG -= qt
CONFIG += staticlib

SOURCES += \
    huffman.c \
    decompress.c \
    crctable.c \
    compress.c \
    bzlib.c \
    blocksort.c \
    randtable.c

HEADERS += \
    bzlib.h \
    bzlib_private.h

CONFIG(release, debug|release) {
    #This is a release build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/release/bz2
    win32:TARGET = ../../../libs/win64_MSVC2012/release/bz2
} else {
    #This is a debug build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/debug/bz2
    win32:TARGET = ../../../libs/win64_MSVC2012/debug/bz2
}
