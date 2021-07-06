// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#include "util.h"
#include "logger.h"

void print_flags(struct lagscope_test *test)
{
	if (test->server_role)
 		printf("%s\n", "*** receiver role");
	if (test->client_role)
		printf("%s\n", "*** sender role");
	if (test->daemon)
		printf("%s\n", "*** run as daemon");

	if (test->cpu_affinity == -1)
		printf("%s:\t\t\t %s\n", "cpu affinity", "No");
	else
		printf("%s:\t\t\t %d\n", "cpu affinity", test->cpu_affinity);

	printf("%s:\t\t\t %s\n", "server address", test->bind_address);

	if (test->domain == AF_INET)
		printf("%s:\t\t\t\t %s\n", "domain", "IPv4");
	if (test->domain == AF_INET6)
		printf("%s:\t\t\t\t %s\n", "domain", "IPv6");
	if (test->protocol == TCP)
		printf("%s:\t\t\t %s\n", "protocol", "TCP");
	else //(test->domain == UDP)
		printf("%s:\t\t\t %s\n", "protocol", "UDP(*not supported yet*)");

	printf("%s:\t\t\t %d\n", "server port", test->server_port);
	if (test->client_role && test->client_port > 0)
		printf("%s:\t\t\t %d\n", "client port", test->client_port);

	if (test->server_role)
		printf("%s:\t %d\n", "receiver socket buffer (bytes)", test->recv_buf_size);
	if (test->client_role)
		printf("%s:\t %d\n", "sender socket buffer (bytes)", test->send_buf_size);

	printf("%s:\t\t %d\n", "message size (bytes)", test->msg_size);

	if (test->client_role) {
		if (test->test_mode == TIME_DURATION) {
			printf("%s:\t\t\t %s\n", "test mode", "TIME DURATION");
			printf("%s:\t\t %d\n", "test duration (sec)", test->duration);
		}
		else {
			printf("%s:\t\t\t %s\n", "test mode", "PING ITERATION");
			printf("%s:\t\t\t %lu\n", "test iteration", test->iteration);
		}
		printf("%s:\t\t %d\n", "test interval (sec)", test->interval);
		if (test->hist) {
			printf("%s:\t\t %s\n", "histogram report", "enabled");
			printf("%s: %d\n", "histogram 1st interval start at", test->hist_start);
			printf("%s:\t %d\n", "histogram length of intervals", test->hist_len);
			printf("%s:\t %d\n", "histogram count of intervals", test->hist_count);
		}
	}

	printf("%s:\t\t\t %s\n", "verbose mode", test->verbose ? "enabled" : "disabled");
	printf("---------------------------------------------------------\n");
}

