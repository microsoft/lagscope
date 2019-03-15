// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#include "util.h"

void print_flags(struct lagscope_test *test)
{
	if (test->server_role)
 		printf("%s\n", "*** receiver role");
	if (test->client_role)
		printf("%s\n", "*** sender role");
	if (test->daemon)
		printf("%s\n", "*** run as daemon");

	if (test->cpu_affinity == -1)
		printf("%s:\t\t\t %s\n", "cpu affinity", "*");
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
	printf("%s:\t %.2f\n", "socket receive buffer (bytes)", test->recv_buf_size);
	printf("%s:\t %.2f\n", "socket send buffer (bytes)", test->send_buf_size);
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
	printf("lagscope: [-r|-s|-D|-f|-6|-u|-p|-b|-B|-z|-t|-n|-i|-R|-P|-H|-a|-l|-c|-V|-h]\n\n");
	printf("\t-r   Run as a receiver\n");
	printf("\t-s   Run as a sender\n");
	printf("\t-D   Run as daemon\n");
	printf("\t-f   CPU affinity. *, or cpuid such as 0, 1, etc.\n");

	printf("\t-6   IPv6 mode    [default: IPv4]\n");
	//printf("\t-u   UDP mode    [default: TCP] NOT SUPPORTED YET\n");
	printf("\t-p   Port number, or starting port number    [default: %d]\n", DEFAULT_SERVER_PORT);
	printf("\t-b   <recv buffer size>    [default: %d bytes]\n", DEFAULT_RECV_BUFFER_SIZE_BYTES);
	printf("\t-B   <send buffer size>    [default: %d bytes]\n", DEFAULT_SEND_BUFFER_SIZE_BYTES);
	printf("\t-z   <message size>        [default: %d bytes]\n", DEFAULT_MESSAGE_SIZE_BYTES);

	printf("\t-t   [SENDER ONLY] test duration       [default: %d second(s)]\n", DEFAULT_TEST_DURATION_SEC);
	printf("\t-n   [SENDER ONLY] ping iteration      [default: %d]\n", DEFAULT_TEST_ITERATION);
	printf("\t-i   [SENDER ONLY] test interval       [default: %d second(s)]\n", DEFAULT_TEST_INTERVAL_SEC);
	printf("\t     '-n' will be ignored if '-t' provided\n");

	printf("\t-R   [SENDER ONLY] dumps latencies into csv file\n");

	printf("\t-H   [SENDER ONLY] print histogram of per-iteration latency values\n");
	printf("\t-a   [SENDER ONLY] histogram 1st interval start value	[default: %d]\n", HIST_DEFAULT_START_AT);
	printf("\t-l   [SENDER ONLY] length of histogram intervals	[default: %d]\n", HIST_DEFAULT_INTERVAL_LEN);
	printf("\t-c   [SENDER ONLY] count of histogram intervals\t	[default: %d] [max: %d]\n", HIST_DEFAULT_INTERVAL_COUNT, HIST_MAX_INTERVAL_COUNT_USER);

	printf("\t-P   [SENDER ONLY] prints 50th, 75th, 90th, 99th, 99.9th, 99.99th, 99.999th percentile of latencies\n");

	printf("\t-V   Verbose mode\n");
	printf("\t-h   Help, tool usage\n");

	printf("Example:\n");
	printf("\treceiver:\n");
	printf("\t1) ./lagscope -r\n");
	printf("\t2) ./lagscope -r192.168.1.1\n");
	printf("\t3) ./late -r -D -f0 -6 -p 6789 -V\n");
	printf("\tsender:\n");
	printf("\t1) ./lagscope -s192.168.1.1\n");
	printf("\t2) ./lagscope -s192.168.1.1 -t 600 -i 1 -V\n");
	printf("\t3) ./lagscope -sfd00::1:1 -n 1000 -6 -i 2 -V\n");
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
		if (test->raw_dump) {
			PRINT_ERR("dumping latencies into a file not supported on receiver side; ignored");
		}
	}

	if (test->client_role) {
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

	if (test->msg_size < 1) {
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

	return NO_ERROR;
}

int parse_arguments(struct lagscope_test *test, int argc, char **argv)
{
	static struct option longopts[] =
	{
		{"receiver", optional_argument, NULL, 'r'},
		{"sender", optional_argument, NULL, 's'},
		{"daemon", no_argument, NULL, 'D'},
		{"cpu-affinity", required_argument, NULL, 'f'},
		{"ipv6", no_argument, NULL, '6'},
		{"udp", no_argument, NULL, 'u'},
		{"server-port", required_argument, NULL, 'p'},
		{"receiver-buffer", required_argument, NULL, 'b'},
		{"send-buffer", required_argument, NULL, 'B'},
		{"message_size", required_argument, NULL, 'z'},
		{"duration", required_argument, NULL, 't'},
		{"iteration", required_argument, NULL, 'n'},
		{"interval", required_argument, NULL, 'i'},
		{"hist", no_argument, NULL, 'H'},
		{"hist-start", required_argument, NULL, 'a'},
		{"hist-len", required_argument, NULL, 'l'},
		{"hist-count", required_argument, NULL, 'c'},
		{"perc", no_argument, NULL, 'P'},
		{"raw_dump", optional_argument, NULL, 'R'},
		{"verbose", no_argument, NULL, 'V'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	int flag;

	while ((flag = getopt_long(argc, argv, "r::s::Df:6up:b:B:z:t:n:i:R::PHa:l:c:Vh", longopts, NULL)) != -1) {
		switch (flag) {
		case 'r':
			test->server_role = true;
			if (optarg)
				test->bind_address = optarg;
			break;

		case 's':
			test->client_role = true;
			if (optarg)
				test->bind_address = optarg;
			break;

		case 'D':
			test->daemon = true;
			break;

		case 'f':
			test->cpu_affinity = atoi(optarg);
			break;

		case '6':
			test->domain = AF_INET6;
			break;

		case 'u':
			test->protocol = UDP;
			break;

		case 'p':
			test->server_port = atoi(optarg);
			break;

		case 'b':
			test->recv_buf_size = unit_atod(optarg);
			break;

		case 'B':
			test->send_buf_size = unit_atod(optarg);
			break;

		case 'z':
			test->msg_size = atoi(optarg);
			break;

		case 't':
			test->duration = atoi(optarg);
			test->test_mode = TIME_DURATION;
			break;

		case 'n':
			test->iteration = atoi(optarg);
			break;

		case 'i':
			test->interval = atoi(optarg);
			break;

		case 'H':
			test->hist = true;
			break;

		case 'a':
			test->hist_start = atoi(optarg);
			break;

		case 'l':
			test->hist_len = atoi(optarg);
			break;

		case 'c':
			test->hist_count = atoi(optarg);
			break;

		case 'V':
			test->verbose = true;
			break;

		case 'P':
			test->perc = true;
			break;

		case 'R':
			test->raw_dump = true;
			if(optarg)
				test->csv_file_name = optarg;
			break;

		case 'h':
		default:
			print_usage();
			exit(ERROR_ARGS);
		}
	}
	return NO_ERROR;
}

void print_iteration_histogram()
{
	PRINT_INFO("TBD");
}

void print_test_stats()
{
	PRINT_INFO("TBD");
}

const long KIBI = 1<<10;
const long MEBI = 1<<20;
const long GIBI = 1<<30;
double unit_atod(const char *s)
{
	double n;
	char suffix = '\0';

	sscanf(s, "%lf%c", &n, &suffix);
	switch (suffix) {
	case 'g': case 'G':
		n *= GIBI;
		break;
	case 'm': case 'M':
		n *= MEBI;
		break;
	case 'k': case 'K':
		n *= KIBI;
		break;
	default:
		break;
	}
	return n;
}

const char *unit_bps[] =
{
	"bps",
	"Kbps",
	"Mbps",
	"Gbps"
};

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