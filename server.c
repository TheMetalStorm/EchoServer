#include <stdio.h> 
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "SimiTCP.h"

#define RCVBUFSIZE 255
#define BACKLOG 5

//TODO: handle multiple connections

void HandleTCPClient(int clientsock){
	int buf[RCVBUFSIZE];
	int recvMessageSize;
	if((recvMessageSize = recv(clientsock, buf, RCVBUFSIZE, 0)) < 0){
		fputs("Error on recv()", stderr);
		exit(1);
	}
	
	while(recvMessageSize > 0){
		if (send(clientsock, buf, recvMessageSize, 0) != recvMessageSize)
		fputs("send() failed", stderr);
		if ((recvMessageSize = recv(clientsock, buf, RCVBUFSIZE, 0)) < 0)
		fputs("recv() failed", stderr);
	}

	puts("Closing connection");	
	close(clientsock);
}

int main(int argc, char const* argv[]){
	
	int servsock, clientsock;
	struct sockaddr_in echoServerAddr, echoClientAddr;
	unsigned short echoServerPort = 1234;
     	unsigned int clientLen;	

	servsock = socket(PF_INET, SOCK_STREAM ,IPPROTO_TCP);
	
	if(servsock < 0){
		fputs("Error creating Server Socket", stderr);
		return 1;	
	}	

	memset(&echoServerAddr, 0, sizeof(echoServerAddr));
	echoServerAddr.sin_family = AF_INET;
	echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServerAddr.sin_port = htons(echoServerPort); 
	
	if(bindAndListen(servsock, echoServerAddr, BACKLOG)<0){
		return 1;
	}	
	
	for(;;){
		clientLen = sizeof(echoClientAddr);
		
		if((clientsock = accept(servsock, (struct sockaddr *) &echoClientAddr, &clientLen))<0){
		
			fputs("Error on Accept", stderr);
			return 1; 
		}

		printf("Handling client %s\n", inet_ntoa(echoClientAddr.sin_addr));

		HandleTCPClient(clientsock);	
	}	

	return 0;	
}




