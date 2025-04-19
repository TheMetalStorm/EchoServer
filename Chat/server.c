#include <stdio.h> 
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "SimiTCP.h"

#define BUFSIZE 1024 
#define BACKLOG 128

typedef struct echo_data{
	int fd;
	char* username;	
	
} echo_data;

void HandleChatMessage(struct epoll_event event, struct epoll_event *all){
	char buf[BUFSIZE];
	int recvMessageSize;
	echo_data *data = ((echo_data*)event.data.ptr);	
	for(;;){
		bzero(buf, sizeof(buf));
		recvMessageSize = read(data->fd, buf, BUFSIZE);		
		if(recvMessageSize <= 0)
			break;
	 	else {
			printf("INFO: read %ld chars from user %s\n", strlen(buf), data->username);	
			int outMessageLen = strlen(buf) + strlen(data->username) + 3;
			char outBuf[outMessageLen];
			snprintf(outBuf, outMessageLen, "%s: %s", data->username, buf);
			for(int i = 0; i < BACKLOG; i++) {
				//if(all[i].data.ptr == NULL ) continue;
				echo_data *curUserData = ((echo_data*)all[i].data.ptr);
				if(curUserData == NULL) continue;
				printf("fd: %d, username: %s\n", curUserData->fd, curUserData->username);	
				//if(curUserData->fd != 0 && curUserData->username != 0 ) {
					write(curUserData->fd, outBuf, outMessageLen);	
					printf("INFO: wrote %ld chars to user %s\n", strlen(buf), curUserData->username);	
				//}
			}
			
			//write(data->fd, outBuf, outMessageLen);	
			//write(data->fd, data->username, strlen(data->username));
			//write(data->fd, ": ", 2);
			//write(data->fd, buf, strlen(buf));
		}
	}	

}

int main(int argc, char const* argv[]){
	
	if (argc < 2) {
        	fprintf(stderr,"usage: %s port_number\n",argv[0]);
        	exit(1);
    	}
	int servsock, clientsock, nfds, epollfd;
	struct epoll_event ev, events[BACKLOG] = {0};
	struct sockaddr_in echoServerAddr, echoClientAddr;
	unsigned short echoServerPort = atoi(argv[1]);
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
	
	int yes = 1;
	if (setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    		perror("setsockopt(SO_REUSEADDR) failed");
    		return 1;
	}

	if (setsockopt(servsock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0) {
    		perror("setsockopt(SO_REUSEPORT) failed");
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
	
	echo_data *e = malloc(sizeof(echo_data));
       	e->fd = servsock;
	e->username = NULL; 	

	ev.data.ptr = e; 
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
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
			if(((echo_data*)events[n].data.ptr)->fd == servsock){
				// new connection				
				clientLen = sizeof(echoClientAddr);
				if((clientsock = accept(servsock, (struct sockaddr *) &echoClientAddr, &clientLen))<0){
					fputs("Error on Accept", stderr);
					return 1; 
				}
				
				char buf[BUFSIZE];
				int usernameLen = read(clientsock, buf, BUFSIZE-1);
				buf[usernameLen] = '\0';
				
				echo_data *data = malloc(sizeof(echo_data));
				data->fd = clientsock;
				data->username = strdup(buf);

				printf("Handling client with username: %s\n", data->username);

				if (setnonblocking(clientsock) == -1){
  					perror("calling fcntl");
					return -1;
				}				
			
				ev.data.ptr = data;
				ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP ;
				
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev) == -1){
					perror("epoll ctl: clientsock");
					return -1;
				}

			}

			else if (events[n].events & EPOLLIN) {
				HandleChatMessage(events[n], events);
			}

			if (events[n].events & (EPOLLRDHUP | EPOLLHUP)) {
				echo_data *data = (echo_data*)events[n].data.ptr;
				printf("Closing connection of user: %s\n", data->username);	
				
				epoll_ctl(epollfd, EPOLL_CTL_DEL,
					  data->fd, NULL);
				
				close(clientsock);
				close(data->fd);
				free(data->username);
				free(data);
				
				
				continue;
			}
		
		}

		

	}	

	close(servsock);
	free(e);

	return 0;	
}




