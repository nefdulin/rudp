#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

class RUDPPacket
{
public:
	RUDPPacket(const void* data, size_t dataLength, size_t flags)
	{
		_data = new unsigned char[dataLength];
		memcpy(_data, data, dataLength);

		_dataLength = dataLength;

		Flags = flags;
	}

	const unsigned char* GetData() { return _data; }
	const size_t GetLength() { return _dataLength; }

	size_t Flags;
private:
	size_t _dataLength;
	unsigned char* _data;
};