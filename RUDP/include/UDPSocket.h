#pragma once
#include <iostream>
#include "RUDPProtocol.h"
#include "RUDPHost.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

class UDPSocket
{
public:
	static int SocketSend(SOCKET socket, sockaddr_in addr, const RUDPBuffer* buffers, size_t bufferCount);
	static int SocketRecv(SOCKET socket, RUDPBuffer* buffers, size_t bufferCount, sockaddr_in& outAddress);
};

