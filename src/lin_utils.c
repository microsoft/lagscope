#include "common.h"
#include "controller.h"

#ifndef _WIN32
long long time_in_usec(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (1000000 * tv.tv_sec + tv.tv_usec);
}

void run_test_timer(int duration)
{
	struct itimerval it_val;

	it_val.it_value.tv_sec = duration;
	it_val.it_value.tv_usec = 0;
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;

	if (signal(SIGALRM, timer_fired) == SIG_ERR) {
		PRINT_ERR("unable to set test timer: signal SIGALRM failed");
		exit(1);
	}
	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		PRINT_ERR("unable to set test timer: setitimer ITIMER_REAL failed");
		exit(1);
	}
}
#endif
