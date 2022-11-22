cmake_minimum_required(VERSION 3.5)

project(xRITDecompressProject VERSION 2.8.1)

add_library(my_compiler_flags INTERFACE)
target_compile_features(my_compiler_flags INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(my_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O2;-Wno-conversion-null;-Wno-pragmas;>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)


add_subdirectory(DISE)
add_subdirectory(COMP)
add_subdirectory(COMP/JPEG)
add_subdirectory(COMP/T4)
add_subdirectory(COMP/WT)
add_subdirectory(xRITDecompress)

