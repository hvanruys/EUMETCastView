# PublicDecompWT history
The versioning of PublicDecompWT follows [Semantic Versioning](https://semver.org/) from version 2.06.
This changelog uses [keep a changelog](https://keepachangelog.com/en/1.0.0/) as its template.

## [2.8.1] - 2021-10-22
### Added
- Conda package for windows

### Fixed
- Remove mentions of cygwin from README

## [2.8.0] - 2021-07-14
### Added
- Support for build with Native Tools for VS (32 and 64 bit).
- conda package for Linux

## [2.7.2] - 2019-12-17
### Fixed
- Bug in headers preventing compilation

## [2.7.1] - 2019-10-28
### Added
- Metadata related to IPR

## [2.7.0] - 2019-08-09
### Changed
- Update README and accompanying metadata
- Release under Apache v2 license

## [2.06] - 2011-07-12
### Added
- Support for `gcc` 4.4.x and previous versions.

## [2.05] - 2009-06-24
### Fixed
- Prevent vector pointer on `m_Tmp` from accessing outside vector boundaries.
This was causing null exceptions depending on the implementation of the standard library.
Fix kindly provided by Andre.

### Changed
- Modified files with respect to the previous `CWBlock.cpp`.

## [2.04] - 2008-08-27
### Added
- Support for `gcc` 4.2.x and previous versions. Upgrade kindly provided by Mateusz Loskot.

### Changed
- `CImage.h`
- `CJBlock.h`
- `CJBlock.cpp`

## [2.03] - 2008-03-17
### Fixed
- Time stamp (i.e. Header Type #5 at offset 0x8c) on decompressed files did not match time stamp of the original compressed files for UNIX platforms.

### Changed
- Reunify 32 and 64 bits Makefiles into just one Makefile.
- Number of Kernel bits can be passed to the Makefile or is picked up automatically for Solaris and Unix platforms.
- Change of test data and its structure.
Original uncompressed data and their compressed counterparts are provided.
The check is performed comparing the original uncompressed data with the decompressed data obtained from running the `xRITDecompress` tool.

## [2.02] - 2007-06-04
### Added
- Support for both little and big endian architectures.
- Support for cygwin.
- Support for Eclipse.
- Support for 32- and 64 bits machines as well as mixed architectures.
- Can be asked for the version number on commandline.

## [2.01] - 2003-06-17
### Known issues
- Works only on big endian architectures (e.g.: Solaris machines).

## [2.00] - 2003-06-05
Update by VCS.
### Known issues
- Works only on little endian architectures (e.g.: Intel desktop machines).

## [1.00] - 2001
First version developed by VCS.
