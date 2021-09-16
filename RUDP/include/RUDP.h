#pragma once
#ifdef _WIN32
#include <RUDPWin32.h>
#else
#include "RUDPUnix.h"
#endif

typedef int32_t RUDPSocket;

struct RUDPAddress
{
    uint32_t host;
    uint16_t port;
};

class RUDP
{
public:
    static int Initialize();
    static int Deinitialize();

    static uint32_t GetTime();

    static int AddressSetHostIp(RUDPAddress *address, const char *ip);

    static int SocketCreate();
    static int SocketSend(RUDPSocket socket, RUDPAddress* address, const RUDPBuffer* buffers, size_t bufferCount);
    static int SocketRecv(RUDPSocket socket, RUDPBuffer* buffers, size_t bufferCount, RUDPAddress* outAddress);
    static int SetSocketNonBlock(const RUDPSocket& socket);
};



