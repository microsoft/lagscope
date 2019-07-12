// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#ifndef TCPSTREAM_H
#define TCPSTREAM_H

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "const.h"

int n_read(int fd, char *buffer, size_t total);
int n_write(int fd, const char *buffer, size_t total);
//int set_socket_non_blocking(int fd);

#endif // TCPSTREAM_H
