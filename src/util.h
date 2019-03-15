// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <time.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "lagscope.h"

void print_flags(struct lagscope_test *test);
void print_usage();
void print_version();
int verify_args(struct lagscope_test *test);
int parse_arguments(struct lagscope_test *test, int argc, char **argv);

void print_iteration_histogram();
void print_test_stats();

int show_percentile(unsigned long, unsigned long);
int show_histogram(int, int, int, unsigned long);
void create_latencies_csv(const char *);
void create_freq_table_json(unsigned long, const char *);
void push(unsigned long);
void latencies_stats_cleanup(void);

double unit_atod(const char *s);
char *retrive_ip_address_str(struct sockaddr_storage *ss, char *ip_str, size_t maxlen);

static inline double get_time_diff(struct timeval *t1, struct timeval *t2)
{
	return fabs((t1->tv_sec + (t1->tv_usec / 1000000.0)) - (t2->tv_sec + (t2->tv_usec / 1000000.0)));
}

static inline void report_progress(struct lagscope_test_runtime *test_runtime)
{
	/*
	 * Only report progress once in every lazy_prog_report_factor of pings
	 * (try to increase ~1%, by estimation)
	 */
	if (test_runtime->ping_elapsed % test_runtime->lazy_prog_report_factor != 0)
		return;

	if (test_runtime->test->test_mode == PING_ITERATION) {
		printf("%s: %lu%% completed.\r",
		test_runtime->test->bind_address,
		test_runtime->ping_elapsed * 100 / test_runtime->test->iteration);
	}
	else {
		double time_elapsed = get_time_diff(&test_runtime->current_time, &test_runtime->start_time);
		printf("%s: %.0f%% completed.\r",
		test_runtime->test->bind_address,
		time_elapsed * 100 / test_runtime->test->duration);
	}
	fflush(stdout);
}
