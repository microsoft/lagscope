// ----------------------------------------------------------------------------------
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// Author: Shihua (Simon) Xiao, sixiao@microsoft.com
// ----------------------------------------------------------------------------------

#include "main.h"

/************************************************************/
//		lagscope sender
/************************************************************/
long run_lagscope_sender(struct lagscope_test_client *client)
{
	char *log = 0;
	bool verbose_log = false;
	struct lagscope_test_runtime *test_runtime;
	int sockfd = 0; //socket id
	int sendbuff, recvbuff = 0;   //send buffer size
	char *buffer; //send buffer
	int msg_actual_size; //the buffer actual size = msg_size * sizeof(char)
	struct lagscope_test *test = client->test;
	int n = 0; //write n bytes to socket

	struct sockaddr_storage local_addr; //for local address
	socklen_t local_addr_size; //local address size, for getsockname(), to get local port
	char *ip_address_str; //used to get remote peer's ip address
	int ip_address_max_size;  //used to get remote peer's ip address
	char *port_str; //to get remote peer's port number for getaddrinfo()
	struct addrinfo hints, *serv_info, *p; //to get remote peer's sockaddr for connect()

	struct timeval now;
	struct timeval send_time;
	struct timeval recv_time;
	double latency = 0;
	int i = 0;

	/* for ping statistics */
	unsigned long n_pings = 0; //number of pings
	double max_latency = 0;
	double min_latency = 60000; //60 seconds
	double sum_latency = 0;

	int latencies_stats_err_check = 0;

	verbose_log = test->verbose;
	test_runtime = new_test_runtime(test);

	ip_address_max_size = (test->domain == AF_INET? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
	if ((ip_address_str = (char *)malloc(ip_address_max_size)) == (char *)NULL) {
		PRINT_ERR("cannot allocate memory for ip address string");
		freeaddrinfo(serv_info);
		return 0;
	}

	/* get address of remote receiver */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = test->domain;
	hints.ai_socktype = test->protocol;
	ASPRINTF(&port_str, "%d", test->server_port);
	if (getaddrinfo(test->bind_address, port_str, &hints, &serv_info) != 0) {
		PRINT_ERR("cannot get address info for receiver");
		free(ip_address_str);
		return 0;
	}
	free(port_str);

	/* only get the first entry of remote receiver to connect */
	for (p = serv_info; p != NULL; p = p->ai_next) {
		/* 1. create socket fd */
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			PRINT_ERR("cannot create socket ednpoint");
			freeaddrinfo(serv_info);
			free(ip_address_str);
			return 0;
		}
		sendbuff = test->send_buf_size;
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &sendbuff, sizeof(sendbuff)) < 0) {
			ASPRINTF(&log, "cannot set socket send buffer size to: %d", sendbuff);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			return 0;
		}
		recvbuff = test->recv_buf_size;
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvbuff, sizeof(recvbuff)) < 0) {
			ASPRINTF(&log, "cannot set socket receive buffer size to: %d", recvbuff);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			return 0;
		}
		local_addr_size = sizeof(local_addr);

		/*
		   2. bind this socket fd to a local random/ephemeral TCP port,
		      so that the sender side will have randomized TCP ports.
		*/
		if (test->domain == AF_INET) {
			(*(struct sockaddr_in*)&local_addr).sin_family = AF_INET;
			(*(struct sockaddr_in*)&local_addr).sin_port = 0;
		}
		else {
			(*(struct sockaddr_in6*)&local_addr).sin6_family = AF_INET6;
			(*(struct sockaddr_in6*)&local_addr).sin6_port = 0;
		}

		if ((i = bind(sockfd, (struct sockaddr *)&local_addr, local_addr_size)) < 0) {
			ASPRINTF(&log, "failed to bind socket: %d to a local ephemeral port. errno = %d", sockfd, errno);
			PRINT_ERR_FREE(log);
		}

		/* 3. connect to receiver */
		ip_address_str = retrive_ip_address_str((struct sockaddr_storage *)p->ai_addr, ip_address_str, ip_address_max_size);
		if ((i = connect(sockfd, p->ai_addr, p->ai_addrlen)) < 0) {
			if (i == -1) {
				ASPRINTF(&log, "failed to connect to receiver: %s:%d on socket: %d. errno = %d", ip_address_str, test->server_port, sockfd, errno);
				PRINT_ERR_FREE(log);
			}
			else {
				ASPRINTF(&log, "failed to connect to receiver: %s:%d on socket: %d. error code = %d", ip_address_str, test->server_port, sockfd, i);
				PRINT_ERR_FREE(log);
			}
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return 0;
		}
		else {
			break; //connected
		}
	}

	/* get local TCP ephemeral port number assigned, for logging */
	if (getsockname(sockfd, (struct sockaddr *) &local_addr, &local_addr_size) != 0) {
		ASPRINTF(&log, "failed to get local address information for socket: %d", sockfd);
		PRINT_ERR_FREE(log);
	}

	ASPRINTF(&log, "New connection: local:%d [socket:%d] --> %s:%d",
			ntohs(test->domain == AF_INET?
					((struct sockaddr_in *)&local_addr)->sin_port:
					((struct sockaddr_in6 *)&local_addr)->sin6_port),
			sockfd,	ip_address_str, test->server_port);
	PRINT_INFO_FREE(log);

	freeaddrinfo(serv_info);

	msg_actual_size = test->msg_size * sizeof(char);
	if ((buffer = (char *)malloc(msg_actual_size)) == (char *)NULL) {
		PRINT_ERR("cannot allocate memory for send message");
		close(sockfd);
		return 0;
	}
	memset(buffer, 'A', msg_actual_size);

	//begin ping test
	turn_on_light();
	if (test->test_mode == TIME_DURATION)
		run_test_timer(test->duration);

	gettimeofday(&now, NULL);
	test_runtime->start_time = now;

	while (is_light_turned_on()) {
		gettimeofday(&now, NULL);
		send_time = now;
		if ((n = n_write(sockfd, buffer, msg_actual_size)) != msg_actual_size) {
			if (n < 0) {
				PRINT_ERR("socket error. cannot write data to a socket");
			}
			else {
				PRINT_ERR("failed to send all bytes");
			}

			goto finished;
		}
		if ((n = n_read(sockfd, buffer, msg_actual_size)) != msg_actual_size) {
			PRINT_ERR("failed to receive bytes from server");
			goto finished;
		}

		gettimeofday(&now, NULL);
		recv_time = now;
		latency = get_time_diff(&recv_time, &send_time) * 1000 * 1000;

		push(latency);		// Push latency onto linked list

		ASPRINTF(&log, "Reply from %s: bytes=%d time=%.3fus",
				ip_address_str,
				n,
				latency);
		PRINT_DBG_FREE(log);

		n_pings++;
		test_runtime->current_time = recv_time;
		test_runtime->ping_elapsed = n_pings;

		/* calculate max. avg. min. */
		sum_latency += latency;
		if (max_latency < latency)
			max_latency = latency;
		if (min_latency > latency)
			min_latency = latency;

		if (test->test_mode == PING_ITERATION)
			if (n_pings >= test->iteration)
				break;

		if (verbose_log == false)
			report_progress(test_runtime);

		if (test->interval !=0)
			sleep(test->interval); //sleep for ping interval, for example, 1 second
	}
	//sleep(60);
