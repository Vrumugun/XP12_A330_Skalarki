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

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "53000"
#define SERVER_PORT (50001)

static DWORD WINAPI ReceiveHandler(LPVOID lpParam);
static DWORD WINAPI TransmitHandler(LPVOID lpParam);
extern void ProcessPacket(char* recvbuf);

static SOCKET ConnectSocket = INVALID_SOCKET;

static char recvbuf[DEFAULT_BUFLEN];
static int recvbuflen = DEFAULT_BUFLEN;

static int MessageCount = 0;
static int ReadIdx = 0;
static int WriteIdx = 0;
static int SkalarkiMessageLength[128];
static char SkalarkiMessageBuffer[128][256];

static HANDLE ghSemaphore;
static HANDLE hMutex;
static HANDLE  hThreadReceive;
static HANDLE  hThreadTransmit;

int InitCommunication(const char* first_buffer, int first_buffer_len)
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
        printf("WSAStartup failed with error: %d\n", iResult);
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
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    iResult = send(ConnectSocket, first_buffer, first_buffer_len, 0);
    if (iResult != SOCKET_ERROR)
    {
    }

    Sleep(1000);

    ghSemaphore = CreateSemaphore(
        NULL,           // default security attributes
        0,  // initial count
        1000,  // maximum count
        NULL);          // unnamed semaphore

    if (ghSemaphore == NULL)
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    hMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    hThreadReceive = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        ReceiveHandler,       // thread function name
        NULL,          // argument to thread function 
        0,                      // use default creation flags 
        NULL);   // returns the thread identifier 

    hThreadTransmit = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        TransmitHandler,       // thread function name
        NULL,          // argument to thread function 
        0,                      // use default creation flags 
        NULL);   // returns the thread identifier 

    //PushMessage(initBuf, intBufLen);

    return 0;
}

int ShutdownCommunication()
{
    CloseHandle(hThreadReceive);
    CloseHandle(hThreadTransmit);

    // shutdown the connection since no more data will be sent
    int iResult = shutdown(ConnectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

void PushMessage(const char* msg, int len)
{
    if (len > 256)
    {
        XPLMDebugString("Skalarki: Invalid length, > 256\n");
        return;
    }

    if (len == 0)
    {
        XPLMDebugString("Skalarki: Invalid length, = 0\n");
        return;
    }

    DWORD dwWaitResult = WaitForSingleObject(
        hMutex,    // handle to mutex
        INFINITE);  // no time-out interval

    if (MessageCount < 99)
    {
        for (int i = 0; i < len; i++)
        {
            SkalarkiMessageBuffer[WriteIdx][i] = msg[i];
        }

        SkalarkiMessageLength[WriteIdx] = len;
        WriteIdx++;
        if (WriteIdx >= 100)
        {
            WriteIdx = 0;
        }

        //XPLMDebugString("Skalarki: Push message\n");
        MessageCount++;

        if (!ReleaseSemaphore(
            ghSemaphore,  // handle to semaphore
            1,            // increase count by one
            NULL))       // not interested in previous count
        {
            printf("ReleaseSemaphore error: %d\n", GetLastError());
        }
    }

    // Release ownership of the mutex object
    if (!ReleaseMutex(hMutex))
    {
        // Handle error.
    }
}

//WSANOTINITIALISED
static DWORD WINAPI TransmitHandler(LPVOID lpParam)
{
    while (true)
    {
        DWORD dwWaitResult = WaitForSingleObject(
            ghSemaphore,   // handle to semaphore
            INFINITE);           // zero-second time-out interval

        dwWaitResult = WaitForSingleObject(
            hMutex,    // handle to mutex
            INFINITE);  // no time-out interval

        if (MessageCount > 0)
        {
            char* buf = &SkalarkiMessageBuffer[ReadIdx][0];
            int len = SkalarkiMessageLength[ReadIdx];

            /*
            char buffer[100];
            sprintf_s(buffer, "Send: %X, %d\n", (UINT32)buf, len);
            XPLMDebugString(buffer);
            */

            char sendBuff[256];
            for (int i = 0; i < len; i++)
            {
                sendBuff[i] = SkalarkiMessageBuffer[ReadIdx][i];
            }

            int iResult = send(ConnectSocket, sendBuff, len, 0);
            if (iResult != SOCKET_ERROR)
            {
                ReadIdx++;
                if (ReadIdx >= 100)
                {
                    ReadIdx = 0;
                }

                MessageCount--;

                //XPLMDebugString("Skalarki: Send socket ok\n");
            }
            else
            {
                int lastError = WSAGetLastError();
                char buffer[100];
                sprintf_s(buffer, "Skalarki: Send socket failed (%d)\n", lastError);
                XPLMDebugString(buffer);
            }
        }

        // Release ownership of the mutex object
        if (!ReleaseMutex(hMutex))
        {
            // Handle error.
        }

        Sleep(10);
    }
}

static DWORD WINAPI ReceiveHandler(LPVOID lpParam)
{
    while (true)
    {
        int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            //printf("Bytes received: %d\n", iResult);

            ProcessPacket(recvbuf);
        }
    }
}
