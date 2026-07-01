# AppImage build script

## Purpose

Provide a single script, `scripts/build-appimage.sh`, that builds EUMETCastView
(and EUMETCastVideo) from source and packages them into a self-contained
`EUMETCastView-x86_64.AppImage`, using the linuxdeploy toolchain already
present on this machine (`~/AppImages/linuxdeploy-x86_64.AppImage`,
`~/AppImages/linuxdeploy-plugin-qt-x86_64.AppImage`,
`~/AppImages/appimagetool-x86_64.AppImage`).

## Background / constraints discovered

- `core/CMakeLists.txt` and `video/CMakeLists.txt` have no `install()` rules.
  `make install DESTDIR=` (as CLAUDE.md's AppImage snippet suggests) installs
  nothing. The script must manually copy binaries into the AppDir instead.
- `AVHRRSatellite`/`Options` config resolution is **CWD-relative, not
  applicationDirPath-relative**:
  - `QSettings settings("EUMETCastView.ini", ...)` (`options.cpp:19`) and the
    same pattern for `GeoSatellites.ini`, `POI.ini`.
  - Default image paths: `"./images/Topography.jpg"`,
    `"./images/NE2_50M_SR_W_4096.jpg"` (`options.cpp:92-93`).
  - Default GSHHS paths: `"./gshhs2_3_7/gshhs_i.b"` etc. (`options.cpp:96-101`,
    `278-280`).
  - Default TLE files: `"weather.tle"`, `"resource.tle"` (`options.cpp:48-49`).
  - MTG FCI processing spawns a subprocess via the **relative path**
    `"./EUMETCastVideo"` (`processmanager.cpp:18,32`).
  - Consequence: the AppImage must not change the working directory, and
    several runtime files must be reachable relative to whatever directory
    the user launches it from — not from `AppDir/usr/bin`.
- `libh5fcidecomp.so` (EUMETSAT FCIDECOMP HDF5 filter plugin, currently at
  `bin/libh5fcidecomp.so`) is loaded via `dlopen` through HDF5's
  `HDF5_PLUGIN_PATH` env var (checked in `mainwindow.cpp:265-278`), not linked
  directly. It depends only on `libstdc++`/`libgcc_s`/`libc`/`libm`.
- `ldd bin/EUMETCastView` confirms Qt6 (from `/home/hugo/Qt/6.9.2/gcc_64`),
  HDF5, netCDF, libarchive, and FreeImage are dynamically linked and need
  bundling; X11/GL/fontconfig are pulled in too but linuxdeploy's blacklist
  correctly excludes those (must come from the host for graphics
  compatibility).
- `qmake6` on `PATH` already resolves to the correct Qt 6.9.2 install, so
  `linuxdeploy-plugin-qt` will auto-detect it with no extra configuration.
- `bin/gshhs2_3_7/` (~162MB, all five GSHHS resolutions) and
  `bin/weather.tle` / `bin/resource.tle` are the source files to bundle, per
  explicit user request (these are otherwise "not included in the repo" per
  CLAUDE.md's external data requirements section).
- `core/EUMETCastView.ini` is tracked in git but is **not** a generic
  template — it holds this machine's actual absolute paths (e.g.
  `/home/hugo/Vol3TPart1/received/...`). It must not be bundled/redistributed.
  Every setting it holds has a hardcoded relative-path fallback in
  `options.cpp` (e.g. `"./images/Topography.jpg"`, `"./gshhs2_3_7/gshhs_i.b"`,
  `"weather.tle"`), so the app works correctly on a fresh CWD with **no
  `EUMETCastView.ini` at all**, as long as the referenced data files are
  present.
- `GeoSatellites.ini`, by contrast, has **no code-level fallback**:
  `Options::InitializeGeo()` (`options.cpp:701-803`) reads the `geos` array
  size via `beginReadArray`, and if the file is absent that's `0` — the
  `geosatellites` list ends up empty and every geostationary satellite
  (MET-9/10/11, MET-12/MTG FCI, GOES-18/19, Himawari-9, FY-2H/G, GOMS3) is
  silently unconfigured. `bin/GeoSatellites.ini` (522 lines, per-satellite
  technical metadata: dimensions, file patterns, CLAHE params, projection
  coefficients) contains no personal paths — confirmed no `/home/hugo`
  occurrences — so unlike `EUMETCastView.ini` it **is** safe and necessary to
  bundle. `bin/POI.ini` (projection/POI presets, also no personal paths) is
  in the same situation, minor UX degradation rather than a broken feature if
  missing, but bundled for the same reason.
- `core/images/` (21MB, git-tracked, generic background/skybox assets) is
  also genuinely generic and safe to bundle.

## Non-goals

- No change to any C++ source or `QSettings` path resolution logic — the
  packaging works around CWD-relative resolution rather than changing it.
- No bundling of user-specific data: satellite receive directories are not
  addressed (already absolute, per-installation paths set via
  DialogPreferences; out of scope).
- No Windows/macOS packaging — AppImage is Linux-only.

## AppDir layout

```
AppDir/
  usr/bin/EUMETCastView          # real GUI binary — passed to linuxdeploy --executable
  usr/bin/EUMETCastVideo         # real video binary — passed to linuxdeploy --executable
  usr/bin/eumetcastview-start    # wrapper script; this is the desktop file's Exec target
  usr/lib/hdf5/plugin/libh5fcidecomp.so
  usr/share/applications/EUMETCastView.desktop     # Exec=eumetcastview-start %F
  usr/share/icons/hicolor/48x48/apps/EUMETCastView.png
  usr/share/EUMETCastView/seed/gshhs2_3_7/         # auto-seeded into CWD if missing
  usr/share/EUMETCastView/seed/weather.tle         # auto-seeded into CWD if missing
  usr/share/EUMETCastView/seed/resource.tle        # auto-seeded into CWD if missing
  usr/share/EUMETCastView/seed/images/             # auto-seeded into CWD if missing
  usr/share/EUMETCastView/seed/GeoSatellites.ini   # auto-seeded into CWD if missing
  usr/share/EUMETCastView/seed/POI.ini             # auto-seeded into CWD if missing
```

`EUMETCastView.ini` is **not** bundled (see "personal paths" note above) —
`Options`'s built-in relative-path defaults are sufficient once the seed data
exists. `GeoSatellites.ini` and `POI.ini` *are* bundled, from `bin/` (the
generic, personal-path-free versions), since geostationary satellite support
has no fallback without them.

## Script flow (`scripts/build-appimage.sh`)

1. Resolve repo root from the script's own location; `set -euo pipefail`.
2. Resolve tool paths: `LINUXDEPLOY`, `LINUXDEPLOY_PLUGIN_QT`, `APPIMAGETOOL`
   env vars, defaulting to the `~/AppImages/*-x86_64.AppImage` files;
   `cmake`, `qmake6` must be on `PATH`. Fail fast with a clear message
   listing what's missing.
3. Unless `--skip-build` is passed: configure a dedicated build directory
   `build-appimage/` (kept separate from the existing debug `build/`) with
   `-DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release`, then
   `cmake --build build-appimage -j$(nproc)`. Output always lands in the
   repo's `bin/` dir (`CMAKE_RUNTIME_OUTPUT_DIRECTORY` is fixed at top-level
   `CMakeLists.txt`).