finished:
	PRINT_INFO("TEST COMPLETED.");

	/* print ping statistics */
	ASPRINTF(&log, "Ping statistics for %s:", ip_address_str);
	PRINT_INFO_FREE(log);
	ASPRINTF(&log, "\tNumber of successful Pings: %ld", n_pings);
	PRINT_INFO_FREE(log);
	if (n_pings > 0) {
		ASPRINTF(&log, "\tMinimum = %.3fus, Maximum = %.3fus, Average = %.3fus",
			min_latency,
			max_latency,
			sum_latency/n_pings);
		PRINT_INFO_FREE(log);
	}

	/* function call to dump latencies into a csv file */
	if(test->raw_dump)
	{
		ASPRINTF(&log, "Dumping all latencies into %s", test->csv_file_name);
		PRINT_INFO_FREE(log);
		create_latencies_csv(test->csv_file_name);
	}

	/* function call to show percentiles */
	if(test->perc)
	{
		latencies_stats_err_check = show_percentile(max_latency, n_pings);
		if(latencies_stats_err_check == ERROR_MEMORY_ALLOC)
		{
			PRINT_ERR("Memory allocation failed, aborting...");
		}
		else if(latencies_stats_err_check == ERROR_GENERAL)
		{
			PRINT_ERR("Interanl Error, aborting...");
		}
	}

	/* function call to show histogram */
	if(test->hist)
	{
		latencies_stats_err_check = show_histogram(test->hist_start, test->hist_len, test->hist_count, (unsigned long) max_latency);
		if(latencies_stats_err_check == ERROR_MEMORY_ALLOC)
		{
			PRINT_ERR("Memory allocation failed, aborting...");
		}
		else if(latencies_stats_err_check == ERROR_GENERAL)
		{
			PRINT_ERR("Interanl Error, aborting...");
		}
	}

	/* free resource */
	free(ip_address_str);
	free(buffer);
	latencies_stats_cleanup();
	close(sockfd);
	return n_pings;
}

