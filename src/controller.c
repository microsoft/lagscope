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

void timer_fired(int signo)
{
	(void) signo; //Silence unused parameter compile warning
	turn_off_light();
}

