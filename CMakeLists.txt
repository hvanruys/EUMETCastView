cmake_minimum_required(VERSION 3.5)

project(EUMETCastView VERSION 1.5.5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH /home/hugo/Qt/5.15.2/gcc_64)

add_library(my_compiler_flags_1 INTERFACE)
target_compile_features(my_compiler_flags_1 INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(my_compiler_flags_1 INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O2;-Wno-conversion-null;-Wno-pragmas;-Wno-trigraphs;-no-pie;-Wformat;-Wformat-security;>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

add_subdirectory(bz2)
add_subdirectory(PublicDecompWT-2.8.1)
add_subdirectory(meteosatlib)
add_subdirectory(QSgp4)
add_subdirectory(core)
add_subdirectory(video)
