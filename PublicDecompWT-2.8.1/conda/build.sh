#!/bin/bash


#$CXX -fPIC -g -Wall *.cpp -shared -o gdal_MSG.so -I $PREFIX/include/ -std=c++11 -L. -L $PREFIX/lib/ -l gdal -I PublicDecompWT/DISE/ -I PublicDecompWT/COMP/Inc/ -I PublicDecompWT/COMP/WT/Inc/

make -C xRITDecompress install DEST_DIR=$PREFIX


