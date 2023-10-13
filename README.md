EUMETCastView
=============

A viewer for the Eumetcast transmissions,EUMETSAT’s primary dissemination mechanism for the near real-time delivery of satellite images.

The Open Source program **EUMETCastView** allows you to view

- AVHRR images from NOAA-19, Metop-A, Metop-B and Metop-C.
- VIIRS images from SUOMI-NPP and NOAA-20.
- OLCI EFR/ERR and SLSTR from Sentinel-3A
- HRIT/LRIT images from Meteosat-11, Meteosat-10, Meteosat-8, Electro L3, FengYun 2H/2G, GOES-16, GOES-17, GOES-18 and Himawari-8.
- MERSI images from FY-3D
- FCI images from MTG-I1

More details are found on http://hvanruys.github.io

Linux software drivers ( for VM's ) : see https://itsfoss.com/install-mesa-ubuntu/

Compile in Linux :

- sudo apt install build-essential
			libfontconfig1
			mesa-common-dev
			libglu1-mesa-dev
			qt5-default
			cmake
			libhdf5-dev
			libnetcdf-dev
			libarchive-dev
			libfreeimage-dev
- mkdir build
- cd build
- cmake ..
- cmake --build .

for building AppImage
- cmake .. -DCMAKE_INSTALL_PREFIX=/usr
- make -j$(nproc)
- make install DESTDIR=AppDir

Compile in Windows :
- Install msys2
- mkdir build
- cd build
- cmake -G "MSYS Makefiles" ..
- cmake --build .

or

Compile in QtCreator (Windows):

- pacman --needed -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-qwt-qt5
- pacman --needed -S mingw-w64-ucrt-x86_64-hdf5 mingw-w64-ucrt-x86_64-netcdf
- pacman --needed -S mingw-w64-ucrt-x86_64-freeimage

Setting up QtCreator

 1. Open the "Build & Run" panel in QtCreator options
 2. In the "Qt Versions" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/qmake.exe`
 3. In the "Compilers" tab, add a new one for MinGW C and C++, respectively pointing to `(MSYS2_PATH)/ucrt64/bin/gcc.exe` and `.../g++.exe`
 4. In the "Debuggers" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/gdb.exe`
 5. In the "CMake" tab, add a new one pointing to `(MSYS2_PATH)/ucrt64/bin/cmake.exe`
 6. In the "Kits" tab, add a new one where you use the compilers, debugger, qt and cmake defined above
 7. Select "CodeBlocks - Ninja" as CMake Generator
