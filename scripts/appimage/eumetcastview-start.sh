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
