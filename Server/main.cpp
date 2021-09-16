#include <iostream>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "RUDPHost.h"
#include "RUDPProtocol.h"

#define BUF_LEN 1024

int main()
{
	RUDPSocket serverSocket;
	RUDPAddress serverAddr, otherAddr;

    RUDP::Initialize();

	int slen, recv_len;
	char buf[BUF_LEN] = { 0 };
	char addressBuffer[64] = { 0 };

	slen = sizeof(otherAddr);

	RUDPHost* host = new RUDPHost(true, 7777);

	while (true)
	{
		host->HostService(1000);
	}

	//closesocket(serverSocket);
    RUDP::Deinitialize();

	return 0;
}