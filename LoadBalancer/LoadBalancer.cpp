#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

#include "CircularBuffer.h"
#include "Data.h"
#include "SortedList.h"



#define DEFAULT_BUFLEN 512
#define CBLEN 1024
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "5000"
#define DEFAULT_PORT_REC 5001
#define DEFAULT_PORT_DIS "5002"

//funkcije
bool InitializeWindowsSockets();
void *ListenClient(void* arguments);
void *ListeningForNewClients(void* arguments);
void *ListeningForNewWorker(void* arguments);
void *NewWorker(void* arguments);
void* Send(void* arguments);
void *SendDistribution(void* arguments);
void *Distribution(void* arguments);


//promenjljive
char recvbuf[DEFAULT_BUFLEN];
char recvbuf2[DEFAULT_BUFLEN];
char memoryDis[512][1024];
int count = 0;
sem_t mutex;
int dis = 0;
struct arg_struct {
    int iResult;
    SOCKET accSocket;
    //char buffer[DEFAULT_BUFLEN];
   
} *args;


int main()
{
    if (bufferCheck()) {
        printf("Buffer uspesno inicijalizovan!");
    }
    sem_init(&mutex, 0, 1);
    //prihvata klijente i prima podatke
    pthread_t ClientThread;
    //prihvata workere i smesta ih u sorted list
    pthread_t WorkerThread;
    //distribucija
    pthread_t DistributionThread;
    
    

   
    pthread_create(&ClientThread, NULL, &ListeningForNewClients, NULL);
    pthread_create(&WorkerThread, NULL, &ListeningForNewWorker, NULL);
    pthread_create(&DistributionThread, NULL, &Distribution, NULL);
   
   
   

    (void)pthread_join(ClientThread, NULL);
    (void)pthread_join(WorkerThread, NULL);
    (void)pthread_join(DistributionThread, NULL);
   
    
    return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}
void *ListeningForNewWorker(void* arguments) {
    
    SOCKET listenSocket = INVALID_SOCKET;
 
    SOCKET acceptedSocket = INVALID_SOCKET;
    
    int iResult;
    

 

    if (InitializeWindowsSockets() == false)
    {
        
    }
   
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

   
    iResult = getaddrinfo(NULL, DEFAULT_PORT_WORKER, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        
    }

   
    listenSocket = socket(AF_INET,      
        SOCK_STREAM,  
        IPPROTO_TCP); 

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
       
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
       
    }

    printf("Server initialized, waiting for workers.\n");
    pthread_t ListenForNewWorkerThread;
    pthread_t SendTrhread;

    args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
    args->accSocket = listenSocket;


    pthread_create(&ListenForNewWorkerThread, NULL, &NewWorker, args);
    pthread_create(&SendTrhread, NULL, &Send, NULL);

    (void)pthread_join(ListenForNewWorkerThread, NULL);
    (void)pthread_join(SendTrhread, NULL);
    
   

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        
    }

    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();
   


    return NULL;
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
       
    }

    printf("Server initialized, waiting for clients.\n");

    do
    {
       
        


        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
           
        }
        
        pthread_t newThread;
        args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
        args->accSocket = acceptedSocket;
       
        args->iResult = iResult;

      
        pthread_create(&newThread, NULL, &ListenClient, args);


       

    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        
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
            strcat_s(recvbuf, "\0");
            printf("Message received from client: %s\n", recvbuf);
            circularBufferPush(recvbuf);
           
            printf("Successfuly pushed to buffer!\n");
            strcpy(recvbuf,"");
           
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
void *NewWorker(void* arguments) {
    struct arg_struct* args = (arg_struct*)arguments;
   
    SOCKET listenSocket = args->accSocket;
    int iResult;
    SOCKET acceptedSocket;
    do
    {
        acceptedSocket = accept(listenSocket, NULL, NULL);
         
            if (acceptedSocket == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocket);
                WSACleanup();
               
            }
            //sem_wait(&mutex);
            printf("Novi worker!!!(SOCKET: %d)", acceptedSocket);
            struct Data *a = (Data*)malloc(sizeof(struct Data));
            a->DataCount = 0;
            a->accSocket = acceptedSocket;
            
            insert(a);
            SortedList* c = Current();
            if (c->next != NULL) {
                dis = 1;
            }
           
            //sem_post(&mutex);
    } while (true);
    
    

    return NULL;
}

void* Send(void* arguments){
    
    do
    {
      



        
           //sem_wait(&mutex);
        SortedList* c = Current();
        char item[1024] = "";
        int iResult;
        if (bufferCheck() && c!=NULL)
        {

            strcpy_s(item, circularBufferPop());
            strcat_s(item, "\0");
            printf("\nPodatak za salnje: %s\n", item);
            AddToCurrent(strlen(item)-1);
            iResult = send(c->data->accSocket, item, strlen(item)-1, 0);
            
            display();
            strcpy_s(item, "");
            sort();

        }


       
        Sleep(2000);

        //sem_post(&mutex);
    } while (1);

}
void* SendDistribution(void* arguments) {
    struct arg_struct* args = (arg_struct*)arguments;
    SOCKET accSoket = args->accSocket;
    int iResult;
    
    do
    {
    if (dis == 1) {
        sem_wait(&mutex);
        Clear();
        iResult = send(accSoket, "1", strlen("1"), 0);
        int count2 = 0;
        do
        {
            iResult = recv(accSoket, recvbuf2, DEFAULT_BUFLEN, 0);
           
            strcpy_s(memoryDis[count], recvbuf2 + '\0');
            printf("Primljeno za distribuciju:%s\n", memoryDis[count]);
            count++;

            SortedList* c = Current();
            char item[1024] = "";
            int iResult;



            strcpy_s(item, memoryDis[count2]);
            strcat_s(item, "\0");
            printf("\nPodatak za distribuciju: %s\n", item);
            iResult = send(c->data->accSocket, item, strlen(item) , 0);
            AddToCurrent(strlen(item));
            display();
            strcpy_s(item, "");
            strcpy(recvbuf2, "");
            sort();




            count2++;
            Sleep(2000);

        } while (iResult > 0);
       /*
        do
        {


            SortedList* c = Current();
            char item[1024] = "";
            int iResult;
     
           

                strcpy_s(item, memoryDis[count2]);
                strcat_s(item, "\0");
                printf("\nPodatak za distribuciju: %s\n", item);
                iResult = send(c->data->accSocket, item, strlen(item) - 1, 0);
                AddToCurrent(strlen(item) - 1);
                display();
                strcpy_s(item, "");
                sort();

         


            count2++;
            Sleep(2000);

        } while (count2!=count);*/

        for (int i = 0; i < count; i++) {
          // printf("%d:%s\n", i, memory + i);
      
          strcpy(memoryDis[i], "");
        }
        count = 0;
        dis = 0;
        count2 = 0;
        
        strcpy(recvbuf2, "");
        sem_post(&mutex);
    }
    } while (true);
    
        
}
void* Distribution(void* arguments){
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
    iResult = getaddrinfo(NULL, DEFAULT_PORT_DIS, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        
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
           
        }
        pthread_t DisSendThread;
        args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
        args->accSocket = acceptedSocket;

        pthread_create(&DisSendThread, NULL, &SendDistribution, args);
        (void)pthread_join(DisSendThread, NULL);

    } while (true);
   


    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
       
    }



    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();



    return NULL;


}


