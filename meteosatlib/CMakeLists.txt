cmake_minimum_required(VERSION 3.14)

project(meteosatlib)

set(PROJECT_SOURCES_CPP
	MSG_data.cpp
	MSG_header.cpp
	MSG_filetype.cpp
	MSG_data_SatelliteStatus.cpp
	MSG_time_cds.cpp
	MSG_spacecraft.cpp
	MSG_machine.cpp
	MSG_compression.cpp
	MSG_channel.cpp
	MSG_data_RadiometricQuality.cpp
	MSG_data_RadiometricProc.cpp
	MSG_data_NavigExtrResult.cpp
	MSG_data_key.cpp
	MSG_data_IMPFConfiguration.cpp
	MSG_data_ImageProdStats.cpp
	MSG_data_ImageDescription.cpp
	MSG_data_ImageAcquisition.cpp
	MSG_data_gts.cpp
	MSG_data_GeometricQuality.cpp
	MSG_data_GeometricProc.cpp
	MSG_data_format.cpp
	MSG_data_CelestialEvents.cpp
	MSG_header_timestamp.cpp
	MSG_header_segment_quality.cpp
	MSG_header_segment_id.cpp
	MSG_header_key.cpp
	MSG_header_image_struct.cpp
	MSG_header_image_navig.cpp
	MSG_header_image_datafunc.cpp
	MSG_header_annotation.cpp
	MSG_header_ancillary_text.cpp
	MSG_data_TimelinComple.cpp
	MSG_data_text.cpp
	MSG_data_SGS_header.cpp
	MSG_data_image.cpp
	MSG_quality.cpp
	MSG_HRIT.cpp 
)

add_library(
    meteosatlib STATIC
    ${PROJECT_SOURCES_CPP}
)

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)

target_include_directories(meteosatlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(meteosatlib PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/DISE")
target_include_directories(meteosatlib PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/Inc")
target_include_directories(meteosatlib PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/JPEG/Inc")
target_include_directories(meteosatlib PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/WT/Inc")
target_include_directories(meteosatlib PUBLIC "${CMAKE_SOURCE_DIR}/PublicDecompWT-2.8.1/COMP/T4/Inc")
target_link_libraries(meteosatlib my_compiler_flags_1)

