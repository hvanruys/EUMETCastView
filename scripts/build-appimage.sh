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

# These seed data files are external, per-machine data (see CLAUDE.md's
# "External data requirements" section) — they are not produced by this
# repo's build, so check for them up front rather than discovering a missing
# one via a bare "cp: cannot stat" error after a full Release rebuild.
for seed_src in "$BIN_DIR/gshhs2_3_7" "$BIN_DIR/weather.tle" "$BIN_DIR/resource.tle" \
                "$BIN_DIR/GeoSatellites.ini" "$BIN_DIR/POI.ini"; do
    if [ ! -e "$seed_src" ]; then
        echo "Missing seed data file: $seed_src" >&2
        echo "  This is external, per-machine data not produced by this repo's" >&2
        echo "  build; see CLAUDE.md's \"External data requirements\" section." >&2
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

# GeoSatellites.ini and POI.ini are generic, per-satellite/projection
# metadata with no personal paths (confirmed via grep for /home/hugo), and
# GeoSatellites.ini has no code-level fallback if absent: Options::InitializeGeo()
# silently ends up with zero geostationary satellites configured. By
# contrast, core/EUMETCastView.ini is deliberately NOT bundled here — it
# holds this machine's real absolute data paths (NFS mounts, /home/hugo/...
# directories), and Options has working relative-path defaults without it.
# Do not add EUMETCastView.ini to this seed dir.
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

# Override any pre-existing (possibly broken) QMAKE env var from the ambient
# shell — linuxdeploy-plugin-qt prefers QMAKE over its own PATH discovery,
# and a QMAKE pointing at a directory instead of the qmake binary causes an
# "exec() failed: Permission denied" error in the plugin.
export QMAKE="$(command -v qmake6)"

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
