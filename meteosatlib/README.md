This version of the meteosat library only contains the low level meteosatlib library as it is found at
[SourceForge](http://sourceforge.net/projects/meteosatlib/files/).
It is used for reading the meteosat images in the EUMETCastView program.
The decompressing is done with the bz2 library. For decrypting the files you need a EUMETSAT key dongle and be registered at Eumetcast.
The source code for the wavelet decrypting can be obtained by filling in the form at http://oiswww.eumetsat.int/WEBOPS-cgi/wavelet/register.
The libraries DISE, JPEG, WT, T4 and COMP it links against are built from PublicDecompWT-2.8.1 by the top level CMakeLists.txt.
