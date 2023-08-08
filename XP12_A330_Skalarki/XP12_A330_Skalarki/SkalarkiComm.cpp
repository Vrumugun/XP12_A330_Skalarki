#include "pch.h"
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"

#include "SkalarkiComm.h"

using namespace std::chrono_literals;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "53000"
#define SERVER_PORT (50001)

SkalarkiComm::SkalarkiComm() : _semaphore(0)
{
    _socket = INVALID_SOCKET;
    _processPacketFxn = nullptr;
}

SkalarkiComm::~SkalarkiComm()
{
	if (_socket != INVALID_SOCKET)
	{
        Shutdown();
	}
}

int SkalarkiComm::Init(const char* first_buffer, int first_buffer_len, T_ProcessPacketFxn process_packet_fxn)
{
    WSADATA wsaData;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        char buffer[100];
        sprintf_s(buffer, "A330_Skalarki: WSAStartup failed with error: %d\n", iResult);
        XPLMDebugString(buffer);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        char buffer[100];
        sprintf_s(buffer, "A330_Skalarki: getaddrinfo failed with error: %d\n", iResult);
        XPLMDebugString(buffer);
        WSACleanup();
        return 1;
    }

    ptr = result;

    // Create a SOCKET for connecting to server
    _socket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);
    if (_socket == INVALID_SOCKET)
    {
        char buffer[100];
        sprintf_s(buffer, "A330_Skalarki: socket failed with error: %ld\n", WSAGetLastError());
        XPLMDebugString(buffer);
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect(_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    if (_socket == INVALID_SOCKET)
    {
        char buffer[100];
        sprintf_s(buffer, "A330_Skalarki: Unable to connect to server!\n");
        XPLMDebugString(buffer);
        WSACleanup();
        return 1;
    }

    iResult = send(_socket, first_buffer, first_buffer_len, 0);
    if (iResult != SOCKET_ERROR)
    {

    }

    std::this_thread::sleep_for(1000ms);

    _processPacketFxn = process_packet_fxn;

    _transmitThread = std::thread(&SkalarkiComm::TransmitThread, this);
    _receiveThread = std::thread(&SkalarkiComm::ReceiveThread, this);

    return 0;
}

int SkalarkiComm::Shutdown()
{
    XPLMDebugString("A330_Skalarki: ShutdownCommunication...\n");

    XPLMDebugString("A330_Skalarki: Stop threads\n");

    _stopTransmit.store(true);
    _stopReceive.store(true);

    _semaphore.release();

    XPLMDebugString("A330_Skalarki: Wait for threads\n");
    _transmitThread.join();

    XPLMDebugString("A330_Skalarki: Shutdown socket\n");

    // shutdown the connection since no more data will be sent
    int iResult = shutdown(_socket, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(_socket);
        WSACleanup();
        return 1;
    }

    _receiveThread.join();

    // cleanup
    closesocket(_socket);
    WSACleanup();

    XPLMDebugString("A330_Skalarki: Finish shutdown\n");

    return 0;
}

void SkalarkiComm::PushMessage(const char* msg, int len)
{
    if (len > SKALARKI_MESSAGE_MAX_LENGTH)
    {
        XPLMDebugString("A330_Skalarki: : Invalid length, > 256\n");
        return;
    }

    if (len == 0)
    {
        XPLMDebugString("A330_Skalarki: : Invalid length, = 0\n");
        return;
    }

    std::lock_guard<std::mutex> guard(_mutex);

    _messageQueue.push(SkalarkiMessage(msg, len));

    _semaphore.release();
}

void SkalarkiComm::TransmitThread()
{
    while (_stopTransmit.load() == false)
    {
        _semaphore.acquire();
        std::lock_guard<std::mutex> guard(_mutex);

        if (_messageQueue.empty() == false)
        {
            SkalarkiMessage msg = _messageQueue.front();
            _messageQueue.pop();

            int iResult = send(_socket, msg._msg, msg._len, 0);
            if (iResult == SOCKET_ERROR)
            {
                int lastError = WSAGetLastError();
                char buffer[100];
                sprintf_s(buffer, "A330_Skalarki: Send socket failed (%d)\n", lastError);
                XPLMDebugString(buffer);
            }
        }

        std::this_thread::sleep_for(10ms);
    }
}

void SkalarkiComm::ReceiveThread()
{
    while (_stopReceive.load() == false)
    {
        static char recvbuf[DEFAULT_BUFLEN];
        static int recvbuflen = DEFAULT_BUFLEN;

        int iResult = recv(_socket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            //printf("Bytes received: %d\n", iResult);

            if (_processPacketFxn != nullptr)
            {
                (*_processPacketFxn)(recvbuf);
            }
        }
    }
}
