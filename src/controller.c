// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#include "controller.h"

static int run_light = 0;

void turn_on_light( void )
{
	run_light = 1;
}

void turn_off_light( void )
{
	run_light = 0;
}

int is_light_turned_on( void )
{
	return run_light;
}

/************************************************************/
//		timer and signal handle
/************************************************************/
void sig_handler(int signo)
{
	//Ctrl+C
	if (signo == SIGINT) {
		PRINT_INFO("Interrupted by Ctrl+C");

		if (is_light_turned_on())
			turn_off_light();
		else
			exit (1);
	}
}

void timer_fired()
{
	turn_off_light();
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
