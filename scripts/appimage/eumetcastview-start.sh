#!/bin/sh
# Launched by AppRun via the desktop file's Exec key. Most of
# EUMETCastView's config/data (EUMETCastView.ini, GeoSatellites.ini,
# POI.ini, weather.tle, resource.tle) is resolved via QSettings/relative
# paths from the current working directory, not from its own install
# location — so this script seeds first-run data into the CWD (never
# overwriting anything already there) before handing off. GSHHS coastline
# data and background/skybox images are NOT seeded here: Options reads the
# APPDIR env var (set automatically by the AppImage runtime) and resolves
# those specifically via $APPDIR/gshhs2_3_7/... and $APPDIR/images/...
# instead (see gshhsdata.cpp, mainwindow.cpp, skybox.cpp) — those files are
# bundled directly at the AppDir root by build-appimage.sh, not here. This
# script also copies EUMETCastVideo into the CWD, since MTG FCI processing
# spawns it via the relative path "./EUMETCastVideo" (processmanager.cpp),
# and points HDF5_PLUGIN_PATH at the bundled FCIDECOMP filter plugin unless
# the user has already set one.
set -e

HERE="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
SEED="$HERE/../share/EUMETCastView/seed"

[ -f ./weather.tle ]       || cp "$SEED/weather.tle" ./weather.tle
[ -f ./resource.tle ]      || cp "$SEED/resource.tle" ./resource.tle
[ -f ./GeoSatellites.ini ] || cp "$SEED/GeoSatellites.ini" ./GeoSatellites.ini
[ -f ./POI.ini ]           || cp "$SEED/POI.ini" ./POI.ini
[ -x ./EUMETCastVideo ]    || cp "$HERE/EUMETCastVideo" ./EUMETCastVideo

: "${HDF5_PLUGIN_PATH:=$HERE/../lib/hdf5/plugin}"
export HDF5_PLUGIN_PATH

exec "$HERE/EUMETCastView" "$@"
