#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <queue>
#include <chrono>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

const int SKALARKI_MESSAGE_MAX_LENGTH = 256;

class SkalarkiMessage
{
public:
	SkalarkiMessage(const char* msg, int len)
	{
		memcpy_s(_msg, SKALARKI_MESSAGE_MAX_LENGTH, msg, len);
		_len = len;
	}
	char _msg[SKALARKI_MESSAGE_MAX_LENGTH];
	int _len;
};
typedef std::queue<SkalarkiMessage> SkalarkiMessageQueue;

typedef void (*T_ProcessPacketFxn)(char* recvbuf);

class SkalarkiComm
{
public:
	SkalarkiComm();
	~SkalarkiComm();

	int Init(const char* first_buffer, int first_buffer_len, T_ProcessPacketFxn process_packet_fxn);
	int Shutdown();

	void PushMessage(const char* msg, int len);

protected:
	void TransmitThread();
	void ReceiveThread();
	
	SOCKET _socket;

	std::thread _transmitThread;
	std::thread _receiveThread;
	
	std::atomic_bool _stopReceive{ false };
	std::atomic_bool _stopTransmit{ false };
	
	std::mutex _mutex;
	std::counting_semaphore<1000> _semaphore;

	T_ProcessPacketFxn _processPacketFxn;
	SkalarkiMessageQueue _messageQueue;
};
