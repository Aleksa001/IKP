#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include "CircularBuffer.cpp"
#include "Data.h"


#define DEFAULT_BUFLEN 512
#define CBLEN 1024
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "5000"
#define DEFAULT_PORT_REC 5001

bool InitializeWindowsSockets();
void *ListenClient(void* arguments);
void *ListeningForNewClients(void* arguments);
void *ListeningForNewWorker(void* arguments);
void *SendDataToWorker(void* arguments);
char recvbuf[DEFAULT_BUFLEN];
struct arg_struct {
    int iResult;
    SOCKET accSocket;
    //char buffer[DEFAULT_BUFLEN];
   
} *args;


int main()
{
    if (bufferCheck()) {
        printf("Buffer uspesno inicijalizovan");
    }
    //prihvata klijente i prima podatke
    pthread_t ClientThread;
    //prihvata workere i smesta ih u sorted list
    pthread_t WorkerThread;
    //odlucuje kom workeru salje
    

   
    pthread_create(&ClientThread, NULL, &ListeningForNewClients, NULL);
    pthread_create(&WorkerThread, NULL, &ListeningForNewWorker, NULL);
 
   
   

    (void)pthread_join(ClientThread, NULL);
    (void)pthread_join(WorkerThread, NULL);
   
    
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
void *ListeningForNewWorker(void* arguments) {
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data

 

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        //return 1;
    }
    //init bufffer
   
    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT_WORKER, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        //return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_STREAM,  // stream socket
        IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
       // return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
       // return 1;
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
       // return 1;
    }

    printf("Server initialized, waiting for workers.\n");

    do
    {
       
        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
           // return 1;
        }
        printf("%d", acceptedSocket);
        Data a;
        a.DataCount = 0;
        a.accSocket = acceptedSocket;
        //ddoa ovo u sorted list 

       //odredi kom ce da salje
        pthread_t LoadBalancerThread;
        pthread_create(&LoadBalancerThread, NULL, &SendDataToWorker, NULL);
        (void)pthread_join(LoadBalancerThread, NULL);
        //TODO
        //stavlja se Data a u sorted list

    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        //return 1;
    }

    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();
   



}
void *ListeningForNewClients(void* arguments) {
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data

   

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        //return 1;
    }
    //init bufffer
    

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        //return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_STREAM,  // stream socket
        IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        //return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        //return 1;
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
       // return 1;
    }

    printf("Server initialized, waiting for clients.\n");

    do
    {
        // Wait for clients and accept client connections.
        // Returning value is acceptedSocket used for further
        // Client<->Server communication. This version of
        // server will handle only one client.

        //Cekamo klijente i kad se poveze pravimo novi tred za njega koji slusa samo njega


        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            //return 1;
        }
        //ovde da se pravi novi tred za svakog kliejnt i da slusa za poruke
        // nego nece da mi ukljuci nesto pthred.h i ne mogu da koristim....
        pthread_t newThread;
        args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
        args->accSocket = acceptedSocket;
       
        args->iResult = iResult;

        //ListenClient(iResult, acceptedSocket,recvbuf,cb);
        pthread_create(&newThread, NULL, &ListenClient, args);


        // here is where server shutdown loguc could be placed

    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        //return 1;
    }

    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();
    

}
void *ListenClient(void* arguments) {
    struct arg_struct* args = (arg_struct*)arguments;
    int iResult = args->iResult;
    SOCKET acceptedSocket = args->accSocket;
    
    do {

        //Receive data until the client shuts down the connection
        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("Message received from client: %s.\n", recvbuf);
            circularBufferPush(recvbuf);
           
            printf("Successfuly pushed to buffer!\n");
           
        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            closesocket(acceptedSocket);

        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(acceptedSocket);
            break;

        }


    } while (1);
    return NULL;
}
void *SendDataToWorker(void* arguments) {
    //TODO
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        //return 1;
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
    serverAddress.sin_port = htons(DEFAULT_PORT_REC);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
    
    do {

       
        char item[1024] = "";
        strcpy(item, circularBufferPop());

        //buffer = item;
        
        if (item != NULL) {
            iResult = send(connectSocket, item, strlen(item), 0);
           
        }
        

        /*if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            //return 1;
        }*/
        //duzina poruke 
        

    } while (true);


    // cleanup
    closesocket(connectSocket);
    WSACleanup();





    return NULL;
}


