#!/bin/sh
# Host tests (Linux, gcc/g++, SDL 2 and SDL2_mixer development files):
#  1. upstream at the locked commit takes amiga/patches;
#  2. the game data fetched from the lock's address has the lock's SHA-256;
#  3. the whole game, amiga/compat and amiga_start.c build with the host's
#     compiler through amiga/Makefile.amigaos (NOIX=, the host's SDL 2);
#  4. test_compat: SDL 1.2's video calls on SDL 2 (5-6-5 screen, colour key,
#     surface alpha, flip, captions), SDL 1.2 key names on SDL 2 key codes,
#     and the first-start copy of the key bindings;
#  5. test_data: all 8 cars and gfxdata.kdf read through the patched readers,
#     then again with their little-endian swaps forced on over a byte-swapped
#     copy of the data (as a big-endian 68k reads it);
#  6. the game itself starts with SDL's dummy drivers in a drawer laid out as
#     on the Amiga (PROGDIR:Data, PROGDIR:User are plain directory names
#     here), loads every picture, car and sound and reaches its game loop.
# Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
. "$ROOT/package/savagewheels.lock"
T=${WORK:-"${TMPDIR:-/tmp}/amigasavagewheels-test"}
mkdir -p "$T"
cd "$ROOT"
ok() { echo "ok: $*"; }

# 1. upstream + patches
SW="$T/savagewheels"
if [ "$(git -C "$SW" rev-parse HEAD 2>/dev/null)" != "$SW_COMMIT" ]; then
    rm -rf "$SW"; mkdir -p "$SW"
    git -C "$SW" init -q
    git -C "$SW" fetch -q --depth 1 "$SW_URL" "$SW_COMMIT"
    git -C "$SW" checkout -q FETCH_HEAD
fi
git -C "$SW" checkout -q -- . && git -C "$SW" clean -qfdx
for p in amiga/patches/*.patch; do git -C "$SW" apply "$ROOT/$p"; done
ok "patches apply to $SW_COMMIT"

# 2. the game data
TGZ="$T/savagewheels-gamedata.tar.gz"
[ -f "$TGZ" ] || curl -sSfL -o "$TGZ" "$SW_DATA_URL"
echo "$SW_DATA_SHA256  $TGZ" | sha256sum -c --quiet -
DATA="$T/data"
rm -rf "$DATA"; mkdir -p "$DATA"; tar xzf "$TGZ" -C "$DATA"
ok "game data SHA-256 matches"

# 3. the build
B="$T/build"
HOST="CC=gcc CXX=g++ CPU= NOIX= SDL2KIT=/usr OPT=-O1 FTIME="
make -s -f amiga/Makefile.amigaos -j8 SW="$SW" BUILD="$B" $HOST \
    DEFS="-D__amigaos__ -DLINUX_BUILD -DWITH_SDLMIXER -DARCH_X64" \
    LIBS="-lSDL2_mixer -lSDL2 -lm" "$B/SavageWheels.debug" > "$T/make.log" 2>&1 || { tail -30 "$T/make.log"; exit 1; }
ok "game builds on the host"

# 4. compat
SDLF=$(sdl2-config --cflags) SDLL=$(sdl2-config --libs)
gcc -O1 -Wall -Iamiga/compat -Iamiga $SDLF -DSW_FTIME=sw_ftime -o "$T/test_compat" tests/host/test_compat.c \
    amiga/compat/sdl12on2.c amiga/amiga_start.c amiga/compat/ftime.c $SDLL
SDL_VIDEODRIVER=dummy "$T/test_compat" "$T"

# 5. data readers, little- and big-endian
CXXF="-O1 -std=gnu++11 -w -D__amigaos__ -DLINUX_BUILD -DWITH_SDLMIXER -DARCH_X64 -Iamiga/compat -I$SW/src -I$SW/tinyxml -I$B/obj $SDLF"
O="$B/obj/src"
g++ $CXXF -o "$T/test_data" tests/host/test_data.cpp $O/CSwv_module.o $O/CKdf.o $O/Utils.o $SDLL
"$T/test_data" "$DATA"
python3 tests/host/swap_data.py "$DATA" "$T/data-be" > /dev/null
g++ $CXXF -DSW_TEST_BIG_ENDIAN -c -o "$T/swv-be.o" "$SW/src/CSwv_module.cpp"
g++ $CXXF -DSW_TEST_BIG_ENDIAN -c -o "$T/kdf-be.o" "$SW/src/CKdf.cpp"
g++ $CXXF -o "$T/test_data_be" tests/host/test_data.cpp "$T/swv-be.o" "$T/kdf-be.o" $O/Utils.o $SDLL
"$T/test_data_be" "$T/data-be"

# 6. the game starts
R="$T/run"
rm -rf "$R"; mkdir -p "$R/PROGDIR:Data"
sh amiga/copy-data.sh "$DATA" "$R/PROGDIR:Data"
cp "$SW/bin/bindings.xml" "$R/PROGDIR:Data/"
(cd "$R" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$B/SavageWheels.debug" & pid=$!; sleep 6; kill -9 $pid) 2>/dev/null || true
LOG="$R/PROGDIR:User/debug.html"
[ -f "$R/PROGDIR:User/bindings.xml" ] || { echo "FAIL: no User/bindings.xml"; exit 1; }
grep -q "Starting Game loop" "$LOG" || { echo "FAIL: no game loop"; sed 's/<[^>]*>//g' "$LOG" | tail -20; exit 1; }
if sed 's/<[^>]*>//g' "$LOG" | grep -i -E "fail|error|missing" | grep -v -i joystick; then
    echo "FAIL: the log above"; exit 1
fi
[ ! -e "$R/PROGDIR:Data/graphics/logo.avi" ] || { echo "FAIL: logo.avi copied"; exit 1; }
ok "the game reaches its game loop with every file loaded"
echo "all host tests passed"
