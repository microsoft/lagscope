// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "const.h"
#include "logger.h"

/* maintain the test parameters accepted from user */
struct lagscope_test
{
	bool    server_role;        /* '-s' for client */
	bool    client_role;        /* '-r' for server */
	char    *bind_address;      /* server address, following by '-s' or '-r' */
	bool    daemon;             /* '-D' for daemon mode */
	int     cpu_affinity;       /* '-f' for CPU affinity */

	int     domain;              /* default for AF_INET, or '-6' for AF_INET6 */
	int     protocol;            /* default for SOCK_STREAM for TCP, or '-u' for SOCK_DGRAM for UDP (does not support UDP for now) */
	uint    server_port;         /* '-p' for server listening base port */
	double  recv_buf_size;       /* '-b' for receive buffer option */
	double  send_buf_size;       /* '-B' for send buffer option */

	/* client-only parameters */
	int     test_mode;           /* lagscope client will decide a test mode based on user input */
	int     msg_size;            /* '-z' for message size */
	int     duration;            /* '-t' for total duration in sec of test */
	unsigned long iteration;     /* '-n' for test iteration */
	int     interval;            /* '-i' for ping interval */

	bool    hist;                /* '-H' for histogram report */
	int     hist_start;          /* '-a' for histogram 1st interval start value */
	int     hist_len;            /* '-l' for length of histogram intervals */
	int     hist_count;          /* '-c' for count of histogram intervals */

	bool	perc;                /* '-P' for getting percentiles */
	/* end of client-only parameters */

	bool    verbose;             /* '-V' for verbose logging */
};

struct lagscope_test_server{
	struct  lagscope_test *test;

	int     listener;     /* this is the socket to listen on port to accept new connections */
	int     max_fd;       /* track the max socket fd */
	fd_set  read_set;     /* set of read sockets */
	fd_set  write_set;    /* set of write sockets */
};

struct lagscope_test_client{
	struct	lagscope_test *test;

	int     test_mode;           /* lagscope client will decide a test mode based on user input  */
};

struct lagscope_test_runtime{
	struct lagscope_test *test;
	struct timeval start_time;
	struct timeval current_time;
	unsigned long  ping_elapsed;

	unsigned long lazy_prog_report_factor;
};

struct lagscope_test *new_lagscope_test();
void default_lagscope_test(struct lagscope_test *test);

struct lagscope_test_server *new_lagscope_server(struct lagscope_test *test);
struct lagscope_test_client *new_lagscope_client(struct lagscope_test *test);
struct lagscope_test_runtime *new_test_runtime(struct lagscope_test *test);
