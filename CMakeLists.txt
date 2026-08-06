cmake_minimum_required(VERSION 3.14)

project(EUMETCastView LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(my_compiler_flags_1 INTERFACE)
target_compile_features(my_compiler_flags_1 INTERFACE cxx_std_17)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(my_compiler_flags_1 INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O0;-Wno-conversion-null;-Wno-pragmas;-Wno-trigraphs;-Wformat;-Wformat-security;>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

# Libraries go in the build tree, one set per configuration, and must stay
# there. They used to be written to a single libs/ beside the source, which two
# build trees then shared - and an archive is not relinked the way an executable
# is. Make only re-creates a .a when it is older than its objects, so a native
# build overwriting libs/ left the Ubuntu 20.04 AppImage build linking archives
# full of glibc 2.38 symbols against a glibc 2.31 runtime. It failed at the very
# last link, on undefined references to __isoc23_sscanf and
# __throw_bad_array_new_length, with nothing pointing at the cause.
#
# Executables stay in bin/. They are relinked whenever any input is newer, so
# they cannot go stale the same way, and build-appimage.sh reads them from there.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(bz2)
add_subdirectory(charls-main)

# CharLS sets no optimisation level of its own, and these builds are configured
# without CMAKE_BUILD_TYPE, so the JPEG-LS decoder was being compiled at -O0.
# That decoder is nearly the whole cost of reading an FCI disc: decoding the 40
# chunks of three visible bands took 18.9 s at -O0, 5.3 s at -O3, and 3.7 s once
# NDEBUG also compiles out the codec's internal asserts. Those are the flags
# upstream's own Release build uses, and the asserts are invariant checks only -
# a malformed stream is rejected by throw_jpegls_error, which NDEBUG leaves
# alone. Set here rather than in charls-main/ so the vendored tree stays as
# upstream ships it.
target_compile_options(charls PRIVATE
  "$<${gcc_like_cxx}:-O3>"
  "$<${msvc_cxx}:/O2>"
)
target_compile_definitions(charls PRIVATE NDEBUG)
add_subdirectory(PublicDecompWT-2.8.1)
add_subdirectory(meteosatlib)
add_subdirectory(QSgp4)
add_subdirectory(aaplus)
add_subdirectory(core)
add_subdirectory(video)

