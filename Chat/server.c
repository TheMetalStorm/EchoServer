#include <stdio.h> 
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "SimiTCP.h"

#define BUFSIZE 1024 
#define BACKLOG 128

typedef struct client_data{
	int fd;
	char* username;	
	
} client_data;

typedef struct client_list{
	client_data *client_data[BACKLOG];
	int count;
	
} client_list;

void HandleChatMessage(struct epoll_event event, client_list all){
	char buf[BUFSIZE];
	int recvMessageSize;
	client_data *sender = ((client_data*)event.data.ptr);	
	for(;;){
		bzero(buf, sizeof(buf));
		recvMessageSize = read(sender->fd, buf, BUFSIZE);		
		if(recvMessageSize <= 0) {
			break;
		}
		
	 	else {
			printf("INFO: read %ld chars from user %s\n", strlen(buf), sender->username);	
			int outMessageLen = strlen(buf) + strlen(sender->username) + 3;
			char outBuf[outMessageLen];
			snprintf(outBuf, outMessageLen, "%s: %s", sender->username, buf);
			for(int i = 0; i < all.count; i++) {
				client_data *curUserData = ((client_data*)all.client_data[i]);
				if(curUserData == NULL) continue;

				write(curUserData->fd, outBuf, outMessageLen);	

				printf("INFO: wrote %ld chars to user %s\n", strlen(buf), curUserData->username);	

			}
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
	client_list clients = {0};
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
	
	client_data *e = malloc(sizeof(client_data));
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
			if(((client_data*)events[n].data.ptr)->fd == servsock){
				// new connection				
				clientLen = sizeof(echoClientAddr);
				if((clientsock = accept(servsock, (struct sockaddr *) &echoClientAddr, &clientLen))<0){
					fputs("Error on Accept", stderr);
					return 1; 
				}
				
				char buf[BUFSIZE];
				int usernameLen = read(clientsock, buf, BUFSIZE-1);
				buf[usernameLen] = '\0';

				if (setnonblocking(clientsock) == -1){
  					perror("calling fcntl");
					return -1;
				}				


				client_data *data = malloc(sizeof(client_data));
				data->fd = clientsock;
				
				if(buf[strlen(buf)-1] == '\n'){
					buf[strlen(buf)-1] = '\0';
				}

				data->username = strdup(buf);
				printf("Handling client with username: %s\n", data->username);

				ev.data.ptr = data;
				ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP ;
				
				clients.client_data[clients.count] = data;
				clients.count++;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev) == -1){
					perror("epoll ctl: clientsock");
					return -1;
				}

			}

			else if (events[n].events & EPOLLIN) {
				HandleChatMessage(events[n], clients);
			}

			if (events[n].events & (EPOLLRDHUP | EPOLLHUP)) {
				client_data *data = (client_data*)events[n].data.ptr;
				printf("Closing connection of user: %s\n", data->username);	
				
				for(int i = 0; i < clients.count; i++){
					if(clients.client_data[i] == data){
						clients.client_data[i] = clients.client_data[--clients.count];
						break;
					}
				}

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




