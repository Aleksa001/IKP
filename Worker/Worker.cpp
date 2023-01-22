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
    
    SOCKET connectSocket = INVALID_SOCKET;
    
    int iResult;
    // message to send
    char m[] = "this is a test";
    char* messageToSend = m;



    if (InitializeWindowsSockets() == false)
    {
        
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        
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
   
    iResult = send(connectSocket, messageToSend, DEFAULT_BUFLEN, 0);
    
    do
    { 
        
        iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        //strcat_s(recvbuf, "\00");

        
        if (iResult > 0)
        {
            
            strcpy_s(memory[count], recvbuf);
            printf("Message received from client: %s.\n", memory[count]);
            printf("Memory:\n");
            for (int i = 0; i < count + 1; i++) {
                printf("%d:%s\n", i, memory[i]);
            }
            count++;
            strcpy(recvbuf, "");
            strcpy(memory[count], "");
            
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
   
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    for (int i = 0; i < count + 1; i++) {

        strcpy(memory[i], "");

    }
    count = 0;
    return NULL;
}
void* Distribution(void* arguments) {

   
    SOCKET connectSocket = INVALID_SOCKET;
   
    int iResult;
    // message to send
    char m[] = "this is a test";
    char* messageToSend = m;



    if (InitializeWindowsSockets() == false)
    {
       
    }

   
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
    }

   
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
   
    serverAddress.sin_port = htons(DEFAULT_PORT_DIS);
   
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
    


    char recvbuf2[DEFAULT_BUFLEN];
    char dis[1024] = "";
    do
    {
        iResult = recv(connectSocket, recvbuf2, DEFAULT_BUFLEN, 0);
        if (atoi(recvbuf2) == 1) {
            strcpy(recvbuf2, "");
            for (int i = 0; i < count+1; i++) {
                // printf("%d:%s\n", i, memory + i);
                strcpy(dis, memory[i]);
                iResult = send(connectSocket, dis, strlen(dis), 0);
                strcpy(memory[i], "");
                strcpy(dis, "");
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
   
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    for (int i = 0; i < count + 1; i++) {
        
        strcpy(memory[i], "");
     
    }
    count = 0;
    return NULL;
}
