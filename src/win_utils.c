#include "common.h"
#include "controller.h"

#ifdef _WIN32
static int duration_ms;
void timer_thread(void *parg)
{
	MSG Msg;
	UINT_PTR TimerId = SetTimer(NULL, 0, duration_ms, (TIMERPROC)timer_fired);
	while(GetMessage(&Msg, NULL, 0, 0))
		DispatchMessage(&Msg);
	_endthread();
}

void run_test_timer(int duration)
{
	duration_ms = 1000*duration;
	_beginthread(timer_thread, 0, NULL);
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

int set_affinity(int cpuid)
{
	if (0 == SetProcessAffinityMask(GetCurrentProcess(), (UINT_PTR)1 << cpuid))
		return 0;
	return 1;
}
#endif
