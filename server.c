#include <stdio.h> 
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "SimiTCP.h"

#define RCVBUFSIZE 255
#define BACKLOG 5

void HandleTCPClient(struct epoll_event event){
	char buf[RCVBUFSIZE];
	int recvMessageSize;
	
	for(;;){
		bzero(buf, sizeof(buf));
		recvMessageSize = read(event.data.fd, buf, RCVBUFSIZE);		
	
		if(recvMessageSize <= 0)
			break;
	 	else {
			write(event.data.fd, buf, strlen(buf));
		}
	}	

}

int main(int argc, char const* argv[]){
	
	int servsock, clientsock, nfds, epollfd;
	struct epoll_event ev, events[BACKLOG];
	struct sockaddr_in echoServerAddr, echoClientAddr;
	unsigned short echoServerPort = 1234;
     	unsigned int clientLen;	

	servsock = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	
	if(servsock < 0){
		fputs("Error creating Server Socket", stderr);
		return 1;	
	}	
	if(setnonblocking(servsock) == -1){
		perror("setnonblocking servosck");
		return 1;
			
	}
	memset(&echoServerAddr, 0, sizeof(echoServerAddr));
	echoServerAddr.sin_family = AF_INET;
	echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServerAddr.sin_port = htons(echoServerPort); 
	
	if(bindAndListen(servsock, echoServerAddr, BACKLOG)<0){
		return 1;
	}	
	
	epollfd = epoll_create(1);
	if (epollfd == -1)
	{
		perror("epoll_create1");
		return -1;		
	}

	ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.fd = servsock;

	int epoll_ctl_res = epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock, &ev);
	
	if(epoll_ctl_res == -1){
		perror("epoll ctl: serversock");
		return 1;
	}

	for(;;){
		nfds = epoll_wait(epollfd, events, BACKLOG, -1);
		if(nfds == -1){
			perror("epoll_wait");
			return -1;
		} 
		
		int n;
		for (n = 0; n< nfds; ++n){
			if(events[n].data.fd == servsock){
				// new connection				
				clientLen = sizeof(echoClientAddr);
				if((clientsock = accept(servsock, (struct sockaddr *) &echoClientAddr, &clientLen))<0){
					fputs("Error on Accept", stderr);
					return 1; 
				}
				
				printf("Handling client %s\n", inet_ntoa(echoClientAddr.sin_addr));
				

				if (setnonblocking(clientsock) == -1){
  					perror("calling fcntl");
					return -1;
				}				
				ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP ;
				ev.data.fd = clientsock;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev) == -1){
					perror("epoll ctl: clientsock");
					return -1;

				}

				
			}

			else if (events[n].events & EPOLLIN) {
				HandleTCPClient(events[n]);
			}

			if (events[n].events & (EPOLLRDHUP | EPOLLHUP)) {
				
				puts("Closing connection");	
				
				epoll_ctl(epollfd, EPOLL_CTL_DEL,
					  events[n].data.fd, NULL);

				close(clientsock);
				continue;
			}
		
		}

		

	}	

	return 0;	
}




