cmake_minimum_required(VERSION 3.14)

project(EUMETCastView LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if(UNIX)
#    set(CMAKE_PREFIX_PATH /home/hugo/Qt5.12.12/5.12.12/gcc_64)
#elseif(WIN32)
#    set(CMAKE_PREFIX_PATH /home/hugo/Data-Linux/Qt/5.15.2/gcc_64)
#endif()

add_library(my_compiler_flags_1 INTERFACE)
target_compile_features(my_compiler_flags_1 INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(my_compiler_flags_1 INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O2;-Wno-conversion-null;-Wno-pragmas;-Wno-trigraphs;-Wformat;-Wformat-security;>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(bz2)
add_subdirectory(PublicDecompWT-2.8.1)
add_subdirectory(meteosatlib)
add_subdirectory(QSgp4)
add_subdirectory(aaplus)
add_subdirectory(core)
add_subdirectory(video)

