# PublicDecompWT
This repository contains the source code for the PublicDecompWT tool.
The tool can be build in Windows, Linux and Solaris.
Note that it only decompresses compressed XRIT files.
It includes a decoder for JPEG, WT and T4.

# Build instructions
## Linux, Solaris and Windows with Native Tools for VS
### Required software
- GNU-GCC (`g++`, tested with version 4.2.x or previous version 3.x)
- GNU-make (`make`, tested with version 3.79.1)
- `ar`
- `ranlib`
- `nmake` as part of the Native Tools for VS

### Instructions
Navigate into `xRITDecompress` and call `make`.
The compiled image will be stored in the `xRITDecompress` directory (image name `xRITDecompress`).

### Instructions for Windows with Native Tools for VS
- Open the Native Tools Command Prompt for VS (x86 or x64, the output executable format will be set accordingly)
- Navigate into PublicDecompWT folder and run "nmake /f makefile.vc". The executable file will be in the xRITDecompress folder.
- To clean, navigate into PublicDecompWT folder and run "nmake /f makefile.vc clean"
The procedure has been tested with Visual Studio Community 2019

### Build the conda package
- It is possible to build the conda package for Linux and Windows (64 bit)
- Prerequisites:
  - both platforms: `conda` and `conda-build` must be installed
  - Windows: Visual Studio Community 2019 must be installed
- Navigate to the `conda` folder in PublicDecompWT and execute:

`conda build .`

## Windows and Cygwin
The source code is reported to be compilable with cygwin64, but the feature is not supported officially

# Usage
You can specify:

```
--help: prints this list of options, stops execution
--version: prints current version, stops execution
-s:* source file (compulsory)
BITS=32 for 32 bits platforms or BITS=64 for 64 bits platforms.
```

Alternatively you can invoke the program with just the XRIT input file name:
```bash
$ xRITDecompress $XRITFilename
```
In Windows the program must be invoked with the `.exe` extension, i.e.

```bash
$ xRITDecompress.exe $XRITFilename
```
The output is the decompressed XRITFile in the current directory (same file name without extension `C`).

# Testing
Under Linux/Unix you can run `make testDx` to verify the software works correctly.
