#!/usr/bin/env bash
# Fetch CrossPoint at the pinned commits and apply the Pocket426 board patches.
#   firmware/setup.sh            -> firmware/crosspoint-reader (patched checkout)
#   cd firmware/crosspoint-reader && pio run -e pocket426 [-t upload]
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
CROSSPOINT_COMMIT=d1509d0735bd0b7832c6765e2aa83e1f0c008eff
FREEINK_SDK_COMMIT=87c4493a6a5aa0c7c0e61aacc4a24e2c273e6895
dst="$here/crosspoint-reader"
if [ ! -d "$dst/.git" ]; then
  git clone --filter=blob:none https://github.com/crosspoint-reader/crosspoint-reader.git "$dst"
fi
git -C "$dst" fetch --depth 1 origin "$CROSSPOINT_COMMIT"
git -C "$dst" checkout --force "$CROSSPOINT_COMMIT"
git -C "$dst" submodule update --init --depth 1 freeink-sdk
git -C "$dst/freeink-sdk" fetch --depth 1 origin "$FREEINK_SDK_COMMIT"
git -C "$dst/freeink-sdk" checkout --force "$FREEINK_SDK_COMMIT"
git -C "$dst" apply --whitespace=nowarn "$here/patches/crosspoint.patch"
git -C "$dst/freeink-sdk" apply --whitespace=nowarn "$here/patches/freeink-sdk.patch"
echo "patched CrossPoint ready at $dst  (pio run -e pocket426)"
