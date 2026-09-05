EUMETCastView
=============

A viewer for the Eumetcast transmissions,EUMETSAT’s primary dissemination mechanism for the near real-time delivery of satellite images.

The Open Source program **EUMETCastView** allows you to view

- AVHRR images from Metop-A, Metop-B and Metop-C.
- VIIRS images from SUOMI NPP, NOAA-20 and NOAA-21 (M-Band and Day/Night Band)
- VII/METimage from Metop SGA1
- OLCI EFR/ERR from Sentinel-3A
- HRIT/LRIT images from Meteosat-11, Meteosat-10, Meteosat-9, Electro L3, FengYun 2H/2G, GOES-18, GOES-19 and Himawari-9.
- MERSI images from FY-3D
- FCI images from Meteosat-12

More details are found on http://hvanruys.github.io

Linux software drivers ( for VM's ) : see https://itsfoss.com/install-mesa-ubuntu/

Compile in Linux :

- sudo apt install build-essential cmake \
                   qt6-base-dev qt6-5compat-dev \
                   libhdf5-dev libnetcdf-dev libarchive-dev libfreeimage-dev \
                   mesa-common-dev libglu1-mesa-dev libfontconfig1
- mkdir build
- cd build
- cmake ..
- cmake --build .

Building the AppImage :

Both variants are scripted; don't build them by hand in build/.

- ./scripts/build-appimage-container.sh — the release AppImage. Builds inside an
  Ubuntu 20.04 container (glibc 2.31, libstdc++ 3.4.28) so it also runs on
  distributions older than this one, in build-appimage-ubuntu2004/ and
  AppDir-ubuntu2004/. Needs podman or docker, Qt in ~/Qt/6.9.2/gcc_64, and
  linuxdeploy, linuxdeploy-plugin-qt and appimagetool in ~/AppImages. When it
  finishes it reports the symbol floor of everything in the AppDir and whether
  OpenSSL 3 was bundled.
- ./scripts/build-appimage.sh — the same build natively, in build-appimage/ and
  AppDir/. Quick to check, but the result only runs on systems as new as this
  one.

Both write EUMETCastView-x86_64.AppImage in the repository root; rename it for a
release. Pass --skip-build to package the binaries already in bin/ instead of
rebuilding.

Both also link into bin/, which every build tree shares — after a container
build bin/EUMETCastView is the Ubuntu 20.04 binary, so rebuild natively before
running or debugging it here.

Compile in Windows :
- Install msys2
- mkdir build
- cd build
- cmake -G "MSYS Makefiles" ..
- cmake --build .

or

Compile in QtCreator (Windows):

- pacman --needed -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-qwt-qt6
- pacman --needed -S mingw-w64-ucrt-x86_64-hdf5 mingw-w64-ucrt-x86_64-netcdf
- pacman --needed -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-qt6-5compat
- pacman --needed -S mingw-w64-ucrt-x86_64-freeimage

Setting up QtCreator

 1. Open the "Build & Run" panel in QtCreator options
 2. In the "Qt Versions" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/qmake.exe`
 3. In the "Compilers" tab, add a new one for MinGW C and C++, respectively pointing to `(MSYS2_PATH)/ucrt64/bin/gcc.exe` and `.../g++.exe`
 4. In the "Debuggers" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/gdb.exe`
 5. In the "CMake" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/cmake.exe`
 6. In the "Kits" tab, add a new one where you use the compilers, debugger, qt and cmake defined above
 7. Select "CodeBlocks - Ninja" as CMake Generator
