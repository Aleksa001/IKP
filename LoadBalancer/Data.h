#pragma once
#include "../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um/WinSock2.h"
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
