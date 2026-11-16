/* Host test of amiga/compat/sdl12on2.c (SDL 1.2's video calls on SDL 2) and
 * of amiga_start.c's sw_copy_if_missing(), with SDL 2's dummy video driver.
 * Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "amiga_start.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static void test_video(void)
{
	SDL_Surface *screen, *bmp, *disp;
	Uint16 *px;
	char name[64];

	screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT);
	CHECK(screen != NULL);
	if (!screen)
		return;
	/* the game's shadows and colour keys are 5-6-5 arithmetic */
	CHECK(screen->w == 640 && screen->h == 480);
	CHECK(screen->format->BytesPerPixel == 2);
	CHECK(screen->format->Rmask == 0xF800 && screen->format->Gmask == 0x07E0 && screen->format->Bmask == 0x001F);
	CHECK(SDL_VideoModeOK(640, 480, 16, 0) == 16);
	CHECK(SDL_ListModes(NULL, 0) == (SDL_Rect **)-1);
	CHECK(SDL_GetVideoInfo() != NULL);
	CHECK(SDL_VideoDriverName(name, sizeof name) != NULL && strcmp(name, "dummy") == 0);

	/* a 24-bit picture with a magenta key, as LoadBitmap makes them */
	bmp = SDL_CreateRGBSurface(0, 4, 1, 24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
	CHECK(bmp != NULL);
	SDL_FillRect(bmp, NULL, SDL_MapRGB(bmp->format, 255, 0, 255));
	CHECK(SDL_SetColorKey(bmp, SDL_SRCCOLORKEY, SDL_MapRGB(bmp->format, 255, 0, 255)) == 0);
	CHECK(SDL_SetAlpha(bmp, SDL_SRCALPHA, 128) == 0);
	disp = SDL_DisplayFormat(bmp);
	CHECK(disp != NULL && disp->format->format == screen->format->format);
	if (disp) {
		Uint32 key = 0;
		Uint8 a = 0;
		SDL_BlendMode m;
		CHECK(SDL_GetColorKey(disp, &key) == 0 && key == 0xF81F);   /* magenta in 5-6-5 */
		CHECK(SDL_GetSurfaceAlphaMod(disp, &a) == 0 && a == 128);
		CHECK(SDL_GetSurfaceBlendMode(disp, &m) == 0 && m == SDL_BLENDMODE_BLEND);
		/* a keyed blit leaves the screen alone */
		SDL_FillRect(screen, NULL, 0x1234);
		SDL_SetAlpha(disp, 0, 255);
		SDL_BlitSurface(disp, NULL, screen, NULL);
		px = (Uint16 *)screen->pixels;
		CHECK(px[0] == 0x1234 && px[3] == 0x1234);
		SDL_FreeSurface(disp);
	}
	SDL_FreeSurface(bmp);

	CHECK(SDL_Flip(screen) == 0);
	SDL_WM_SetCaption("Savage Wheels v1.7", "None");
	CHECK(strcmp(SDL_GetWindowTitle(sdl12on2_window), "Savage Wheels v1.7") == 0);
	/* a second mode switch reuses the window */
	{
		SDL_Window *w = sdl12on2_window;
		CHECK(SDL_SetVideoMode(320, 240, 16, 0) != NULL && sdl12on2_window == w);
	}
}

static void test_keys(void)
{
	Uint8 state[SDL_NUM_SCANCODES];

	memset(state, 0, sizeof state);
	state[SDL_SCANCODE_UP] = 1;
	state[SDL_SCANCODE_A] = 1;
	state[SDL_SCANCODE_KP_5] = 1;
	CHECK(sdl12on2_key_down(state, SDLK_UP));
	CHECK(sdl12on2_key_down(state, SDLK_a));
	CHECK(sdl12on2_key_down(state, SDLK_KP5));       /* 1.2 name */
	CHECK(!sdl12on2_key_down(state, SDLK_DOWN));
	CHECK(!sdl12on2_key_down(state, SDLK_ESCAPE));
	CHECK(!sdl12on2_key_down(state, 0));
	CHECK(!sdl12on2_key_down(NULL, SDLK_UP));
	/* the splash screen asks keys 0-254: none is down */
	memset(state, 0, sizeof state);
	{
		int i, any = 0;
		for (i = 0; i < 255; i++)
			any |= sdl12on2_key_down(state, i);
		CHECK(!any);
	}
}

static void test_copy(const char *dir)
{
	char from[512], to[512], buf[16];
	FILE *f;

	snprintf(from, sizeof from, "%s/from.xml", dir);
	snprintf(to, sizeof to, "%s/to.xml", dir);
	remove(to);
	f = fopen(from, "wb");
	fputs("<bindings/>", f);
	fclose(f);
	CHECK(sw_copy_if_missing(from, to) == 1);
	f = fopen(to, "rb");
	CHECK(f && fread(buf, 1, sizeof buf, f) == 11 && memcmp(buf, "<bindings/>", 11) == 0);
	if (f)
		fclose(f);
	/* the player's own copy is kept */
	f = fopen(to, "wb");
	fputs("mine", f);
	fclose(f);
	CHECK(sw_copy_if_missing(from, to) == 0);
	f = fopen(to, "rb");
	CHECK(f && fread(buf, 1, sizeof buf, f) == 4);
	if (f)
		fclose(f);
	snprintf(from, sizeof from, "%s/none.xml", dir);
	remove(to);
	CHECK(sw_copy_if_missing(from, to) == -1);
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("FAIL SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	test_video();
	test_keys();
	SDL_Quit();
	test_copy(argc > 1 ? argv[1] : ".");
	printf("%s: %d failed\n", argv[0], fails);
	return fails != 0;
}
