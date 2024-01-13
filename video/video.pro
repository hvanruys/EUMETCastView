QT += core gui network widgets printsupport

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#unix:TARGET = ../deployEUMETCastView/EUMETCastVideo

SOURCES += main.cpp \
    generalverticalperspective.cpp \
    geoseglist.cpp \
    globals.cpp \
    rssvideo.cpp \
    msgdataaccess.cpp \
    msgfileaccess.cpp \
    gshhsdata.cpp \
    pixgeoconversion.cpp \
    xmlvideoreader.cpp


DISTFILES += \
    example.xml \
    EUMETCastVideo.xml

HEADERS += \
    generalverticalperspective.h \
    geoseglist.h \
    rssvideo.h \
    msgdataaccess.h \
    msgfileaccess.h \
    gshhsdata.h \
    gshhs.h \
    pixgeoconversion.h \
    globals.h \
    xmlvideoreader.h



unix:INCLUDEPATH += ../meteosatlib ../QSgp4
#unix:LIBS += -lpthread -lz -lfreeimage
unix:LIBS += -L$$PWD/../libs -lmeteosatlib -lbz2 -lhdf5_serial -larchive -lQSgp4
#unix:LIBS += -L/usr/lib/x86_64-linux-gnu/ -lnetcdf
unix:LIBS += -L$$PWD/../libs -lDISE -lJPEG -lWT -lT4 -lCOMP


