#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "RUDPHost.h"
#include "RUDPProtocol.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

#define BUF_LEN 1024

int main()
{
	SOCKET serverSocket;

	sockaddr_in serverAddr, otherAddr;
	int slen, recv_len;
	char buf[BUF_LEN] = { 0 };
	char addressBuffer[64] = { 0 };
	WSADATA wsa;

	slen = sizeof(otherAddr);

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed initializing WinSock... " << WSAGetLastError() << std::endl;
		return 1;
	}

	RUDPHost* host = new RUDPHost(true, 7777);

	while (true)
	{
		host->HostService(1000);
	}

	//closesocket(serverSocket);
	WSACleanup();

	return 0;
}