#include "common.h"

#ifdef _WIN32
long long time_in_nanosec(void)
{
	LARGE_INTEGER Time, Frequency;

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&Time);

	return (Time.QuadPart * 1000000000I64 / Frequency.QuadPart);
}

static int vasprintf(char **strp, const char *format, va_list ap)
{
	int len = _vscprintf(format, ap);
	if (len == -1)
		return -1;
	char *str = (char*)malloc((size_t) len + 1);
	if (!str)
		return -1;
	int rv = vsnprintf(str, len + 1, format, ap);
	if (rv == -1) {
		free(str);
		return -1;
	}
	*strp = str;
	return rv;
}

int asprintf(char **strp, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int rv = vasprintf(strp, format, ap);
	va_end(ap);
	return rv;
}

#endif