void print_usage()
{
	printf("Author: %s\n", AUTHOR_NAME);
	printf("lagscope: [-r|-s|-D|-f|-6|-u|-p|-o|-b|-z|-t|-n|-i|-R|-P|-H|-a|-l|-c|-V|-h]\n\n");
	printf("\t-r   Run as a receiver\n");
	printf("\t-s   Run as a sender\n");
	printf("\t-D   Run as daemon (Linux only)\n");
	printf("\t-f   Processor number to affinitize to (default: no affinity)\n");

	printf("\t-6   IPv6 mode    [default: IPv4]\n");
	//printf("\t-u   UDP mode    [default: TCP] NOT SUPPORTED YET\n");
	printf("\t-p   Server port number    [default: %d]\n", DEFAULT_RCV_PORT);
	printf("\t-o   Client port number    [default: %d]\n", DEFAULT_SRC_PORT);
	printf("\t-b   <buffer size in bytes>    [default: %d (receiver); %d (sender)]\n", DEFAULT_RECV_BUFFER_SIZE_BYTES, DEFAULT_SEND_BUFFER_SIZE_BYTES);
	printf("\t-z   <message size>        [default: %d bytes]\n", DEFAULT_MESSAGE_SIZE_BYTES);

	printf("\t-t   [SENDER ONLY] test duration       [default: %d second(s)]\n", DEFAULT_TEST_DURATION_SEC);
	printf("\t-n   [SENDER ONLY] ping iteration      [default: %d]\n", DEFAULT_TEST_ITERATION);
	printf("\t-i   [SENDER ONLY] test interval       [default: %d second(s)]\n", DEFAULT_TEST_INTERVAL_SEC);
	printf("\t     '-n' will be ignored if '-t' provided\n");

	printf("\t-R   [SENDER ONLY] dumps raw latencies into csv file\n");

	printf("\t-H   [SENDER ONLY] print histogram of per-iteration latency values\n");
	printf("\t-a   [SENDER ONLY] histogram 1st interval start value	[default: %d]\n", HIST_DEFAULT_START_AT);
	printf("\t-l   [SENDER ONLY] length of histogram intervals	[default: %d]\n", HIST_DEFAULT_INTERVAL_LEN);
	printf("\t-c   [SENDER ONLY] count of histogram intervals\t	[default: %d] [max: %d]\n", HIST_DEFAULT_INTERVAL_COUNT, HIST_MAX_INTERVAL_COUNT_USER);
	printf("\t-P   [SENDER ONLY] prints 50th, 75th, 90th, 95th, 99th, 99.9th, 99.99th, 99.999th percentile of latencies\n");
	printf("\t     Dump latency frequency table to a json file if specified after '-P'\n");

	printf("\t-V   Verbose mode\n");
	printf("\t-h   Help, tool usage\n");

	printf("Example:\n");
	printf("\treceiver:\n");
	printf("\t1) ./lagscope -r\n");
	printf("\t2) ./lagscope -r192.168.1.1\n");
	printf("\t3) ./lagscope -r -D -f0 -6 -p6789 -V\n");
	printf("\tsender:\n");
	printf("\t1) ./lagscope -s192.168.1.1\n");
	printf("\t2) ./lagscope -s192.168.1.1 -t600 -i1 -V\n");
	printf("\t3) ./lagscope -s192.168.1.1 -n1000 -6 -i2 -V\n");
	printf("\t4) ./lagscope -s192.168.1.1 -H -a10 -l1 -c98\n");
	printf("\t5) ./lagscope -s192.168.1.1 -Pfreq_table.json\n");
	printf("\t6) ./lagscope -s192.168.1.1 -Rraw_latency_values.csv\n");

	printf("\nNote: There should be no space between option and its value\n");
}

void print_version()
{
	printf("%s %s\n", TOOL_NAME, TOOL_VERSION);
	printf("---------------------------------------------------------\n");
}

/* Check flag or role compatibility; set default value for some params */
int verify_args(struct lagscope_test *test)
{
	if (test->server_role && test->client_role) {
		PRINT_ERR("both sender and receiver roles provided");
		return ERROR_ARGS;
	}

	if (test->domain == AF_INET6 && !strstr(test->bind_address, ":")) {
		PRINT_ERR("invalid ipv6 address provided");
		return ERROR_ARGS;
	}

	if (test->domain == AF_INET && !strstr(test->bind_address, ".")) {
		PRINT_ERR("invalid ipv4 address provided");
		return ERROR_ARGS;
	}

	if (test->protocol == UDP) {
		PRINT_ERR("UDP is not supported so far");
		return ERROR_ARGS;
	}

	if (!test->server_role && !test->client_role) {
		PRINT_INFO("no role specified; use receiver role");
		test->server_role = true;
	}

	if (test->server_role) {
		if (test->hist) {
			PRINT_ERR("histogram report is not supported in receiver side; ignored.");
		}
	}
	if (test->server_role) {
		if (test->perc) {
			PRINT_ERR("percentile report is not supported on receiver side; ignored.");
		}
	}

	if (test->server_role) {
		if (test->raw_dump)
			PRINT_ERR("dumping latencies into a file not supported on receiver side; ignored");
	}

	if (test->client_role) {
		if (0 == strcmp(test->bind_address, "0.0.0.0")) {
			// This is needed due to behaviour if Win socket in client mode
			test->bind_address = "127.0.0.1";
		}
		if (test->client_port > 0 && test->client_port <= 1024) {
			test->client_port = DEFAULT_SRC_PORT;
			PRINT_ERR("source port is too small. use the default value");
		}
		if (test->hist_start < 0) {
			PRINT_ERR("histogram interval start value provided is invalid; use default value.");
			test->hist_start = HIST_DEFAULT_START_AT;
		}
		if (test->hist_count > HIST_MAX_INTERVAL_COUNT_USER) {
			PRINT_ERR("count of histogram intervals is too big; use the max allowed value.");
			test->hist_start = HIST_MAX_INTERVAL_COUNT_USER;
		}
		if (test->hist_count < 1) {
			PRINT_ERR("count of histogram intervals is too small; use the default value.");
			test->hist_start = HIST_DEFAULT_INTERVAL_COUNT;
		}
	}

	/* Interop with latte.exe:
	 * latte needs minimum 4 bytes of data */
	if (test->msg_size < 4) {
		PRINT_INFO("invalid message size. use default value.");
		test->msg_size = DEFAULT_MESSAGE_SIZE_BYTES;
	}

	if (test->test_mode == TIME_DURATION && test->duration < 1) {
		PRINT_INFO("invalid test duration; use default value.");
		test->duration  = DEFAULT_TEST_DURATION_SEC;
	}

	if (test->test_mode == PING_ITERATION && test->iteration < 1) {
		PRINT_INFO("invalid ping iteration; use default value.");
		test->iteration = DEFAULT_TEST_ITERATION;
	}

	if (test->domain == AF_INET6 && strcmp(test->bind_address, "0.0.0.0") == 0)
		test->bind_address = "::";

	return NO_ERR;
}

