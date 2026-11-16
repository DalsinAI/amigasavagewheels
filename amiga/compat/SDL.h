/* Savage Wheels' "SDL.h": SDL 2's, with the SDL 1.2 calls it makes
 * (sdl12on2.h). Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#ifndef SAVAGEWHEELS_COMPAT_SDL_H
#define SAVAGEWHEELS_COMPAT_SDL_H
#include_next <SDL.h>
#include "sdl12on2.h"
#ifdef SW_TEST_BIG_ENDIAN
/* tests/host: the readers' swaps on, as on a 68k */
#undef SDL_SwapLE32
#define SDL_SwapLE32(x) SDL_Swap32(x)
#endif
#endif