/************************************************************/
//		lagscope receiver
/************************************************************/
/* listen on the port specified by ss, and return the socket fd */
int lagscope_server_listen(struct lagscope_test_server *server)
{
	char *log;
	struct lagscope_test *test = server->test;
	bool verbose_log = test->verbose;
	int opt = 1;
	int sockfd = 0; //socket file descriptor
	int sendbuff, recvbuff = 0; //receive buffer size
	char *ip_address_str; //used to get local ip address
	int ip_address_max_size;  //used to get local ip address
	char *port_str; //to get remote peer's port number for getaddrinfo()
	struct addrinfo hints, *serv_info, *p; //to get remote peer's sockaddr for bind()

	int i = 0; //just for debug purpose

	/* get receiver/itself address */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = test->domain;
	hints.ai_socktype = test->protocol;
	ASPRINTF(&port_str, "%d", test->server_port);
	if (getaddrinfo(test->bind_address, port_str, &hints, &serv_info) != 0) {
		PRINT_ERR("cannot get address info for receiver");
		return -1;
	}
	free(port_str);

	ip_address_max_size = (test->domain == AF_INET? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
	if ((ip_address_str = (char *)malloc(ip_address_max_size)) == (char *)NULL) {
		PRINT_ERR("cannot allocate memory for ip address string");
		freeaddrinfo(serv_info);
		return -1;
	}

	/* get the first entry to bind and listen */
	for (p = serv_info; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			PRINT_ERR("cannot create socket ednpoint");
			freeaddrinfo(serv_info);
			free(ip_address_str);
			return -1;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
			ASPRINTF(&log, "cannot set socket options SO_REUSEADDR: %d", sockfd);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return -1;
		}

		opt = 0;
		if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, (char *) &opt, sizeof(opt)) < 0) {
			ASPRINTF(&log, "cannot set socket options TCP_QUICKACK: %d", sockfd);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return -1;
		}

		sendbuff = test->send_buf_size;
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &sendbuff, sizeof(sendbuff)) < 0) {
			ASPRINTF(&log, "cannot set socket send buffer size to: %d", sendbuff);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return -1;
		}
		recvbuff = test->recv_buf_size;
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvbuff, sizeof(recvbuff)) < 0) {
			ASPRINTF(&log, "cannot set socket receive buffer size to: %d", recvbuff);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return -1;
		}
/*		if (set_socket_non_blocking(sockfd) == -1) {
			ASPRINTF(&log, "cannot set socket as non-blocking: %d", sockfd);
			PRINT_ERR_FREE(log);
			freeaddrinfo(serv_info);
			free(ip_address_str);
			close(sockfd);
			return -1;
		}
*/
		if ((i = bind(sockfd, p->ai_addr, p->ai_addrlen)) < 0) {
			ASPRINTF(&log, "failed to bind the socket to local address: %s on socket: %d. errcode = %d",
			ip_address_str = retrive_ip_address_str((struct sockaddr_storage *)p->ai_addr, ip_address_str, ip_address_max_size), sockfd, i);

			if (i == -1)
				ASPRINTF(&log, "%s. errcode = %d", log, errno);
			PRINT_DBG_FREE(log);
			continue;
		}
		else {
			break; //connected
		}
	}
	freeaddrinfo(serv_info);
	free(ip_address_str);
	if (p == NULL) {
		ASPRINTF(&log, "cannot bind the socket on address: %s", test->bind_address);
		PRINT_ERR_FREE(log);
		close(sockfd);
		return -1;
	}

	server->listener = sockfd;
	if (listen(server->listener, MAX_CONNECTIONS_PER_THREAD) < 0) {
		ASPRINTF(&log, "failed to listen on address: %s: %d", test->bind_address, test->server_port);
		PRINT_ERR_FREE(log);
		close(server->listener);
		return -1;
	}

	FD_ZERO(&server->read_set);
	FD_ZERO(&server->write_set);
	FD_SET(server->listener, &server->read_set);
	if (server->listener > server->max_fd)
		server->max_fd = server->listener;

	ASPRINTF(&log, "lagscope server is listening on %s:%d", test->bind_address, test->server_port);
	PRINT_DBG_FREE(log);

	return server->listener;
}

