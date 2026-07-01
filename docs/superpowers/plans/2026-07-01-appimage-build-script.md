# AppImage Build Script Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `scripts/build-appimage.sh`, which builds EUMETCastView and EUMETCastVideo from source and packages them into a self-contained `EUMETCastView-x86_64.AppImage` using the linuxdeploy toolchain already installed at `~/AppImages/`.

**Architecture:** One bash orchestration script (`scripts/build-appimage.sh`) drives: prerequisite checks → cmake configure/build → manual AppDir assembly (there are no `install()` CMake rules) → `linuxdeploy --plugin qt` → `appimagetool`. Two small static template files live under `scripts/appimage/`: a corrected `.desktop` file and a POSIX-`sh` wrapper (`eumetcastview-start.sh`) that seeds first-run data files into the launch directory and sets `HDF5_PLUGIN_PATH`, working around the app's CWD-relative config/data resolution documented in the design spec.

**Tech Stack:** bash, POSIX sh, CMake, linuxdeploy + linuxdeploy-plugin-qt, appimagetool.

**Design spec:** `docs/superpowers/specs/2026-07-01-appimage-build-script-design.md` — read this first for the *why* behind every path/seeding decision below.

---

### Task 1: Desktop file template

**Files:**
- Create: `scripts/appimage/EUMETCastView.desktop`

- [ ] **Step 1: Write the file**

```
[Desktop Entry]
Type=Application
Name=EUMETCastView
Comment=Viewer for EUMETCast polar and geostationary satellite imagery
Exec=eumetcastview-start %F
Icon=EUMETCastView
Categories=Science;Education;
Terminal=false
```

This replaces the checked-in `EUMETCastView.desktop` at the repo root for
AppImage purposes only — that file has `Exec=AppRun %F`, which is not a
valid `Exec` target for linuxdeploy (it must name an executable inside
`AppDir/usr/bin`, not the AppImage runtime's own entry point). The repo-root
file is untouched by this plan.

- [ ] **Step 2: Verify required keys are present**

Run: `grep -E "^(Type|Name|Exec|Icon|Categories)=" scripts/appimage/EUMETCastView.desktop`
Expected: 5 lines printed, one per key, matching the content above exactly.

- [ ] **Step 3: Commit**

```bash
git add scripts/appimage/EUMETCastView.desktop
git commit -m "$(cat <<'EOF'
Add AppImage desktop file template

Exec points at the eumetcastview-start wrapper (added next), not
AppRun directly, since the wrapper needs to run first-run seeding
logic before launching the real binary.

Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 2: First-run wrapper script

**Files:**
- Create: `scripts/appimage/eumetcastview-start.sh`

- [ ] **Step 1: Write the file**

```sh
#!/bin/sh
# Launched by AppRun via the desktop file's Exec key. EUMETCastView
# resolves its config/data (EUMETCastView.ini, GeoSatellites.ini, POI.ini,
# images/, gshhs2_3_7/, weather.tle, resource.tle) via QSettings/relative
# paths from the current working directory, not from its own install
# location — so this script seeds first-run data into the CWD (never
# overwriting anything already there) before handing off. It also copies
# EUMETCastVideo into the CWD, since MTG FCI processing spawns it via the
# relative path "./EUMETCastVideo" (processmanager.cpp), and points
# HDF5_PLUGIN_PATH at the bundled FCIDECOMP filter plugin unless the user
# has already set one.
set -e

HERE="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
SEED="$HERE/../share/EUMETCastView/seed"

[ -d ./gshhs2_3_7 ]        || cp -r "$SEED/gshhs2_3_7" ./gshhs2_3_7
[ -d ./images ]            || cp -r "$SEED/images" ./images
[ -f ./weather.tle ]       || cp "$SEED/weather.tle" ./weather.tle
[ -f ./resource.tle ]      || cp "$SEED/resource.tle" ./resource.tle
[ -f ./GeoSatellites.ini ] || cp "$SEED/GeoSatellites.ini" ./GeoSatellites.ini
[ -f ./POI.ini ]           || cp "$SEED/POI.ini" ./POI.ini
[ -x ./EUMETCastVideo ]    || cp "$HERE/EUMETCastVideo" ./EUMETCastVideo

: "${HDF5_PLUGIN_PATH:=$HERE/../lib/hdf5/plugin}"
export HDF5_PLUGIN_PATH

exec "$HERE/EUMETCastView" "$@"
```

- [ ] **Step 2: Make it executable**

Run: `chmod +x scripts/appimage/eumetcastview-start.sh`

- [ ] **Step 3: Syntax-check it**

Run: `sh -n scripts/appimage/eumetcastview-start.sh`
Expected: no output, exit code 0. Confirm with `echo $?` → `0`.

- [ ] **Step 4: Commit**

```bash
git add scripts/appimage/eumetcastview-start.sh
git commit -m "$(cat <<'EOF'
Add AppImage first-run seeding wrapper script

Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 3: Ignore build/package artifacts

**Files:**
- Modify: `.gitignore`

- [ ] **Step 1: Append new entries**

Current contents of `.gitignore`:
```
build/
bin/
libs/
hugostoken
CMakeLists.txt.user
```

Add three lines so the file becomes:
```
build/
bin/
libs/
hugostoken
CMakeLists.txt.user
build-appimage/
AppDir/
*.AppImage
```

- [ ] **Step 2: Verify**

Run: `git status --short`
Expected: only `M .gitignore` listed (no untracked `build-appimage/`, `AppDir/`, or `*.AppImage` yet, since none exist on disk at this point).

- [ ] **Step 3: Commit**

```bash
git add .gitignore
git commit -m "$(cat <<'EOF'
Ignore AppImage build artifacts

Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 4: Main orchestration script

**Files:**
- Create: `scripts/build-appimage.sh`

- [ ] **Step 1: Write the file**

```bash
#!/usr/bin/env bash
set -euo pipefail

# --- Paths -------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$REPO_ROOT/build-appimage"
APPDIR="$REPO_ROOT/AppDir"
OUTPUT="$REPO_ROOT/EUMETCastView-x86_64.AppImage"
BIN_DIR="$REPO_ROOT/bin"

LINUXDEPLOY="${LINUXDEPLOY:-$HOME/AppImages/linuxdeploy-x86_64.AppImage}"
LINUXDEPLOY_PLUGIN_QT="${LINUXDEPLOY_PLUGIN_QT:-$HOME/AppImages/linuxdeploy-plugin-qt-x86_64.AppImage}"
APPIMAGETOOL="${APPIMAGETOOL:-$HOME/AppImages/appimagetool-x86_64.AppImage}"

SKIP_BUILD=0
for arg in "$@"; do
    case "$arg" in
        --skip-build)
            SKIP_BUILD=1
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            echo "Usage: $0 [--skip-build]" >&2
            exit 1
            ;;
    esac
