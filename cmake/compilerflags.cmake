# Compile settings shared by every target in the tree.
#
# This lives in its own file because there are two possible top-level projects:
# the one in the root CMakeLists.txt, which builds everything, and video/, which
# can be configured on its own so QtCreator can build and debug EUMETCastVideo
# in isolation. bz2, meteosatlib and QSgp4 all link my_compiler_flags_1, so
# whichever of the two is the top level has to define it.

include_guard(GLOBAL)

add_library(my_compiler_flags_1 INTERFACE)
target_compile_features(my_compiler_flags_1 INTERFACE cxx_std_17)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(my_compiler_flags_1 INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O0;-Wno-conversion-null;-Wno-pragmas;-Wno-trigraphs;-Wformat;-Wformat-security;>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
