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

double unit_atod(const char *s);
char *retrive_ip_address_str(struct sockaddr_storage *ss, char *ip_str, size_t maxlen);
double get_time_diff(struct timeval *t1, struct timeval *t2);
