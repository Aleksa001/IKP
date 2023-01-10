#pragma once
#include <WinSock2.h>

typedef struct WorkerData {
	char *memory[512];
	SOCKET acceptedSocket;

}WorkerData;
