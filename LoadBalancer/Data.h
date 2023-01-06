#pragma once
#include "../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um/WinSock2.h"
struct Data 
{
	char data[512];
	SOCKET acceptedSocket;

};

struct ClientData
{
	char data[512];
	int len;

};