// This is a custom getopt common for both Windows & Linux
// This don't accept space between option and its value
char* optarg2 = NULL;
int getopt2(int argc, char *const argv[], const char *optstr)
{
	static int optind = 1;

	if ((optind >= argc) || (argv[optind][0] == 0))
		return -1;

	if (argv[optind][0] != '-')
		return '?';

	int opt = argv[optind][1];
	const char *p = strchr(optstr, opt);

	if (p == NULL)
		return '?';

	optarg2 = &argv[optind][2];

	if (p[1] == ':') {
		if (p[2] != ':') {
			if (optarg2[0] == 0)
				return '?';
		}
		if (p[2] == ':') {
			if (optarg2[0] == 0)
				optarg2 = NULL;
		}
	}

	optind++;
	return opt;
}

int parse_arguments(struct lagscope_test *test, int argc, char **argv)
{
	int flag;

	while ((flag = getopt2(argc, argv, "r::s::Df:6up:o:b:z:t:n:i:R::P::Ha:l:c:Vh")) != -1) {
		switch (flag) {
		case 'r':
			test->server_role = true;
			if (optarg2)
				test->bind_address = optarg2;
			break;

		case 's':
			test->client_role = true;
			if (optarg2)
				test->bind_address = optarg2;
			break;

		case 'D':
			test->daemon = true;
			break;

		case 'f':
			test->cpu_affinity = atoi(optarg2);
			break;

		case '6':
			test->domain = AF_INET6;
			break;

		case 'u':
			test->protocol = UDP;
			break;

		case 'p':
			test->server_port = atoi(optarg2);
			break;

		case 'o':
			test->client_port = atoi(optarg2);
			break;

		case 'b':
			test->recv_buf_size = atoi(optarg2);
			test->send_buf_size = atoi(optarg2);
			break;

		case 'z':
			test->msg_size = atoi(optarg2);
			break;

		case 't':
			test->duration = atoi(optarg2);
			test->test_mode = TIME_DURATION;
			break;

		case 'n':
			test->iteration = atoi(optarg2);
			break;

		case 'i':
			test->interval = atoi(optarg2);
			break;

		case 'H':
			test->hist = true;
			break;

		case 'a':
			test->hist_start = atoi(optarg2);
			break;

		case 'l':
			test->hist_len = atoi(optarg2);
			break;

		case 'c':
			test->hist_count = atoi(optarg2);
			break;

		case 'V':
			test->verbose = true;
			break;

		case 'P':
			test->perc = true;
			if(optarg2)
			{
				test->freq_table_dump = true;
				test->json_file_name = optarg2;
			}
			break;

		case 'R':
			test->raw_dump = true;
			if(optarg2)
				test->csv_file_name = optarg2;
			break;

		case 'h':
		default:
			print_usage();
			exit(ERROR_ARGS);
		}
	}
	return NO_ERR;
}

void print_iteration_histogram()
{
	PRINT_INFO("TBD");
}

void print_test_stats()
{
	PRINT_INFO("TBD");
}

char *retrive_ip_address_str(struct sockaddr_storage *ss, char *ip_str, size_t maxlen)
{
	switch(ss->ss_family) {
	case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)ss)->sin_addr), ip_str, maxlen);
		break;

	case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)ss)->sin6_addr), ip_str, maxlen);
		break;

	default:
		break;
	}
	return ip_str;
}
