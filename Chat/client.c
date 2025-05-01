// TODO: make input text support common text osp like skiping words etc using readline/editline lib  
// TODO: Find alternative way to print error and dont crash 
// TODO: handle resize with SIGWINCH
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#include "SimiTCP.h"

#define OUTBUFSIZE 1024
#define INBUFSIZE (OUTBUFSIZE * 2)
#define USERNAMESIZE 64

void deinitNcurses(void){
	endwin();
}

int getstrnb(char *buf){
	char c;
	noecho();
	c = getch();
	if(c == -1) {
		echo();	
		return -1;
	}

	if(c == 127){
		echo();
		buf[strlen(buf)] = '\0';
		
		return -1;
	}

	buf[strlen(buf)] = c;
	buf[strlen(buf)+1] = '\0';
	
	

	//TODO: check max buffer len

	if(c == '\n') {
		echo();
		return 0;
	}
	
	echo();
	return -1;

}

int main(int argc, char const* argv[]){

	if (argc < 3) {
        	fprintf(stderr,"usage: %s ip_addr port_number\n",argv[0]);
        	exit(1);
    	}	
	
	int row, col;
	initscr();
    getmaxyx(stdscr, row, col); /* get the number of rows and columns */
	atexit(deinitNcurses);
	
	int sfd = -1;
	const char* ipAddrToConnectTo = argv[1];
	const char* serverPort = argv[2];
	struct addrinfo hints;
	char outBuf[OUTBUFSIZE] = {0}, inBuf[INBUFSIZE];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_flags = 0;

	if(connectTo(&sfd, ipAddrToConnectTo, serverPort, hints) != 0) return 1;
	
	setnonblocking(sfd);
	//nodelay(stdscr, true);
	timeout(100);
	int  	len;

	for(;;){
		//clear();
		

		int userRes = getstrnb(outBuf);//fgets(outBuf, USERNAMESIZE, stdin);

		mvprintw(row-1,0,"Input Username: %s", outBuf);

		//refresh();
	
		if (userRes != -1) {	
			len = strlen(outBuf);
        	if(len != 0) {
				if (len > 0 && outBuf[len-1] == '\n') {
						outBuf[len-1] = '\0'; // Remove newline
						len--;
				}


				if(write(sfd, outBuf, len) != len){
					//perror("Error on message send");
					continue;
				}

				break;
			}	
		}	

		// else {
   		// 	//perror("Error reading input");
		// 	continue;
        // }
		refresh();

	}
	
	clear();
	char *inputMessage = "Input Message: ";
	int inputMessageLen = strlen(inputMessage);
	bzero(outBuf, OUTBUFSIZE);

	// main loop
	// TODO: fix weird behaviour when message with more than 1022 chars 
	for(;;){
		int getStrRes = getstrnb(outBuf );
		mvprintw(row-1,0,"%s%s", inputMessage, outBuf);

		if (getStrRes != -1) {

			len = strlen(outBuf);
			if(len != 0) {
				if (outBuf[len-1] == '\n') {
					outBuf[len-1] = '\0'; // Remove newline
					len--;
				}
				
				if(write(sfd, outBuf, len) != len){
					//perror("Error on message send");
					//continue;	
				}
					
				move(row-1, inputMessageLen);
				clrtoeol();
				bzero(outBuf, OUTBUFSIZE);
				refresh();

			}
		}



		int n = read(sfd, inBuf, INBUFSIZE-1);
		if(n <= 0) {
		  	refresh();
		  	continue;
         }

        
        inBuf[n] = '\0';  
		mvprintw(row-2, 0, "%s\n", inBuf);
		refresh();
	
	}
	return 0;
}
