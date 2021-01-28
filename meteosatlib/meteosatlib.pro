TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

SOURCES += \
    MSG_data.cpp \
    MSG_header.cpp \
    MSG_filetype.cpp \
    MSG_data_SatelliteStatus.cpp \
    MSG_time_cds.cpp \
    MSG_spacecraft.cpp \
    MSG_machine.cpp \
    MSG_compression.cpp \
    MSG_channel.cpp \
    MSG_data_RadiometricQuality.cpp \
    MSG_data_RadiometricProc.cpp \
    MSG_data_NavigExtrResult.cpp \
    MSG_data_key.cpp \
    MSG_data_IMPFConfiguration.cpp \
    MSG_data_ImageProdStats.cpp \
    MSG_data_ImageDescription.cpp \
    MSG_data_ImageAcquisition.cpp \
    MSG_data_gts.cpp \
    MSG_data_GeometricQuality.cpp \
    MSG_data_GeometricProc.cpp \
    MSG_data_format.cpp \
    MSG_data_CelestialEvents.cpp \
    MSG_header_timestamp.cpp \
    MSG_header_segment_quality.cpp \
    MSG_header_segment_id.cpp \
    MSG_header_key.cpp \
    MSG_header_image_struct.cpp \
    MSG_header_image_navig.cpp \
    MSG_header_image_datafunc.cpp \
    MSG_header_annotation.cpp \
    MSG_header_ancillary_text.cpp \
    MSG_data_TimelinComple.cpp \
    MSG_data_text.cpp \
    MSG_data_SGS_header.cpp \
    MSG_data_image.cpp \
    MSG_quality.cpp \
    MSG_HRIT.cpp
    

HEADERS += \
    MSG_header.h \
    MSG_data.h \
    MSG_filetype.h \
    MSG_hrit_specdoc.h \
    MSG_data_SatelliteStatus.h \
    MSG_time_cds.h \
    MSG_spacecraft.h \
    MSG_machine.h \
    MSG_compression.h \
    MSG_channel.h \
    MSG_data_RadiometricQuality.h \
    MSG_data_RadiometricProc.h \
    MSG_data_NavigExtrResult.h \
    MSG_data_key.h \
    MSG_data_IMPFConfiguration.h \
    MSG_data_ImageProdStats.h \
    MSG_data_image_mpef.h \
    MSG_data_ImageDescription.h \
    MSG_data_ImageAcquisition.h \
    MSG_data_image.h \
    MSG_data_gts.h \
    MSG_data_GeometricQuality.h \
    MSG_data_GeometricProc.h \
    MSG_data_format.h \
    MSG_data_CelestialEvents.h \
    MSG_header_timestamp.h \
    MSG_header_segment_quality.h \
    MSG_header_segment_id.h \
    MSG_header_key.h \
    MSG_header_image_struct.h \
    MSG_header_image_navig.h \
    MSG_header_image_datafunc.h \
    MSG_header_annotation.h \
    MSG_header_ancillary_text.h \
    MSG_data_TimelinComple.h \
    MSG_data_text.h \
    MSG_data_SGS_header.h \
    MSG_HRIT.h \
    Compress.h \
    MSG_projection.h \
    MSG_quality.h


CONFIG(release, debug|release) {
    #This is a release build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/release/meteosat
    else:win32:TARGET = ../../../libs/win64_mingw64/release/meteosat
} else {
    #This is a debug build
    unix:TARGET = $$_PRO_FILE_PWD_/../libs/linux_gplusplus/debug/meteosat
    else:win32:TARGET = ../../../libs/win64_mingw64/debug/meteosat
}

unix:INCLUDEPATH += ../PublicDecompWT_2.7.2-master/DISE \
                    ../PublicDecompWT_2.7.2-master/COMP/JPEG/Inc \
                    ../PublicDecompWT_2.7.2-master/COMP/Inc \
                    ../PublicDecompWT_2.7.2-master/COMP/WT/Inc \
                    ../PublicDecompWT_2.7.2-master/COMP/T4/Inc
else:win32:INCLUDEPATH += ../../PublicDecompWT-master/DISE \
                    ../../PublicDecompWT-master/COMP/JPEG/Inc \
                    ../../PublicDecompWT-master/COMP/Inc \
                    ../../PublicDecompWT-master/COMP/WT/Inc \
                    ../../PublicDecompWT-master/COMP/T4/Inc

DISTFILES += \
    README.md

