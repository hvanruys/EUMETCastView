# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

EUMETCastView is a Qt6/C++20 desktop application for viewing satellite imagery received via EUMETSAT's EUMETCast dissemination service. It supports:

- **Polar orbit**: AVHRR (Metop-A/B/C), VIIRS (SUOMI-NPP, NOAA-20/21), OLCI EFR/ERR (Sentinel-3A), MERSI (FY-3D)
- **Geostationary (HRIT/LRIT/HDF5/netCDF)**: Meteosat-9/10/11, GOMS3, FY-2H/G, GOES-18/19, Himawari-9, Meteosat-12 (MTG FCI)

Two executables are produced:
- `bin/EUMETCastView` — the main GUI application (`core/`)
- `bin/EUMETCastVideo` — standalone video animation tool (`video/`)

## Build

```bash
# Linux
mkdir build && cd build
cmake ..
cmake --build .

# AppImage
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
make install DESTDIR=AppDir

# Windows (MSYS2/MinGW)
mkdir build && cd build
cmake -G "MSYS Makefiles" ..
cmake --build .
```

Executables go to `bin/`, which is shared by every build tree — the last build
to link wins, so after an AppImage container build `bin/EUMETCastView` is the
Ubuntu 20.04 binary until you rebuild natively.

`FormMovie` spawns `EUMETCastVideo` from the directory `EUMETCastView` is itself
running from, so the two have to stay together — which `bin/` gives you. Build
both from the top-level `CMakeLists.txt`, on Windows as well. CMake is the only
build there is; the qmake project files are gone.

Static libraries go to `<build tree>/libs`, one set per configuration. They must
not be shared: an archive is only re-created when it is older than its objects,
so a shared `libs/` lets one build tree hand another its stale, wrongly-targeted
archives.

### Required system dependencies (Linux)

```
libhdf5-dev  libnetcdf-dev  libarchive-dev  libfreeimage-dev
mesa-common-dev  libglu1-mesa-dev  qt6-base-dev  qt6-5compat-dev
```

### Key CMake defines

- `-DOPENGL33` — enables `QOpenGLFunctions_3_3_Core` (the default; alternatives: `OPENGL30`, `OPENGL40`, `OPENGL43`, `OPENGLES`). 3.3 is what the shaders need — they are all `#version 330` — and `main.cpp` requests a 3.3 **core** context to match. `OPENGL30` is a compatibility class and cannot be initialised on a core context, so the two settings have to move together.
- `-DHDF5_DISABLE_VERSION_CHECK=1`
- `-DCHARLS_STATIC`

HDF5 and NetCDF are discovered via `pkg-config`. Debug builds use `-O0 -g3 -ggdb -fno-omit-frame-pointer`.

## Bundled libraries (submodules / vendored)

| Directory | Purpose |
|-----------|---------|
| `bz2/` | bzip2 compression |
| `charls-main/` | CharLS JPEG-LS codec (linked as `libcharls.a`) |
| `PublicDecompWT-2.8.1/` | EUMETSAT xRIT wavelet decompressor; builds the `JPEG`, `T4`, `WT`, `DISE`, `COMP` static libs used by `meteosatlib` and `core` |
| `meteosatlib/` | MSG/Meteosat HRIT segment, header, and channel parsing |
| `QSgp4/` | Qt SGP4/SDP4 satellite orbit propagator (Vallado/Crawford algorithm) |
| `aaplus/` | AA+ astronomical algorithms (solar/lunar position, ephemeris) |

## Architecture

### Data model

`AVHRRSatellite` (`avhrrsatellite.h`) is the central container for all segment lists. It owns one `SegmentList*` per polar sensor type and ten `SegmentListGeostationary` objects (one per supported geo satellite, indexed by `eGeoSatellite`).

```
AVHRRSatellite
├── SegmentListMetop       (AVHRR on Metop)
├── SegmentListHRP
├── SegmentListVIIRSM / SegmentListVIIRSDNB  (×3 for NPP / NOAA-20 / NOAA-21)
├── SegmentListOLCI
├── SegmentListMERSI
├── SegmentListVII
└── SegmentListGeostationary[0..9]  (MET_11 … MET_12)
```

Each `SegmentList*` holds a `QList<Segment*>` and a parallel `QList<Segment*>` for selected segments. The concrete segment types are `SegmentMetop`, `SegmentHRP`, `SegmentVIIRSM`, etc.

`SatelliteList` (separate from `AVHRRSatellite`) holds `Satellite` objects propagated via `QSgp4` from TLE data.

### Main window views

`MainWindow` hosts a stacked widget with three views, switched from the menu:

