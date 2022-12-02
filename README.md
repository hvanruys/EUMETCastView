EUMETCastView
=============

A viewer for the Eumetcast transmissions,EUMETSAT’s primary dissemination mechanism for the near real-time delivery of satellite images.

The Open Source program **EUMETCastView** allows you to view

- AVHRR images from NOAA-19, Metop-A, Metop-B and Metop-C.
- VIIRS images from SUOMI-NPP and NOAA-20.
- OLCI EFR/ERR and SLSTR from Sentinel-3A
- HRIT/LRIT images from Meteosat-11, Meteosat-10, Meteosat-8, Electro L3, FengYun 2H/2G, GOES-16, GOES-17, GOES-18 and Himawari-8.
- and MERSI from FY-3D

More details are found on http://hvanruys.github.io

Compile in Linux :
- mkdir build
- cd build
- cmake ..
- cmake --build .

Compile in Windows :
- Install msys2
- mkdir build
- cd build
- cmake -G "MSYS Makefiles" ..
- cmake --build .
