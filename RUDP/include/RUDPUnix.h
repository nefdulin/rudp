#pragma once

#ifndef _WIN32
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

struct RUDPBuffer
{
    void* data;
    size_t length;
};
#endif

