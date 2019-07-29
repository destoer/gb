#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "headers/net.h"
#ifdef SERIAL
// probably should be init client but allow it
void init_client(Connection_struct *c,char *ip,char *port)
{
	int rc = WSAStartup(MAKEWORD(2,2), &c->wsaData);
	if(rc != 0)
	{
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		exit(1);
	}
	
	ZeroMemory(&c->hints,sizeof(c->hints));
	c->hints.ai_family = AF_UNSPEC;
	c->hints.ai_socktype = SOCK_STREAM;
	c->hints.ai_protocol = IPPROTO_TCP;
	
	//resolve server address and port
	rc = getaddrinfo(ip,port,&c->hints, &c->result);
	if(rc != 0)
	{
		puts("getaddrinfo failed!");
		WSACleanup();
		exit(1);
	}
	
	// attempt connection until one succeeds
	for(c->ptr=c->result; c->ptr != NULL; c->ptr=c->ptr->ai_next)
	{
		c->clientSocket = socket(c->ptr->ai_family, c->ptr->ai_socktype, c->ptr->ai_protocol);
		if(c->clientSocket == INVALID_SOCKET)
		{
			printf("socket failed: %d\n", WSAGetLastError());
			WSACleanup();
			exit(1);
		}

		// connect to the server
		rc = connect(c->clientSocket,c->ptr->ai_addr, (int)c->ptr->ai_addrlen);
		if(rc == SOCKET_ERROR)
		{
			closesocket(c->clientSocket);
			c->clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	
	freeaddrinfo(c->result);
	
	if(c->clientSocket == INVALID_SOCKET)
	{
		puts("unable to connect to server");
		WSACleanup();
		exit(1);
	}
}



bool peek_data(Connection_struct *c)
{

	unsigned long rc;
	ioctlsocket(c->clientSocket,FIONREAD,&rc);
	
	return rc > 0;
}

// init the server ready to listen for a client connection
void init_server(Connection_struct *c)
{
	int rc = WSAStartup(MAKEWORD(2,2), &c->wsaData);
	if(rc != 0) // non zero is a fatal error
	{
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		exit(1); 
	}
	
	
	
	ZeroMemory(&c->hints,sizeof(c->hints));
	c->hints.ai_family = AF_INET;
	c->hints.ai_socktype = SOCK_STREAM;
	c->hints.ai_protocol = IPPROTO_TCP;
	c->hints.ai_flags = AI_PASSIVE;
	
	// resolve the server address and port
	// hardcoded port
	rc = getaddrinfo(NULL,"6996", &(c->hints), &(c->result));
	if(rc != 0)
	{
		printf("getaddrinfo failed: %d\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	
	// create a socket connecting to the server
	c->listenSocket = socket(c->result->ai_family, c->result->ai_socktype, c->result->ai_protocol);
	if(c->listenSocket == INVALID_SOCKET)
	{
		printf("socket failed: %d\n", WSAGetLastError());
		freeaddrinfo(c->result);
		WSACleanup();
		exit(1);
	}
	
	
	// bind the socket
	rc = bind(c->listenSocket,c->result->ai_addr, (int)c->result->ai_addrlen);
	if(rc == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(c->result);
		closesocket(c->listenSocket);
		WSACleanup();
		exit(1);
	}
	
	freeaddrinfo(c->result);
}


int send_byte(Connection_struct *c,const char byte, char flag)
{
	char buf[2];
	
	// zero contains hte flag to tell what the data sent is for
	buf[0] = flag; 
	buf[1] = byte; // buf[1] houses the actual byte
	
	int rc = send(c->clientSocket,buf,2,0);
	if(rc == SOCKET_ERROR)
	{	
		printf("send failed: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}
	
	// other side has ended the connection
	else if(rc == 0)
	{
		puts("Connection closed!");
		closesocket(c->clientSocket);
		return SOCKET_ERROR;
	}
	return  1;
}


// recv from the client for a fixed length
// check len does not exceed max send size
int recv_byte(Connection_struct *c,char *byte)
{
	
	char buf[2];
	// recv data for length
	int rc = recv(c->clientSocket,buf,2,0);
	
	if( rc < 0 )
	{
		printf("recv failed: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}	
	// other side has ended the connection
	else if(rc == 0)
	{
		puts("Connection closed!");
		closesocket(c->clientSocket);
		return SOCKET_ERROR;		
	}
	
	*byte = buf[1]; // give it back the actual data
	
	return buf[0];
}
#endif