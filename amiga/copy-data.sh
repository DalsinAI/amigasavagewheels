#!/bin/sh
# The game data into the drawer's Data: autos/, graphics/ and sound/ from the
# unpacked savagewheels-gamedata.tar.gz, without graphics/logo.avi (the game
# never opens it).
#   copy-data.sh <unpacked game data> <drawer>/Data
# Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels.
set -eu
SRC=$1; DST=$2
mkdir -p "$DST"
for d in autos graphics sound; do
    [ -d "$SRC/$d" ] || { echo "copy-data.sh: $SRC has no $d/" >&2; exit 1; }
    mkdir -p "$DST/$d"
    for f in "$SRC/$d"/*; do
        case "${f##*/}" in logo.avi) continue ;; esac
        cp "$f" "$DST/$d/"
    done
done
