
#ifndef COMMON_H
#define COMMON_H

// C standard library header files
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <inttypes.h>
#include <math.h>

// Linux specific header files
#ifndef _WIN32
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#endif

#ifdef _WIN32
#define INIT_SOCKFD_VAR() WSADATA wsaData; SOCKET sockfd = INVALID_SOCKET; WSAStartup(MAKEWORD(2,2), &wsaData)
#define CLOSE(s) closesocket(s)
#define WSACLEAN() WSACleanup()
#define SLEEP(t) Sleep(t*1000)
#else
#define INIT_SOCKFD_VAR() int sockfd = 0
#define CLOSE(s) close(s)
#define WSACLEAN() (void)0
#define SLEEP(t) sleep(t)
#endif

// Windows specific
#ifdef _WIN32

#undef UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// Todo: Fix these warnings instead of suppress
#pragma warning(disable : 4244) // Possible data loss due to conversion
#pragma warning(disable : 4996) // Unsecure function in use

#endif // _WIN32

#endif // COMMON_H
