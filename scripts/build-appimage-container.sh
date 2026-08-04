#!/usr/bin/env bash
set -euo pipefail

# Builds the AppImage inside an Ubuntu 20.04 container so it also runs on
# systems older than this one. See scripts/appimage/Dockerfile.ubuntu2004 for
# why that is the only way to lower the glibc floor.
#
# Everything the build needs from this machine is bind mounted at the path it
# already has, so the paths inside the container are the paths outside it: the
# repository, the Qt installation, and the linuxdeploy/appimagetool AppImages.
# The build itself is the ordinary scripts/build-appimage.sh - the container
# only changes which compiler and system libraries it finds.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE="${IMAGE:-eumetcastview-appimage-builder:ubuntu20.04}"
QT_ROOT="${QT_ROOT:-$HOME/Qt}"
QT_DIR="${QT_DIR:-$QT_ROOT/6.9.2/gcc_64}"
APPIMAGE_TOOLS="${APPIMAGE_TOOLS:-$HOME/AppImages}"

# Separate from the native build's, so the two CMake caches never meet.
CONTAINER_BUILD_DIR="${CONTAINER_BUILD_DIR:-$REPO_ROOT/build-appimage-ubuntu2004}"
CONTAINER_APPDIR="${CONTAINER_APPDIR:-$REPO_ROOT/AppDir-ubuntu2004}"

ENGINE="${ENGINE:-}"
if [ -z "$ENGINE" ]; then
    if command -v podman >/dev/null 2>&1; then
        ENGINE=podman
    elif command -v docker >/dev/null 2>&1; then
        ENGINE=docker
    else
        echo "Neither podman nor docker found on PATH." >&2
        exit 1
    fi
fi

for dir in "$QT_DIR" "$APPIMAGE_TOOLS"; do
    if [ ! -d "$dir" ]; then
        echo "Not a directory: $dir" >&2
        echo "  Set QT_DIR / APPIMAGE_TOOLS to override." >&2
        exit 1
    fi
done

echo "==> Building image $IMAGE"
"$ENGINE" build -t "$IMAGE" -f "$SCRIPT_DIR/appimage/Dockerfile.ubuntu2004" "$SCRIPT_DIR/appimage"

echo "==> Building the AppImage in $ENGINE ($IMAGE)"
# Rootless podman maps the container's root onto this user, so everything the
# build writes into the repository comes back owned by the invoking user.
"$ENGINE" run --rm \
    -v "$REPO_ROOT:$REPO_ROOT" \
    -v "$QT_ROOT:$QT_ROOT:ro" \
    -v "$APPIMAGE_TOOLS:$APPIMAGE_TOOLS:ro" \
    -w "$REPO_ROOT" \
    -e "PATH=$QT_DIR/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" \
    -e "BUILD_DIR=$CONTAINER_BUILD_DIR" \
    -e "APPDIR=$CONTAINER_APPDIR" \
    -e "LINUXDEPLOY=$APPIMAGE_TOOLS/linuxdeploy-x86_64.AppImage" \
    -e "LINUXDEPLOY_PLUGIN_QT=$APPIMAGE_TOOLS/linuxdeploy-plugin-qt-x86_64.AppImage" \
    -e "APPIMAGETOOL=$APPIMAGE_TOOLS/appimagetool-x86_64.AppImage" \
    -e "OPENSSL3_LIB_DIR=/usr/local/lib" \
    "$IMAGE" \
    "$REPO_ROOT/scripts/build-appimage.sh" "$@"

echo
echo "==> Symbol floor of everything in the AppDir"
# The point of the exercise, so measure it rather than trust it. Over the
# executables *and* every bundled library, since a library dragged in from the
# build host sets the floor just as surely as the binaries do: nothing may ask
# for a glibc newer than 20.04's 2.31, or a libstdc++ newer than its 3.4.28.
appdir_versions() {
    find "$CONTAINER_APPDIR" -type f \( -name '*.so*' -o -path '*/usr/bin/*' \) -print0 \
        | xargs -0 -r -n1 objdump -p 2>/dev/null \
        | grep -oE "$1_[0-9.]+" | sort -u -V | tail -1
}
printf 'highest glibc      %s   (Ubuntu 20.04 has 2.31)\n' "$(appdir_versions GLIBC)"
printf 'highest libstdc++  %s (Ubuntu 20.04 has 3.4.28)\n' "$(appdir_versions GLIBCXX)"

# Qt dlopens these, so their absence is silent until an HTTPS request fails on
# the target machine rather than here.
if [ -f "$CONTAINER_APPDIR/usr/lib/libssl.so.3" ] && [ -f "$CONTAINER_APPDIR/usr/lib/libcrypto.so.3" ]; then
    echo "OpenSSL 3          bundled (HTTPS available)"
else
    echo "OpenSSL 3          MISSING - the TLE download over HTTPS will fail" >&2
fi

echo
echo "Note: the build wrote its binaries to bin/ as usual, so bin/EUMETCastView"
echo "is now the Ubuntu 20.04 build. Rebuild natively when you want to run or"
echo "debug it on this machine again."