4. Verify `bin/EUMETCastView` and `bin/EUMETCastVideo` exist and are
   executable; fail with a clear message otherwise.
5. Recreate `AppDir` from scratch (`rm -rf`, `mkdir -p`) and populate it per
   the layout above:
   - Copy the two binaries into `usr/bin/`.
   - Write `usr/bin/eumetcastview-start` (see below), `chmod +x`.
   - Copy `bin/libh5fcidecomp.so` into `usr/lib/hdf5/plugin/`.
   - Generate a corrected `.desktop` file (the checked-in
     `EUMETCastView.desktop` has `Exec=AppRun %F`, invalid for linuxdeploy;
     the script writes its own with `Exec=eumetcastview-start %F`,
     `Icon=EUMETCastView`, `Categories=Science;Education;`).
   - Copy `Globe_48x48.png` to the icon path (as `EUMETCastView.png`).
   - Copy `bin/gshhs2_3_7/`, `bin/weather.tle`, `bin/resource.tle`,
     `bin/GeoSatellites.ini`, `bin/POI.ini`, and `core/images/` into
     `usr/share/EUMETCastView/seed/`.
6. Run `linuxdeploy` with `--appdir AppDir --executable usr/bin/EUMETCastView
   --executable usr/bin/EUMETCastVideo --desktop-file <generated> --icon-file
   <icon> --plugin qt`, using `--appimage-extract-and-run` to avoid FUSE
   dependency.
