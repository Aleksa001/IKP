#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <iostream>
#include "WorkerStruct.h"
#include <pthread.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5000
#define DEFAULT_PORT_REC "5001"
#define DEFAULT_PORT_DIS 5002

bool InitializeWindowsSockets();
void *RegisterToLoadBalancer(void* arguments);
void* Distribution(void* arguments);

char memory[512][1024];
int count = 0;
char recvbuf[DEFAULT_BUFLEN];


int main()
{
    
    pthread_t RegisterThread;
   
    pthread_t DistributionThreaad;


    pthread_create(&RegisterThread, NULL, &RegisterToLoadBalancer, NULL);
    pthread_create(&DistributionThreaad, NULL, &Distribution, NULL);
  


    (void)pthread_join(RegisterThread, NULL);
    (void)pthread_join(DistributionThreaad, NULL);

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
void *RegisterToLoadBalancer(void* arguments) {
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char m[] = "this is a test";
    char* messageToSend = m;



    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
       // return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        //return 1;
    }

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_pton(AF_INET6,"127.0.0.1", );
    //inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(DEFAULT_PORT);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
    //dodam na kraj oznaku za kraj stringa jer posle ispisuje nesto dodatno nakon same poruke
    iResult = send(connectSocket, messageToSend, DEFAULT_BUFLEN, 0);
    
    
   
    do
    { 
        
        iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        //strcat_s(recvbuf, "\00");
        //memory[count] = (char*)malloc(strlen(recvbuf));
        strcpy_s(memory[count], recvbuf+'\0');
        if (iResult > 0)
        {
            printf("Message received from client: %s.\n", memory[count]);
            count++;
            strcpy(recvbuf, "");
            

        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            //closesocket(acceptedSocket);

        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            //closesocket(acceptedSocket);
            //break;

        }
        
        //strcpy_s(recvbuf, "");
        printf("Memory:\n");
        for (int i = 0; i < count; i++) {
            printf("%d:%s\n", i, memory+i);
        }
        Sleep(4000);
       
    } while (true);
  

   
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        //return 1;
    }
    //duzina poruke 
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    return NULL;
}

void* Distribution(void* arguments) {

    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char m[] = "this is a test";
    char* messageToSend = m;



    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
       // return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        //return 1;
    }

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_pton(AF_INET6,"127.0.0.1", );
    //inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(DEFAULT_PORT_DIS);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
    //dodam na kraj oznaku za kraj stringa jer posle ispisuje nesto dodatno nakon same poruke
    //iResult = send(connectSocket, messageToSend, DEFAULT_BUFLEN, 0);


    char recvbuf2[DEFAULT_BUFLEN];
    char dis[1024] = "";
    do
    {
        iResult = recv(connectSocket, recvbuf2, DEFAULT_BUFLEN, 0);
        if (atoi(recvbuf2) == 1) {
            strcpy(recvbuf2, "");
            for (int i = 0; i < count; i++) {
                // printf("%d:%s\n", i, memory + i);
                strcpy(dis, memory[i]);
                iResult = send(connectSocket, memory[i], strlen(memory[i]), 0);
                strcpy((char*)memory + i, "");
            }
            count = 0;
            printf("Memory:\n");
            for (int i = 0; i < count; i++) {
                printf("%d:%s\n", i, memory + i);
            }

        }




    } while (true);



    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        //return 1;
    }
    //duzina poruke 
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    return NULL;
}
