Savage Wheels for GFOS
======================

A port of Savage Wheels, KenamicK Entertainment's top-down car-crashing
derby, from https://github.com/kenamick/savagewheels at commit
d173d0d539caf27cc65db1e87ccb3834d5bf6c5a (game v1.7.1), for GFOS:
AmigaOS 3.2 with GFOpenUp, on a 68040 or 68060 with an FPU, on GFOpenUp's
SDL 2.

Eight armoured cars, each with a driver and a story, fight to the last car
standing in a deadly tournament. Collect bonuses for speed, damage and
repairs, drop mines, and run down the crowd for points. Play the campaign,
a hot-seat match for two, or a melee against the computer's drivers.

Status: host tests pass; built in Cradle's os32-gcc16-open6 stove; not yet
run in a lab.


What you need
-------------

- GFOS: AmigaOS 3.2 with GFOpenUp 0.8.26 or later, which installs
  LIBS:GFOpenGPU/SDL2.module and SDL2_mixer.module.
- A 68040 or 68060 with an FPU (not an LC040 or EC040), an RTG screen
  with room for a 640 x 480 window, and about 24 MB of free memory.
- Nothing else: the pictures, cars, music and sounds are in the drawer.


How to play
-----------

Install it one of three ways:

- GFOpenUp: `GFOpenUpTool INSTALL Parts/SavageWheels` puts it in
  SYS:Games/SavageWheels.
- Run Install-SavageWheels from the archive (it offers Work:Games).
- Or copy the SavageWheels drawer where you want it.

Then double-click SavageWheels. The menu is played with the mouse. Player
one drives with the cursor keys, lays mines with Right Shift and honks
with Right Alt; player two drives with W, A, S, D, Left Shift and Left
Alt. On an Amiga keyboard Alt is the key beside the Amiga keys. F4 turns the music on and off, F5 changes the
arena, F9 the shadows, P pauses, Esc leaves. Help.html in the drawer has
the whole game's help.

The key bindings (bindings.xml), the preferences and the log (debug.html)
are kept in the drawer's User drawer. On the first start the default
bindings are copied there from Data; change that copy. Install-SavageWheels
and GFOpenUp keep User when they update the game.


Building
--------

    sh package/build-amiga.sh        (STOVE=/path/to/os32-gcc16-open6)

or, in Cradle, stove_build with the os32-gcc16-open6 stove and target
package. It fetches upstream at the commit package/savagewheels.lock
names, applies amiga/patches, fetches the game data and checks its
SHA-256, builds with amiga/Makefile.amigaos and packs
SavageWheels-VERSION-68040.lha and the GFOpenUp part Parts/SavageWheels.

Host tests: `sh tests/host/test_all.sh` (Linux, gcc, SDL 2 and SDL2_mixer
development files) checks that the patch applies and the data matches its
SHA-256; builds the whole game with the host's compiler through the same
Makefile; tests the SDL 1.2 calls on SDL 2 (the 5-6-5 screen, colour keys,
surface alpha, the flip, key names) and the first-start copy of the
bindings; reads all eight cars and gfxdata.kdf through the patched readers,
once as they are and once as a big-endian 68k reads them; and starts the
game with SDL's dummy drivers in a drawer laid out as on the Amiga, where
it must load every picture, car and sound and reach its game loop.


What was changed
----------------

- amiga/compat/: Savage Wheels is written for SDL 1.2. sdl12on2.c gives
  it the 1.2 calls it makes, on SDL 2: SDL_SetVideoMode makes the window
  and a 16-bit 5-6-5 screen surface in memory (the game's shadows and
  colour keys are 5-6-5 pixel arithmetic), SDL_Flip copies that to the
  window, and SDL_DisplayFormat, SDL_SetAlpha, the captions, the video
  information calls and the joystick names map to SDL 2's. Upstream's
  `#include "SDL.h"` finds amiga/compat/SDL.h first.
- amiga/patches/0001-amigaos-drawers-keys-byte-order.patch:
  - the data are read from PROGDIR:Data, the bindings, preferences and
    log are kept in PROGDIR:User, and an AmigaDOS path (with a colon)
    counts as a full path;
  - keys are read from SDL 2's keyboard state by SDL 2 key code;
  - the cars (.swv) and gfxdata.kdf were written on 32-bit x86: their
    numbers are little-endian, and the readers now swap them on a
    big-endian 68k (the .swv layout is checked at compile time).
- amiga/amiga_start.c: 256 KB of stack, the $VER string, PROGDIR:User
  made at start and the default bindings copied into it.
- amiga/copy-data.sh: the game data into Data, without logo.avi.
- amiga/: the Makefile, the GlowIcons (our own art), the Installer script
  and its spec. package/: the build, the release archive and the GFOpenUp
  part.

The AmiCradle Team.
