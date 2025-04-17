#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * Establishes a socket connection to a server on the specified port.
 *
 * @param sfd         [in/out] Pointer to an integer storing the socket file descriptor.
 *                     If *sfd == -1, a new socket is created. On success, *sfd contains
 *                     the connected socket; on failure, it is set to -1.
 * @param serverPort  [in]     The port (as a string, e.g., "8080") to connect to.
 * @param hints       [in]     Struct containing socket criteria (e.g., AF_INET, SOCK_STREAM).
 *
 * @return            0 on success, 1 on failure (with error messages printed to stderr).
 *
 * @note              The caller is responsible for closing the socket (if successful).
 * @warning           If `sfd` or `serverPort` is NULL, the function fails immediately.
 */
int connectTo(int* sfd, const char* serverAddr, const char* serverPort, struct addrinfo hints){
	if (!sfd || !serverPort) {
        	fprintf(stderr, "Invalid arguments\n");
        	return 1;
	}

	struct addrinfo *result, *rp;
	int s = getaddrinfo(serverAddr, serverPort, &hints, &result);

	if(s != 0){
		perror("Error on getaddrinfo");
		return 1;
	}

	for(rp = result; rp != NULL; rp = rp->ai_next){
		if(*sfd == -1){
			*sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);	
			if (*sfd == -1) {
            		    perror("socket");
                		continue;
            		}
		}
		
		if (connect(*sfd, rp->ai_addr, rp->ai_addrlen)!= -1)
			break;
		
		close(*sfd);
		*sfd = -1;  // Reset socket on failure
	}
	

	if(rp == NULL){
		perror("Could not connect :(");
		return 1;
	}

	freeaddrinfo(result);

	return 0;
}

int bindAndListen(int servsock, struct sockaddr_in echoServerAddr, int maxNumConnections){
	
	if(bind(servsock, (struct sockaddr *) &echoServerAddr, sizeof(echoServerAddr)) < 0){
		fputs("Error binding Server Socket", stderr);
		return 1;
	}

	if(listen(servsock, maxNumConnections) < 0){
		fputs("Error on listen", stderr);
		return 1;
	}

	return 0;
}

int setnonblocking(int fd){
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}
