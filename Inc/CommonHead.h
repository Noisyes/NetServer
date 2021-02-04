#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <assert.h>
#include <signal.h>
#include <iostream>

#define DEFAULT_PORT 8888
#define TIME_OUT 10
#define MAX_EVENT_NUMBER 1024