int lagscope_server_select(struct lagscope_test_server *server)
{
	int err_code = NO_ERROR;
	int opt = 0;
	char *log = NULL;
	struct lagscope_test *test = server->test;
	bool verbose_log = test->verbose;

	int n_fds = 0, newfd, current_fd = 0;
	char *buffer; //receive buffer
	int msg_actual_size; //the buffer actual size = msg_size * sizeof(char)
	long nbytes; //bytes read
	fd_set read_set, write_set;

	struct sockaddr_storage peer_addr, local_addr; //for remote peer, and local address
	socklen_t peer_addr_size, local_addr_size;
	char *ip_address_str;
	int ip_address_max_size;

	msg_actual_size = test->msg_size * sizeof(char);
	if ((buffer = (char *)malloc(msg_actual_size)) == (char *)NULL) {
		PRINT_ERR("cannot allocate memory for receive message");
		return ERROR_MEMORY_ALLOC;
	}
	ip_address_max_size = (test->domain == AF_INET? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
	if ((ip_address_str = (char *)malloc(ip_address_max_size)) == (char *)NULL) {
		PRINT_ERR("cannot allocate memory for ip address of peer");
		free(buffer);
		return ERROR_MEMORY_ALLOC;
	}

	/* accept new client, receive data from client */
	while (1) {
		memcpy(&read_set, &server->read_set, sizeof(fd_set));
		memcpy(&write_set, &server->write_set, sizeof(fd_set));

		/* we are notified by select() */
		n_fds = select(server->max_fd + 1, &read_set, NULL, NULL, NULL);
		if (n_fds < 0 && errno != EINTR) {
			PRINT_ERR("error happened when select()");
			err_code = ERROR_SELECT;
			continue;
		}

		/*run through the existing connections looking for data to be read*/
		for (current_fd = 0; current_fd <= server->max_fd; current_fd++) {
			if (!FD_ISSET(current_fd, &read_set))
				continue;

			/* then, we got one fd to handle */
			/* a NEW connection coming */

			/* need to reset TCP_QUICKACK every time */
			setsockopt(current_fd, IPPROTO_TCP, TCP_QUICKACK, (char *) &opt, sizeof(opt));
			if (current_fd == server->listener) {
 				/* handle new connections */
				peer_addr_size = sizeof(peer_addr);
				if ((newfd = accept(server->listener, (struct sockaddr *) &peer_addr, &peer_addr_size)) < 0) {
					err_code = ERROR_ACCEPT;
					break;
				}

				/* then we got a new connection */
/*				if (set_socket_non_blocking(newfd) == -1) {
					ASPRINTF(&log, "cannot set the new socket as non-blocking: %d", newfd);
					PRINT_DBG_FREE(log);
				}
*/
				FD_SET(newfd, &server->read_set); /* add the new one to read_set */
				if (newfd > server->max_fd) {
					/* update the maximum */
					server->max_fd = newfd;
				}

				/* print out new connection info */
				local_addr_size = sizeof(local_addr);
				if (getsockname(newfd, (struct sockaddr *) &local_addr, &local_addr_size) != 0) {
					ASPRINTF(&log, "failed to get local address information for the new socket: %d", newfd);
					PRINT_DBG_FREE(log);
				}
				else {
					ASPRINTF(&log, "New connection: %s:%d --> local:%d [socket %d]",
							ip_address_str = retrive_ip_address_str(&peer_addr, ip_address_str, ip_address_max_size),
							ntohs(test->domain == AF_INET ?
									((struct sockaddr_in *)&peer_addr)->sin_port
									:((struct sockaddr_in6 *)&peer_addr)->sin6_port),
							ntohs(test->domain == AF_INET ?
									((struct sockaddr_in *)&local_addr)->sin_port
									:((struct sockaddr_in6 *)&local_addr)->sin6_port),
							newfd);
					PRINT_INFO_FREE(log);
				}
				turn_on_light();
			}
			/* handle data from an EXISTING client */
			else {
				bzero(buffer, msg_actual_size);

				/* got error or connection closed by client */
				if ((nbytes = n_read(current_fd, buffer, msg_actual_size)) <= 0) {
					if (nbytes == 0) {
						ASPRINTF(&log, "socket closed: %d", current_fd);
						PRINT_DBG_FREE(log);
					}
					else {
						ASPRINTF(&log, "error: cannot read data from socket: %d", current_fd);
						PRINT_INFO_FREE(log);
						err_code = ERROR_NETWORK_READ;
						/* need to continue test and check other socket, so don't end the test */
					}
					close(current_fd);
					FD_CLR(current_fd, &server->read_set); /* remove from master set when finished */
				}
				/* report ping request eceived */
				else {
					if ((nbytes = n_write(current_fd, buffer, msg_actual_size)) != msg_actual_size) {
						ASPRINTF(&log, "error: cannot write echo data to client from socket: %d", current_fd);
						PRINT_ERR_FREE(log);
					}
					ASPRINTF(&log, "Received from %s: bytes=%ld",
							ip_address_str = retrive_ip_address_str(&peer_addr, ip_address_str, ip_address_max_size),
							nbytes);
					PRINT_DBG_FREE(log);
				}
			}
		}
	}

	free(buffer);
	free(ip_address_str);
	close(server->listener);
	return err_code;
}

long run_lagscope_receiver(struct lagscope_test_server *server)
{
	char *log = NULL;
	//long n_pings = 0; //number of pings received

	server->listener = lagscope_server_listen(server);
	if (server->listener < 0) {
		ASPRINTF(&log, "listen error at port: %d", server->test->server_port);
		PRINT_ERR_FREE(log);
	}
	else {
		if (lagscope_server_select(server) != NO_ERROR) {
			ASPRINTF(&log, "select error at port: %d", server->test->server_port);
			PRINT_ERR_FREE(log);
		}
	}

	return 0; //don't return anything on server side
}

int main(int argc, char **argv)
{
	int err_code = NO_ERROR;
	cpu_set_t cpuset;
	struct lagscope_test *test;
	struct lagscope_test_server *server;
	struct lagscope_test_client *client;

	/* catch SIGINT: Ctrl + C */
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		PRINT_ERR("main: error when setting the disposition of the signal SIGINT");

	print_version();
	test = new_lagscope_test();
	if (!test) {
		PRINT_ERR("main: error when creating new test");
		exit (-1);
	}

	default_lagscope_test(test);
	err_code = parse_arguments(test, argc, argv);
	if (err_code != NO_ERROR) {
		PRINT_ERR("main: error when parsing args");
		print_flags(test);
		free(test);
		exit (-1);
	}

	err_code = verify_args(test);
	if (err_code != NO_ERROR) {
		PRINT_ERR("main: error when verifying the args");
		print_flags(test);
		free(test);
		exit (-1);
	}

	if (test->verbose)
		print_flags(test);

	turn_off_light();

	if (test->cpu_affinity != -1) {
		CPU_ZERO(&cpuset);
		CPU_SET(test->cpu_affinity, &cpuset);
		PRINT_INFO("main: set cpu affinity");
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
			PRINT_ERR("main: cannot set cpu affinity");
	}

	if (test->daemon) {
		PRINT_INFO("main: run this tool in the background");
		if (daemon(0, 0) != 0)
			PRINT_ERR("main: cannot run this tool in the background");
	}

	if (test->client_role == true) {
		client = new_lagscope_client(test);
		err_code = run_lagscope_sender(client);
		free(client);
	}
	else {
		server = new_lagscope_server(test);
		err_code = run_lagscope_receiver(server);
		free(server);
	}

	return err_code;
}
