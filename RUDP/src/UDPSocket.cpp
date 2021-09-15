#include "../include/UDPSocket.h"
#include <random>
#include <WinSock2.h>
#include <WS2tcpip.h>

#ifdef RUDP_DEBUG
	#define PACKET_LOSS_PERC 0
#endif

int UDPSocket::SocketSend(SOCKET socket, sockaddr_in addr, const RUDPBuffer* buffers, size_t bufferCount)
{
	DWORD sentLength;
	int recvAddrSize = sizeof(addr);

	// Packet Loss Debug
#ifdef RUDP_DEBUG
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist100(1, 100);
	int randomValue = dist100(rng);

	if (randomValue < PACKET_LOSS_PERC)
	{
		std::cout << "Failed sending packet" << std::endl;
		return 0;
	}
		
#endif

	if (WSASendTo(socket,
		(LPWSABUF)buffers,
		(DWORD)bufferCount,
		&sentLength,
		0,
		(sockaddr*)&addr,
		recvAddrSize,
		NULL,
		NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

		std::cout << "Send operation failed: " << WSAGetLastError() << std::endl;
		return -1;
	}

#ifdef RUDP_DEBUG
	if (sentLength > 0)
		std::cout << "Succesfully sent the packet length: " << sentLength << std::endl;
#endif
	return sentLength;
}

int UDPSocket::SocketRecv(SOCKET socket, RUDPBuffer* buffers, size_t bufferCount, sockaddr_in& outAddress)
{
	int addressLength = sizeof(outAddress);

	DWORD flags = 0;
	DWORD recvLength = 0;

	if (WSARecvFrom(socket,
		(LPWSABUF)buffers,
		(DWORD)bufferCount,
		&recvLength,
		&flags,
		(sockaddr*)&outAddress,
		&addressLength,
		NULL,
		NULL) == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
		case WSAECONNRESET:
			return 0;
		default:
			std::cout << "Recv failed with error: " << WSAGetLastError() << std::endl;
			break;
		}

		return -1;
	}

	return (int)recvLength;
}