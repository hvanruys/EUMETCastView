echo on

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
nmake /f makefile.vc /e install DEST_DIR=$(PREFIX)
