#pragma once
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501 // keep windows happy and define the version we want to use
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string.h>


// struct for connections (link cable)
typedef struct {
	struct addrinfo *result;
	struct addrinfo *ptr;
	struct addrinfo hints;
	SOCKET clientSocket;
	SOCKET listenSocket;
	WSADATA wsaData;
} Connection_struct;


// if 2nd byte of data is to init a 
// conneciton to a slave get ready to recv data
#define SEND_DATA 1
#define SEND_INIT_SLAVE 2




// Connection_struct defined in lib.h
// find a way to get this all together in the net impl
// in a way that doesent suck

void init_client(Connection_struct *c,char *ip,char *port);
void init_server(Connection_struct *c);
int send_byte(Connection_struct *c,const char byte, char flag);
int recv_byte(Connection_struct *c,char *byte);
bool peek_data(Connection_struct *c);
