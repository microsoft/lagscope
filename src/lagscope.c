// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#include "lagscope.h"

struct lagscope_test *new_lagscope_test()
{
	struct lagscope_test *test;
	test = (struct lagscope_test *) malloc(sizeof(struct lagscope_test));
	if (!test)
		return NULL;

	memset(test, 0, sizeof(struct lagscope_test));
	return test;
}

void default_lagscope_test(struct lagscope_test *test)
{
	test->server_role      = false;
	test->client_role      = false;
	test->daemon           = false;
	test->cpu_affinity     = -1; //no hard cpu affinity
	test->bind_address     = "0.0.0.0";

	test->domain           = AF_INET; //ipv4
	test->protocol         = TCP;
	test->server_port      = DEFAULT_SERVER_PORT;
	test->recv_buf_size    = DEFAULT_RECV_BUFFER_SIZE_BYTES; //64K
	test->send_buf_size    = DEFAULT_SEND_BUFFER_SIZE_BYTES; //128K

	test->test_mode	       = PING_ITERATION;		 //run ping test with specified	number of iterations
	test->msg_size         = DEFAULT_MESSAGE_SIZE_BYTES;	 //4 bytes
	test->duration         = DEFAULT_TEST_DURATION_SEC;	 //60 s
	test->iteration        = DEFAULT_TEST_ITERATION;	 //1000
	test->interval         = DEFAULT_TEST_INTERVAL_SEC;	 //1 s

	test->hist             = false;
	test->hist_start       = HIST_DEFAULT_START_AT;	 	 //0
	test->hist_len         = HIST_DEFAULT_INTERVAL_LEN;	 //100
	test->hist_count       = HIST_DEFAULT_INTERVAL_COUNT;	 //10

	test->perc             = false;
	test->freq_table_dump  = false;

	test->raw_dump	       = false;
	test->csv_file_name    = DEFAULT_CSV_FILENAME;

	test->verbose          = false;
}

struct lagscope_test_server *new_lagscope_server(struct lagscope_test *test)
{
	struct lagscope_test_server *s;
	s = (struct lagscope_test_server *) malloc(sizeof(struct lagscope_test_server));
	if (!s)
	 	return NULL;

	memset(s, 0, sizeof(struct lagscope_test_server));
	s->test = test;

	return s;
}

struct lagscope_test_client *new_lagscope_client(struct lagscope_test *test)
{
	struct lagscope_test_client *c;
	c = (struct lagscope_test_client *) malloc(sizeof(struct lagscope_test_client));
	if (!c)
	 	return NULL;

	memset(c, 0, sizeof(struct lagscope_test_client));
	c->test = test;

	return c;
}

struct lagscope_test_runtime *new_test_runtime(struct lagscope_test *test)
{
	struct lagscope_test_runtime *r;
	r = (struct lagscope_test_runtime *) malloc(sizeof(struct lagscope_test_runtime));
	if (!r)
		return NULL;

	memset(r, 0, sizeof(struct lagscope_test_runtime));
	r->test = test;

	/* calculate the lazy_prog_report_factor */
	unsigned long total_pings = 1;
	/*
	 * For PING_ITERATION, we know the total number of pings will be executed;
	 * For TIME_DURATION, we estimate the total number of pings:
	 *   a) total test duration time, divided by
	 *   b) interval between each pings, or 1 ms (0.001 sec) if the interval == 0.
	 */
	if (test->test_mode == PING_ITERATION) {
		total_pings = test->iteration;
	}
	else {
		if (test->interval !=0)
			total_pings = test->duration / test->interval;
		else
			total_pings = test->duration / 0.001;
	}
	/*
	 * We report the percentage of test progress.
	 * So basically it is only needed to be updated in every 1%.
	 * That means, in every lazy_prog_report_factor,
	 * the progress percentage needs to be updated only once.
	 */
	if (total_pings > 100)
		r->lazy_prog_report_factor = total_pings / 100;
	else
		r->lazy_prog_report_factor = 1;

	return r;
}
