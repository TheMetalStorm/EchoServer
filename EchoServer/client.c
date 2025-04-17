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

	if (argc < 2) {
        	fprintf(stderr,"usage: %s port_number\n",argv[0]);
        	exit(1);
    	}	

	int sfd = -1;
	const char* serverPort = argv[1];
	struct addrinfo hints;
	char buf[BUFSIZE];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_flags = 0;

	if(connectTo(&sfd, serverPort, hints) != 0) return 1;

	int  	len;
	
	for(;;){
		printf("Input message to Echo: ");
		
		char* getRes = fgets(buf, BUFSIZE, stdin);

		if (getRes == NULL) {
            		perror("Error reading input");
            		return 1;
        	}
		else if(getRes[0] == '\n'){
		 	continue;
		 }


		len = strlen(buf);
        	if (len > 0 && buf[len-1] == '\n') {
            		buf[len-1] = '\0'; // Remove newline
            		len--;
        	}

		if(write(sfd, buf, len) != len){
			perror("Error on message send");
			return 1;
		}	

		int n = read(sfd, buf, BUFSIZE-1);
        	if(n <= 0) {
            		perror("Error on read or connection closed");
            		break;
        	}
        
        	buf[n] = '\0';
	      
		printf("Received message from server: %s\n", buf);
	}

	return 0;
}