done

# --- Prerequisite checks -------------------------------------------------

missing=0
for tool_var in LINUXDEPLOY LINUXDEPLOY_PLUGIN_QT APPIMAGETOOL; do
    tool_path="${!tool_var}"
    if [ ! -x "$tool_path" ]; then
        echo "Missing or non-executable $tool_var: $tool_path" >&2
        echo "  Set $tool_var to override, or place it at that path." >&2
        missing=1
    fi
done

for cmd in cmake qmake6; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Missing required command on PATH: $cmd" >&2
        missing=1
    fi
done

if [ "$missing" -ne 0 ]; then
    echo "One or more prerequisites are missing; see above. Aborting." >&2
    exit 1
fi

# --- Build -----------------------------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> Configuring ($BUILD_DIR)"
    cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
    echo "==> Building"
    cmake --build "$BUILD_DIR" -j"$(nproc)"
else
    echo "==> --skip-build passed, reusing existing bin/ binaries"
fi

if [ ! -x "$BIN_DIR/EUMETCastView" ] || [ ! -x "$BIN_DIR/EUMETCastVideo" ]; then
    echo "Expected built binaries not found in $BIN_DIR" >&2
    echo "  (EUMETCastView and EUMETCastVideo must both exist and be executable)" >&2
    exit 1
fi

# --- Assemble AppDir --------------------------------------------------------

echo "==> Assembling AppDir ($APPDIR)"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" \
         "$APPDIR/usr/lib/hdf5/plugin" \
         "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/icons/hicolor/48x48/apps" \
         "$APPDIR/usr/share/EUMETCastView/seed"

cp "$BIN_DIR/EUMETCastView" "$APPDIR/usr/bin/EUMETCastView"
cp "$BIN_DIR/EUMETCastVideo" "$APPDIR/usr/bin/EUMETCastVideo"

cp "$SCRIPT_DIR/appimage/eumetcastview-start.sh" "$APPDIR/usr/bin/eumetcastview-start"
chmod +x "$APPDIR/usr/bin/eumetcastview-start"

cp "$SCRIPT_DIR/appimage/EUMETCastView.desktop" "$APPDIR/usr/share/applications/EUMETCastView.desktop"

cp "$REPO_ROOT/Globe_48x48.png" "$APPDIR/usr/share/icons/hicolor/48x48/apps/EUMETCastView.png"

