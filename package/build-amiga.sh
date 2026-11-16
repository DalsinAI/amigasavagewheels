#!/bin/sh
# The release archive and the GFOpenUp part, for Cradle's stove_build
# (target package):
#  1. upstream Savage Wheels, fetched at the commit package/savagewheels.lock
#     names (it is never kept in this repository), with amiga/patches
#     applied, and its game data, fetched from the lock's address and kept
#     only when its SHA-256 matches;
#  2. the game built by amiga/Makefile.amigaos;
#  3. the drawer, its icon and the Installer script beside it packed as
#     NAME-VERSION-68040.lha (tools/pack_dist.py), with its .sha256 and a
#     short .readme;
#  4. OUT/Parts/SavageWheels, a GFOpenUp part: its Part file and the
#     drawer's files for SYS:Games/SavageWheels, installed with
#       GFOpenUpTool INSTALL Parts/SavageWheels
#     It needs the GFOpenGPU part, whose SDL2.module draws the game.
# Nothing is published from here.
# Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
. "$HERE/package.env"
. "$HERE/savagewheels.lock"
OUT=${OUT:-"$ROOT/build-amiga-package"}
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
STOVE=${STOVE:-"$HOME/AmiCradle/kitchen/stoves/os32-gcc16-open6"}
# outside the checkout: Cradle cleans it (git clean -ffdx) before every job
WORK=${WORK:-"$HOME/.cache/amigasavagewheels"}
mkdir -p "$WORK"
cd "$ROOT"

# 1. upstream at the locked commit, patched afresh every time
SW="$WORK/savagewheels"
if [ "$(git -C "$SW" rev-parse HEAD 2>/dev/null)" != "$SW_COMMIT" ]; then
    rm -rf "$SW"; mkdir -p "$SW"
    git -C "$SW" init -q
    git -C "$SW" fetch -q --depth 1 "$SW_URL" "$SW_COMMIT"
    git -C "$SW" checkout -q FETCH_HEAD
fi
git -C "$SW" checkout -q -- . && git -C "$SW" clean -qfdx
for p in "$ROOT"/amiga/patches/*.patch; do
    git -C "$SW" apply "$p"
done
#    and the game data
TGZ="$WORK/savagewheels-gamedata.tar.gz"
if ! echo "$SW_DATA_SHA256  $TGZ" | sha256sum -c --quiet - 2>/dev/null; then
    curl -sSfL -o "$TGZ.part" "$SW_DATA_URL"
    echo "$SW_DATA_SHA256  $TGZ.part" | sha256sum -c --quiet -
    mv "$TGZ.part" "$TGZ"
fi
DATA="$WORK/data"
rm -rf "$DATA"; mkdir -p "$DATA"
tar xzf "$TGZ" -C "$DATA"

# 2. the game (a stove's compiler is in its prefix/, or STOVE is the prefix)
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then PREFIX="$STOVE/prefix"; else PREFIX="$STOVE"; fi
rm -rf "$DIST"
make -f amiga/Makefile.amigaos SW="$SW" DATA="$DATA" STOVE="$STOVE" PREFIX="$PREFIX" -j4 $TARGET > "$OUT/build-output.txt" 2>&1 || { tail -n 60 "$OUT/build-output.txt"; exit 1; }

# 3. the archive
python3 "$ROOT/tools/pack_dist.py" --dist "$DIST" --name "$NAME" --out "$OUT"

# 4. the GFOpenUp part
sh "$ROOT/package/write-part.sh" "$DIST/$NAME" "$OUT/Parts/$NAME" "$NAME" "$PORT_VERSION" \
    "Savage Wheels: the top-down car-crashing derby, campaign, 1 or 2 players"
echo "part: $(grep -c '^copy' "$OUT/Parts/$NAME/Part") files, $(du -sk "$OUT/Parts/$NAME" | cut -f1) KB"
ls -l "$DIST/$NAME/$NAME"
