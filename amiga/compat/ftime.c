/* ftime(), which upstream's log stamps use and libnix lacks: the time of
 * day from gettimeofday(). (tests/host builds it as sw_ftime.)
 * Copyright (c) 2026 AmiCradle Team. MIT, like Savage Wheels. */
#include <sys/time.h>
#include <sys/timeb.h>

#ifndef SW_FTIME
#define SW_FTIME ftime
#endif

int SW_FTIME(struct timeb *tb)
{
	struct timeval tv;

	if (gettimeofday(&tv, 0) != 0)
		return -1;
	tb->time = tv.tv_sec;
	tb->millitm = (unsigned short)(tv.tv_usec / 1000);
	tb->timezone = 0;
	tb->dstflag = 0;
	return 0;
}