1. **`FormMapCyl`** — 2D cylindrical equidistant map (`MapFieldCyl` + `CylEquiDist`). Draws satellite ground tracks and segment footprints using QPainter; overlays coastlines via `gshhsData` (GSHHS binary format).
2. **`Globe`** — Interactive 3D OpenGL globe (requires an OpenGL ≥ 3.3 core profile). Uses `GeometryEngine` (sphere mesh), `SkyBox`, `SatGL` (satellite positions), `SegmentGL` (swath footprints), `ProjExtentsGL` (projection boundary overlays), and `TextureWriter` (satellite imagery as GL textures).
3. **`FormImage`** — Flat image viewer (`QGraphicsView`-based) for composed satellite images with zoom/pan.

A dock widget contains `FormToolbox` with tabs for each sensor type. `FormGeostationary` is a separate widget that browses and triggers geostationary image composition.

### Image composition pipeline

**Polar**: `SegmentList::ComposeAVHRRImage()` iterates selected `Segment` objects and composites scanlines into a `SegmentImage`. Heavy work runs on a thread via `QtConcurrent::run` + `QFutureWatcher`.

**Geostationary** (`SegmentListGeostationary`):
- HRIT/XRIT: `ComposeImageXRITMSGInThread` → decoded by `meteosatlib` + `PublicDecompWT`
- HDF5 (Himawari/GOES): `ComposeImageHDFInThread`
- netCDF (FY-2, MTG FCI): `ComposeImagenetCDFInThread` / `ComposeImagenetCDFMTGInThread`
- MTG FCI uses `QtConcurrent::mapped` for per-segment concurrent processing (`concurrentMinMaxMTG`, `concurrentLUTGeoMTG`, `concurrentImageMTG`)
- RGB recipes (Airmass, Dust, 24h Microphysics, Ash, Day Microphysics, Severe Storms, Snow, Natural Colors, Night Microphysics) are composed in `ComposeGeoRGBRecipeInThread`
- Rayleigh correction for FCI VIS channels: `RayleighCorrector` (`rayleigh.h`)

### Projections

`SegmentImage` holds the projected image. Supported map projections:
- `GeneralVerticalPerspective`
- `LambertConformalConic`
- `Stereographic`
- `ObliqueMercator`
- `CylEquiDist` (cylindrical equidistant / equirectangular)

### Key enums (`globals.h`)

- `eSegmentType` — `SEG_METOP`, `SEG_HRP`, `SEG_VIIRSM`, `SEG_VIIRSDNB`, `SEG_OLCIEFR`, `SEG_OLCIERR`, `SEG_MERSI`, etc.
- `eGeoSatellite` — `MET_11` … `MET_12` (index 0–9, `NOGEO` = 10)
- `eImageType` — `IMAGE_AVHRR_*`, `IMAGE_GEOSTATIONARY`, `IMAGE_PROJECTION`, `IMAGE_VIIRSM`, `IMAGE_OLCI`, `IMAGE_MERSI`, `IMAGE_VII`
- `eRgbRecipes` — `RGB_AIRMASS` … `RGB_NIGHTMICRO`

### Configuration

Settings are stored in `core/EUMETCastView.ini` (read/written by Qt's `QSettings`). The `Options` class (global singleton accessed throughout the app) parses this INI at startup. It stores satellite data directories, TLE file paths, GSHHS coastline paths, display colors, and per-sensor channel/band configurations. `DialogPreferences` is the GUI for editing these settings.

The `GeoSatellites` struct in `options.h` fully describes each geo satellite: orbital position, image dimensions, file naming patterns, spectrum lists, HRIT/HDF/netCDF format details, and CLAHE parameters.

### Download management

`DownloadManager` queues HTTP downloads via `QNetworkAccessManager` (used for TLE updates from Celestrak). `ProcessManager` manages a pool of concurrent `QProcess` instances (used for spawning external MTG FCI processing jobs).

### `video/` tool (`EUMETCastVideo`)

A lightweight companion executable that reads XML or JSON configuration files (`xmlvideoreader`, `jsonvideoreader`) describing a sequence of geostationary images and produces animation frames. It reuses `SegmentListGeostationary`, `msgfileaccess`/`msgdataaccess`, `pixgeoconversion`, and `gshhsdata` from shared code (copied into `video/`).

## External data requirements

The application needs these data files configured in `EUMETCastView.ini`:

- **GSHHS coastline files** (`gshhs_l.b`, `wdb_borders_i.b`, `wdb_rivers_i.b`, `gshhs_i.b`) — binary vector files, not included in the repo
- **Background map images** (`images/Topography.jpg`, `images/NE2_50M_SR_W_4096.jpg`)
- **TLE files** (downloaded from Celestrak; paths configured per-installation)
- **Satellite data directories** — paths to EUMETCast received data
