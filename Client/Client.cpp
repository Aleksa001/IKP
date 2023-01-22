#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <iostream>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5059


bool InitializeWindowsSockets();


int main()
{
    
    SOCKET connectSocket = INVALID_SOCKET;
   
    int iResult;
    


    if (InitializeWindowsSockets() == false)
    {
        
        return 1;
    }

    
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        
    serverAddress.sin_port = htons(DEFAULT_PORT);
    
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
    char buffer[DEFAULT_BUFLEN];
    int n;
    do {

        
        printf("Type message:");
        n = 0;
        while ((buffer[n++] = getchar()) != '\n');
        if ((strncmp(buffer, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
        //dodam na kraj oznaku za kraj stringa jer posle ispisuje nesto dodatno nakon same poruke
        buffer[n++] = '\0';
        
        iResult = send(connectSocket, buffer, n, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
        //duzina poruke 
        printf("Bytes Sent: %ld\n", n - 1);

    } while (true);


    // cleanup
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

