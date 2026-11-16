/* Host test of the game-data readers the port patches (amiga/patches): every
 * car in Data/autos loads through CSwv_module::SearchAndLoad and each of its
 * pictures is a BMP inside the file; gfxdata.kdf opens through CKdf_Packeger
 * and each picture it names is a BMP inside it. With SW_TEST_BIG_ENDIAN the
 * readers' little-endian swaps are forced on, and the test feeds them byte-
 * swapped copies, as a 68k sees the x86-written numbers.
 *   test_data <Data drawer>
 * Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#include "Main.h"
#include "CKdf.h"
#include "CSwv_module.h"
#include <string>

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static long file_size(const std::string &p)
{
	FILE *f = fopen(p.c_str(), "rb");
	long n;
	if (!f)
		return -1;
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	fclose(f);
	return n;
}

static bool is_bmp_at(const std::string &p, long pos, long len, long size)
{
	char m[2] = { 0, 0 };
	FILE *f;
	if (pos < 0 || len < 2 || pos + len > size)
		return false;
	if (!(f = fopen(p.c_str(), "rb")))
		return false;
	fseek(f, pos, SEEK_SET);
	fread(m, 1, 2, f);
	fclose(f);
	return m[0] == 'B' && m[1] == 'M';
}

int main(int argc, char **argv)
{
	std::string data(argc > 1 ? argv[1] : "Data");
	std::string autos = data + "/autos";

	CSwv_module swv;
	CHECK(swv.SearchAndLoad(autos.c_str()) == SWV_SUCCESS);
	CHECK(swv.GetVehicles() == 8);
	const SWV_HEADER *v = swv.GetVehiclesData();
	for (unsigned c = 0; v && c < swv.GetVehicles(); c++) {
		std::string p = autos + "/" + v[c].filename;
		long size = file_size(p);
		CHECK(size > 0);
		CHECK(v[c].animation_frames > 0 && v[c].animation_frames < 256);
		CHECK((int)v[c].hp >= 0 && (int)v[c].hp < 16);
		CHECK((int)v[c].max_vel >= 0 && (int)v[c].max_vel < 8);
		int n = v[c].animation_frames * 2 + 3;
		for (int i = 0; v[c].animation_frames > 0 && v[c].animation_frames < 256 && i < n; i++)
			CHECK(is_bmp_at(p, v[c].pfiles[i].pos, v[c].pfiles[i].length, size));
		printf("car %u %.8s %s: %d frames, hp %d\n", c, v[c].vehiclename,
		       v[c].filename, v[c].animation_frames, (int)v[c].hp);
	}

	std::string kdf = data + "/graphics/gfxdata.kdf";
	long ksize = file_size(kdf);
	CKdf_Packeger k;
	CHECK(k.Open(kdf.c_str()) == KDF_SUCCESS);
	/* the menu's and the game's pictures, as the game asks for them */
	const char *names[] = { "fnt.bmp", "advert.bmp", "logo.bmp", "cursor.bmp" };
	int found = 0;
	for (unsigned i = 0; i < sizeof names / sizeof names[0]; i++) {
		long pos = k.GetFilePosition(names[i]);
		if (pos == KDF_ERROR_FILEDOESNOTEXIST)
			continue;
		found++;
		CHECK(is_bmp_at(kdf, pos, k.GetFileSize(names[i]), ksize));
	}
	CHECK(found > 0);
	printf("kdf: %d of %u names checked\n", found, (unsigned)(sizeof names / sizeof names[0]));
	printf("%s: %d failed\n", argv[0], fails);
	return fails != 0;
}
