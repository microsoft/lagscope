#include "common.h"

#ifdef _WIN32
char* optarg = NULL;

int getopt(int argc, char *const argv[], const char *optstr)
{
	static int optind = 1;

	if ((optind >= argc) || (argv[optind][0] != '-') || (argv[optind][0] == 0))
		return -1;

	int opt = argv[optind][1];
	const char *p = strchr(optstr, opt);

	if (p == NULL)
		return '?';

	if (p[1] == ':') {
		optarg = &argv[optind][2];
		if (optarg[0] == 0)
			optarg = NULL;
	}

	optind++;
	return opt;
}

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
