// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#define TOOL_NAME "lagscope"
#define TOOL_VERSION "0.1.2"
#define AUTHOR_NAME "Shihua (Simon) Xiao, sixiao@microsoft.com"

#define TCP 				SOCK_STREAM
#define UDP 				SOCK_DGRAM

#define DEFAULT_CSV_FILENAME          "latencies_log.csv"
#define DEFAULT_RTT_CSV_FILENAME      "tcprtt_latencies_log.csv"

#define TIME_DURATION			1
#define PING_ITERATION			2

#define ROLE_SENDER			1
#define ROLE_RECEIVER			2

#define TEST_UNKNOWN			10
#define TEST_NOT_STARTED		11
#define TEST_RUNNING			12
#define TEST_FINISHED			13
#define TEST_INTERRUPTED		14


/* default values */
#define MAX_CONNECTIONS_PER_THREAD	512
#define DEFAULT_SERVER_PORT		6001
#define DEFAULT_RECV_BUFFER_SIZE_BYTES	64 * 1024
#define DEFAULT_SEND_BUFFER_SIZE_BYTES	128 * 1024
#define DEFAULT_MESSAGE_SIZE_BYTES	4
#define DEFAULT_TEST_DURATION_SEC	60
#define DEFAULT_TEST_ITERATION		1000000
#define DEFAULT_TEST_INTERVAL_SEC	0

#define NO_ERROR			0
#define ERROR_GENERAL			-1000
#define ERROR_ARGS			-1001
#define ERROR_MEMORY_ALLOC		-1002
#define ERROR_LISTEN			-1103
#define ERROR_ACCEPT			-1104
#define ERROR_SELECT			-1105
#define ERROR_NETWORK_READ		-1106
#define ERROR_NETWORK_WRITE		-1107
#define ERROR_RECEIVER_NOT_READY	-1108

/* consts for reporting */
#define HIST_DEFAULT_START_AT		0
#define HIST_DEFAULT_INTERVAL_LEN	100
#define HIST_DEFAULT_INTERVAL_COUNT	10
#define HIST_MAX_INTERVAL_COUNT		100
#define HIST_MAX_INTERVAL_COUNT_USER	100 - 2
