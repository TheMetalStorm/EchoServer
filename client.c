#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "SimiTCP.h"

#define BUFSIZE 1024

int main(int argc, char const* argv[]){
	
	int sfd = -1;
	const char* serverPort = "1234";
	struct addrinfo hints;
	char inbuf[BUFSIZE], outbuf[BUFSIZE];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_flags = 0;

	if(connectTo(&sfd, serverPort, hints) != 0) return 1;

	int  	len;
    int	lenread = 0;
	
	for(;;){
		printf("Input message to Echo: ");
		
		char* getRes = fgets(outbuf, BUFSIZE, stdin);

		if (getRes == NULL) {
            		perror("Error reading input");
            		return 1;
        	}
		else if(getRes[0] == '\n'){
		 	continue;
		 }


		len = strlen(outbuf);
        	if (len > 0 && outbuf[len-1] == '\n') {
            		outbuf[len-1] = '\0'; // Remove newline
            		len--;
        	}

		if(write(sfd, outbuf, len) != len){
			perror("Error on message send");
			return 1;
		}	

		int n = read(sfd, inbuf, BUFSIZE-1);
        	if(n <= 0) {
            		perror("Error on read or connection closed");
            		break;
        	}
        
        	inbuf[n] = '\0';
	      
		if(lenread == -1){
			perror("error on read");
			return 1;
		}
			
		inbuf[n] = 0;

		printf("Received message from server: %s\n", inbuf);
	}

	return 0;
}
