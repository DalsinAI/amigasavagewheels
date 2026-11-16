#!/usr/bin/env python3
"""A copy of the game data with the x86-written numbers the port's readers
swap (the .swv headers and frame tables, gfxdata.kdf's file count) byte-
swapped, so a host test with SW_TEST_BIG_ENDIAN reads them as a 68k does.
  swap_data.py <Data drawer> <copy>"""
# Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels.
import shutil, struct, sys
from pathlib import Path

src, dst = Path(sys.argv[1]), Path(sys.argv[2])
if dst.exists():
    shutil.rmtree(dst)
shutil.copytree(src, dst)

def swap(b, off):
    b[off:off + 4] = b[off:off + 4][::-1]

for f in sorted((dst / "autos").glob("*.swv")):
    b = bytearray(f.read_bytes())
    frames = struct.unpack_from("<i", b, 108)[0]      # animation_frames
    for off in range(76, 112, 4):                      # the 9 numbers
        swap(b, off)
    for i in range(frames * 2 + 3):                    # pos, length
        swap(b, 116 + i * 264)
        swap(b, 116 + i * 264 + 4)
    f.write_bytes(bytes(b))

k = dst / "graphics" / "gfxdata.kdf"
b = bytearray(k.read_bytes())
swap(b, 5)                                             # num_files after "KDF" + version
k.write_bytes(bytes(b))
print(f"swapped copy in {dst}")