if [ -f "$BIN_DIR/libh5fcidecomp.so" ]; then
    cp "$BIN_DIR/libh5fcidecomp.so" "$APPDIR/usr/lib/hdf5/plugin/libh5fcidecomp.so"
else
    echo "Warning: $BIN_DIR/libh5fcidecomp.so not found; FCIDECOMP support will be unavailable in the AppImage." >&2
fi

SEED="$APPDIR/usr/share/EUMETCastView/seed"
cp -r "$BIN_DIR/gshhs2_3_7" "$SEED/gshhs2_3_7"
cp -r "$REPO_ROOT/core/images" "$SEED/images"
cp "$BIN_DIR/weather.tle" "$SEED/weather.tle"
cp "$BIN_DIR/resource.tle" "$SEED/resource.tle"
cp "$BIN_DIR/GeoSatellites.ini" "$SEED/GeoSatellites.ini"
cp "$BIN_DIR/POI.ini" "$SEED/POI.ini"

# --- linuxdeploy + appimagetool ---------------------------------------------

# linuxdeploy's --plugin qt looks for an executable literally named
# "linuxdeploy-plugin-qt" on PATH; the downloaded file is named
# "linuxdeploy-plugin-qt-x86_64.AppImage", so expose it under the expected
# name via a throwaway symlink directory prepended to PATH.
PLUGIN_LINK_DIR="$(mktemp -d)"
trap 'rm -rf "$PLUGIN_LINK_DIR"' EXIT
ln -s "$LINUXDEPLOY_PLUGIN_QT" "$PLUGIN_LINK_DIR/linuxdeploy-plugin-qt"
export PATH="$PLUGIN_LINK_DIR:$PATH"
export ARCH=x86_64

echo "==> Running linuxdeploy"
"$LINUXDEPLOY" --appimage-extract-and-run \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/EUMETCastView" \
    --executable "$APPDIR/usr/bin/EUMETCastVideo" \
    --desktop-file "$APPDIR/usr/share/applications/EUMETCastView.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/48x48/apps/EUMETCastView.png" \
    --plugin qt

echo "==> Running appimagetool"
rm -f "$OUTPUT"
"$APPIMAGETOOL" --appimage-extract-and-run "$APPDIR" "$OUTPUT"

echo "==> Done: $OUTPUT"
echo "Launch it from a working directory where you want EUMETCastView's"
echo "config/data files to live (GeoSatellites.ini, POI.ini, gshhs2_3_7/,"
echo "images/, weather.tle, resource.tle will be seeded there on first run"
echo "if not already present)."
```

- [ ] **Step 2: Make it executable**

Run: `chmod +x scripts/build-appimage.sh`

- [ ] **Step 3: Syntax-check it**

Run: `bash -n scripts/build-appimage.sh`
Expected: no output, exit code 0. Confirm with `echo $?` → `0`.

- [ ] **Step 4: Commit**

```bash
git add scripts/build-appimage.sh
git commit -m "$(cat <<'EOF'
Add AppImage build orchestration script

Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 5: Verify prerequisite fail-fast behavior

No code changes in this task — this exercises the checks added in Task 4
without requiring a full build.

**Files:** none (verification only)

- [ ] **Step 1: Force a missing-tool failure**

Run:
```bash
LINUXDEPLOY=/nonexistent/linuxdeploy scripts/build-appimage.sh --skip-build; echo "exit=$?"
```
Expected: stderr includes `Missing or non-executable LINUXDEPLOY: /nonexistent/linuxdeploy`, and the last line printed is `exit=1`. (`LINUXDEPLOY_PLUGIN_QT` and `APPIMAGETOOL` still resolve to their real defaults on this machine, so only the `LINUXDEPLOY` line should appear — if `cmake`/`qmake6` are also on `PATH`, no other missing-command lines print either.)

- [ ] **Step 2: Force an unknown-argument failure**

Run:
```bash
scripts/build-appimage.sh --bogus-flag; echo "exit=$?"
```
Expected: stderr includes `Unknown argument: --bogus-flag`, last line printed is `exit=1`.

No commit for this task (no files changed).

---

### Task 6: Full build and package run

**Files:** none (verification only; produces gitignored artifacts)

- [ ] **Step 1: Run the full script**

Run: `scripts/build-appimage.sh`
Expected: configures and builds via `build-appimage/`, then prints
`==> Done: /home/hugo/EUMETCastTools/EUMETCastView/EUMETCastView-x86_64.AppImage`
as the final line. This recompiles the whole project in Release mode, so
expect it to take a few minutes.

If it fails during the `linuxdeploy` or `appimagetool` step, re-run with
`--skip-build` after fixing the issue to avoid rebuilding from scratch:
`scripts/build-appimage.sh --skip-build`.