7. Run `appimagetool` (also via `--appimage-extract-and-run`) on the AppDir
   to produce `EUMETCastView-x86_64.AppImage` in the repo root.
8. Print the resulting file path and a short usage note: launch it from a
   working directory you want the app's data/config to live in (same
   requirement as running `bin/EUMETCastView` today).

## `eumetcastview-start` wrapper script

Runs after linuxdeploy's generated `AppRun` has already exported
`LD_LIBRARY_PATH`/plugin paths, and before the real binary starts. Never
changes directory, never overwrites existing files:

```sh
#!/bin/sh
set -e
HERE="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
SEED="$HERE/../share/EUMETCastView/seed"

[ -d ./gshhs2_3_7 ]   || cp -r "$SEED/gshhs2_3_7" ./gshhs2_3_7
[ -d ./images ]       || cp -r "$SEED/images" ./images
[ -f ./weather.tle ]  || cp "$SEED/weather.tle" ./weather.tle
[ -f ./resource.tle ] || cp "$SEED/resource.tle" ./resource.tle
[ -f ./GeoSatellites.ini ] || cp "$SEED/GeoSatellites.ini" ./GeoSatellites.ini
[ -f ./POI.ini ] || cp "$SEED/POI.ini" ./POI.ini
[ -x ./EUMETCastVideo ] || cp "$HERE/EUMETCastVideo" ./EUMETCastVideo

: "${HDF5_PLUGIN_PATH:=$HERE/../lib/hdf5/plugin}"
export HDF5_PLUGIN_PATH

exec "$HERE/EUMETCastView" "$@"
```

## Error handling

- Missing prerequisite tools/binaries: fail fast, before any AppDir mutation,
  with a message naming the missing tool and (for the three linuxdeploy
  tools) the expected `~/AppImages` path or override env var.
- Build failure: script exits non-zero via `set -e`; no partial AppImage is
  produced (AppDir assembly only starts after binaries are confirmed built).
- linuxdeploy/appimagetool failure: propagates via `set -e`; script does not
  attempt to interpret their output.

## Testing plan

- Run the script end to end on this machine; confirm
  `EUMETCastView-x86_64.AppImage` is produced.
- `./EUMETCastView-x86_64.AppImage --appimage-extract-and-run` (or direct
  execution) from an empty scratch directory; confirm `gshhs2_3_7/`,
  `images/`, `weather.tle`, `resource.tle`, `GeoSatellites.ini`, `POI.ini`,
  and `EUMETCastVideo` get seeded into that directory on first launch, and
  the GUI starts.
- Confirm the geostationary satellite list (MET-9/10/11, MET-12, GOES,
  Himawari, FY-2H/G, GOMS3) is populated on first launch — i.e. that seeding
  `GeoSatellites.ini` actually prevented the empty-list case described above.
- Re-run from the same directory; confirm seeded files are not overwritten
  (touch/modify one and check it survives a second launch).
- Trigger an MTG FCI composition to confirm the `./EUMETCastVideo` subprocess
  spawn (`processmanager.cpp`) succeeds from the AppImage-launched process.
- Confirm `H5Zfilter_avail(32018)` reports the FCIDECOMP filter available
  (via the existing debug log line in `mainwindow.cpp`), showing
  `HDF5_PLUGIN_PATH` took effect.
