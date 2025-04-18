#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "SimiTCP.h"

#define OUTBUFSIZE 1024
#define INBUFSIZE (OUTBUFSIZE * 2)
#define USERNAMESIZE 64

int main(int argc, char const* argv[]){

	if (argc < 3) {
        	fprintf(stderr,"usage: %s ip_addr port_number\n",argv[0]);
        	exit(1);
    	}	
	int sfd = -1;
	const char* ipAddrToConnectTo = argv[1];
	const char* serverPort = argv[2];
	struct addrinfo hints;
	char outBuf[OUTBUFSIZE], inBuf[INBUFSIZE];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_flags = 0;

	if(connectTo(&sfd, ipAddrToConnectTo, serverPort, hints) != 0) return 1;
	
	int  	len;
	// username:
	for(;;){
		printf("Input username : ");
		char* userRes = fgets(outBuf, USERNAMESIZE, stdin);
	
		if (userRes == NULL) {
            		perror("Error reading input");
            		return 1;
        	}
		else if(userRes[0] == '\n'){
		 	continue;
		 }


		len = strlen(outBuf);
        	if (len > 0 && outBuf[len-1] == '\n') {
            		outBuf[len-1] = '\0'; // Remove newline
            		len--;
        	}

		if(write(sfd, outBuf, len) != len){
			perror("Error on message send");
			return 1;
		}	

		break;

	}
	
	// main loop
	// TODO: fix weird behaviour when message with more than 1022 chars 
	// TODO: make input text support common text osp like skiping words etc using readline/editline lib  
	for(;;){
		printf("Input Chat Message: ");
		
		// make non blocking so if we dont send we still read		
		char* getRes = fgets(outBuf, OUTBUFSIZE, stdin);

		if (getRes == NULL) {
            		perror("Error reading input");
            		return 1;
        	}
		else if(getRes[0] == '\n'){
			printf("SKIP\n");
			continue;
		 }


		len = strlen(outBuf);
		if (len > 0 && outBuf[len-1] == '\n') {
            		outBuf[len-1] = '\0'; // Remove newline
            		len--;
        	}

		if(write(sfd, outBuf, len) != len){
			perror("Error on message send");
			return 1;
		}	
		
		int n = read(sfd, inBuf, INBUFSIZE-1);
        	if(n <= 0) {
            		perror("Error on read or connection closed");
            		break;
        	}
        
        	inBuf[n] = '\0';
	      
		printf("%s\n", inBuf);
	}

	return 0;
}
