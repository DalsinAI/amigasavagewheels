/* The SDL 1.2 calls Savage Wheels makes, on SDL 2 (GFOpenUp's SDL2.module).
 *
 * The game draws into one 16-bit 5-6-5 "screen" surface, as it did on SDL
 * 1.2 (its shadows and colour keys are 5-6-5 pixel arithmetic). This keeps
 * that surface in memory and SDL_Flip() copies it to the window's surface.
 * The keyboard state is SDL 2's, read through sdl12on2_key_down() with SDL
 * 2 key codes (the game's bindings name keys by their SDLK_ names).
 *
 * Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#ifndef SDL12ON2_H
#define SDL12ON2_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDL_SWSURFACE
#define SDL_SWSURFACE   0u
#endif
#define SDL_HWSURFACE   0u
#define SDL_DOUBLEBUF   0u
#define SDL_ANYFORMAT   0u
#define SDL_FULLSCREEN  0x80000000u
#define SDL_SRCCOLORKEY SDL_TRUE
#define SDL_SRCALPHA    0x00010000u

/* SDL 1.2's keypad names */
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9

typedef struct SDL_VideoInfo {
    Uint32 hw_available, wm_available, blit_hw, blit_hw_CC, blit_hw_A,
           blit_sw, blit_sw_CC, blit_sw_A, blit_fill, video_mem;
    SDL_PixelFormat *vfmt;
    int current_w, current_h;
} SDL_VideoInfo;

/* the window and the 1.2 screen surface SDL_SetVideoMode() gives */
extern SDL_Window *sdl12on2_window;
extern SDL_Surface *sdl12on2_screen;

SDL_Surface *SDL_SetVideoMode(int w, int h, int bpp, Uint32 flags);
int SDL_Flip(SDL_Surface *screen);
int SDL_WM_ToggleFullScreen(SDL_Surface *screen);
void SDL_WM_SetCaption(const char *title, const char *icon);
SDL_Surface *SDL_DisplayFormat(SDL_Surface *s);
int SDL_SetAlpha(SDL_Surface *s, Uint32 flags, Uint8 alpha);
const SDL_VideoInfo *SDL_GetVideoInfo(void);
char *SDL_VideoDriverName(char *buf, int len);
SDL_Rect **SDL_ListModes(const SDL_PixelFormat *fmt, Uint32 flags);
int SDL_VideoModeOK(int w, int h, int bpp, Uint32 flags);
/* The 5-6-5 format the screen has (the window's own may differ). */
Uint32 sdl12on2_screen_format(int bpp);
/* SDL 1.2's keys[SDLK_x], with an SDL 2 key code. */
int sdl12on2_key_down(const Uint8 *keys, int key);

#define SDL_GetKeyState(n)      ((Uint8 *)SDL_GetKeyboardState(n))
#define SDL_JoystickName(i)     SDL_JoystickNameForIndex(i)
#define SDL_JoystickOpened(i)   0

#ifdef __cplusplus
}
#endif
#endif
