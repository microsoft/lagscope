#include "common.h"
#include "controller.h"

#ifndef _WIN32
double time_in_usec(void)
{
	long ns;
	time_t sec;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);
	sec = spec.tv_sec;
	ns = spec.tv_nsec;

	return (sec * 1000000 + ns / 1000.0);
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

int set_affinity(int cpuid)
{
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpuid, &cpuset);
	if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
		return 0;
	return 1;
}
#endif
