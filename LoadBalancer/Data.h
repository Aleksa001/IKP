#pragma once
#include <WinSock2.h>
typedef struct Data 
{
	
	int DataCount;
	SOCKET accSocket;

} Data;

typedef struct ClientData
{
	char data[512];
	int len;

} DataClient;