- [ ] **Step 2: Confirm the output is a valid AppImage**

Run: `file EUMETCastView-x86_64.AppImage`
Expected: output contains `ELF 64-bit LSB executable` (AppImages are
self-mounting ELF binaries with an embedded squashfs payload).

- [ ] **Step 3: Confirm seed data landed in the AppDir**

Run: `find AppDir/usr/share/EUMETCastView/seed -maxdepth 1 | sort`
Expected:
```
AppDir/usr/share/EUMETCastView/seed
AppDir/usr/share/EUMETCastView/seed/GeoSatellites.ini
AppDir/usr/share/EUMETCastView/seed/POI.ini
AppDir/usr/share/EUMETCastView/seed/gshhs2_3_7
AppDir/usr/share/EUMETCastView/seed/images
AppDir/usr/share/EUMETCastView/seed/resource.tle
AppDir/usr/share/EUMETCastView/seed/weather.tle
```

- [ ] **Step 4: Confirm the two real binaries and the wrapper are all in place**

Run: `ls -la AppDir/usr/bin/`
Expected: `EUMETCastView`, `EUMETCastVideo`, and `eumetcastview-start` all
present and executable (`-rwxr-xr-x` or similar, `x` bit set for all three).

No commit for this task — `build-appimage/`, `AppDir/`, and `*.AppImage` are
all gitignored (Task 3).

---

### Task 7: Runtime seeding verification

This validates the wrapper script's seeding behavior end-to-end. Full GUI
verification (confirming the geostationary satellite list is populated in
the UI, triggering an actual MTG FCI composition to confirm the
`./EUMETCastVideo` subprocess spawn, and checking the FCIDECOMP debug log
line) requires a display/X server and interactive use of the app — flag
that as a manual follow-up for the user rather than something to automate
here.

**Files:** none (verification only)

- [ ] **Step 1: Run from an empty scratch directory**

Run:
```bash
mkdir -p /tmp/eumetcast-appimage-smoke && cd /tmp/eumetcast-appimage-smoke
/home/hugo/EUMETCastTools/EUMETCastView/EUMETCastView-x86_64.AppImage --appimage-extract-and-run || true
```
Expected: the seeding `cp` commands in `eumetcastview-start.sh` run first
(unconditionally), then `exec` hands off to the real `EUMETCastView`
binary. Without a display, Qt will fail to start (e.g. an "xcb"/"could not
connect to display" style error) and the process will exit non-zero — that
failure is expected and tolerated by `|| true`; what this step actually
checks is the seeding side effect, verified in Step 2.

- [ ] **Step 2: Confirm seed files landed in the scratch directory**

Run: `cd /tmp/eumetcast-appimage-smoke && ls -1`
Expected:
```
EUMETCastVideo
GeoSatellites.ini
POI.ini
gshhs2_3_7
images
resource.tle
weather.tle
```

- [ ] **Step 3: Confirm seeding is idempotent (never overwrites)**

Run:
```bash
cd /tmp/eumetcast-appimage-smoke
echo "MARKER" >> GeoSatellites.ini
/home/hugo/EUMETCastTools/EUMETCastView/EUMETCastView-x86_64.AppImage --appimage-extract-and-run || true
tail -1 GeoSatellites.ini
```
Expected: last line printed is `MARKER` — confirms the second run did not
overwrite the already-present file.

- [ ] **Step 4: Clean up the scratch directory**

Run: `rm -rf /tmp/eumetcast-appimage-smoke`

- [ ] **Step 5: Report the manual-follow-up items to the user**

Tell the user (no code action): to fully confirm the packaged app, they
should run `EUMETCastView-x86_64.AppImage` from a real desktop session and
check that (a) the geostationary satellite list is populated on first
launch, (b) an MTG FCI composition succeeds (exercises the
`./EUMETCastVideo` subprocess spawn), and (c) the debug log reports the
FCIDECOMP filter as available (confirms `HDF5_PLUGIN_PATH` took effect).

No commit for this task.

---

## Plan self-review notes

- **Spec coverage:** every AppDir layout entry, script-flow step, wrapper
  behavior, and testing-plan item from the design spec maps to a task above
  (Tasks 1–4 build it, Tasks 5–7 verify it against the spec's own testing
  plan).
- **No placeholders:** all file contents are complete and final; no
  TBD/TODO markers.
- **Type/name consistency:** `LINUXDEPLOY` / `LINUXDEPLOY_PLUGIN_QT` /
  `APPIMAGETOOL` env var names, the `SEED` path
  (`usr/share/EUMETCastView/seed`), and the wrapper filename
  (`eumetcastview-start`) are used identically across the desktop file, the
  wrapper script, and the orchestration script.
