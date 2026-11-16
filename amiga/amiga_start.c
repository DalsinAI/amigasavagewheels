/* The Amiga port's start: the stack, the version string and PROGDIR:User
 * (the key bindings, the preferences and the log), made before main(). The
 * game reads its bindings from User/bindings.xml: on the first start that
 * is a copy of Data/bindings.xml, and the player's own edits stay there
 * (Install-SavageWheels and GFOpenUp keep User when they update the game).
 * Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#include <stdio.h>
#include <sys/stat.h>
#include "amiga_start.h"

/* A Shell gives 4 KB of stack; SDL 2 and the game want more. libnix's
 * startup sets this much up. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) =
    "$VER: SavageWheels 1.7.1 (16.11.2026)";

int sw_copy_if_missing(const char *from, const char *to)
{
	char buf[4096];
	size_t n;
	FILE *in, *out;

	if ((out = fopen(to, "rb")) != NULL) {
		fclose(out);
		return 0;                       /* the player's own: keep it */
	}
	if ((in = fopen(from, "rb")) == NULL)
		return -1;
	if ((out = fopen(to, "wb")) == NULL) {
		fclose(in);
		return -1;
	}
	while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
		if (fwrite(buf, 1, n, out) != n) {
			fclose(in);
			fclose(out);
			remove(to);
			return -1;
		}
	}
	fclose(in);
	if (fclose(out) != 0) {
		remove(to);
		return -1;
	}
	return 1;
}

#ifdef __amigaos__
static void __attribute__((constructor)) amiga_user_drawer(void)
{
	mkdir("PROGDIR:User", 0755);
	sw_copy_if_missing("PROGDIR:Data/bindings.xml", "PROGDIR:User/bindings.xml");
}
#endif
