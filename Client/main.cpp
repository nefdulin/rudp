#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <RUDPHost.h>
#include <RUDPPeer.h>
#include <csignal>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

#define BUF_LEN 1024

int main()
{
	sockaddr_in otherAddr;
	int s, slen = sizeof(otherAddr);
	char buf[BUF_LEN] = { 0 };
	char message[512] = { 0 };
	char addressBuffer[64] = { 0 };
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed initializing " << WSAGetLastError() << std::endl;
		return 1;
	}

	RUDPHost* host = new RUDPHost(0, 0);

	host->Connect("127.0.0.1", 7777);

	while (1)
	{
		host->HostService(1000);

		int x;
		while (std::cin >> x)
		{
			if (x == 0)
			{
				RUDPPacket* packet = new RUDPPacket("ab", 2, RUDP_PACKET_FLAG_RELIABLE);
				host->SendPacket(packet);
				break;
			}
		}
	}

	//while (1)
	//{
		//std::cout << "Enter size of data " << std::endl;
		//int y;
		//std::cin >> y;

		//std::cout << "Enter the data: " << std::endl;
		//char* x = new char[y];
		//std::cin >> x;

		//host->SendPacket(nullptr);
	//}

	while (1);

	//while (1)
	//{
	//	std::cout << "Message: ";
	//	std::cin >> message;

	//	std::cout << "Message length: " << sizeof(message) << std::endl;

	//	if (sendto(s, message, strlen(message), 0, (sockaddr*)&otherAddr, slen) == SOCKET_ERROR)
	//	{
	//		std::cout << "sendto failed wih error code " << WSAGetLastError();
	//		return 1;
	//	}

	//	ZeroMemory(buf, BUF_LEN);

	//	int bytesReceived = 0;

	//	bytesReceived = recvfrom(s, buf, BUF_LEN, 0, (sockaddr*)&otherAddr, &slen);

	//	std::cout << "Received packet from: " << inet_ntop(AF_INET, &otherAddr, addressBuffer, 64) << " " << ntohs(otherAddr.sin_port) << " with size: " << bytesReceived << std::endl;
	//	std::cout << "message: " << buf << std::endl;
	//}

	//closesocket(s);
	WSACleanup();
	return 0;
}