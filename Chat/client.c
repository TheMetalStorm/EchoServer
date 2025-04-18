// TODO: make input text support common text osp like skiping words etc using readline/editline lib  
// TODO: Find alternative way to print error and dont crash 
// TODO: handle resize with SIGWINCH
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ncurses.h>

#include "SimiTCP.h"

#define OUTBUFSIZE 1024
#define INBUFSIZE (OUTBUFSIZE * 2)
#define USERNAMESIZE 64

void deinitNcurses(void){
	endwin();
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
		clear();
		mvprintw(row-1,0,"Input Username: ");
		refresh();
	
		int userRes = getstr(outBuf);//fgets(outBuf, USERNAMESIZE, stdin);
	

		if (userRes == OK) {	
			len = strlen(outBuf);
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

		else{
   			//perror("Error reading input");
			continue;
        	}


	}
	
	
	// main loop
	// TODO: fix weird behaviour when message with more than 1022 chars 
	for(;;){
		clear();
		mvprintw(row-1,0,"Input message: ");
		refresh();

		int getRes = getstr(outBuf );

		if (getRes == OK) {
			len = strlen(outBuf);
			if (len > 0 && outBuf[len-1] == '\n') {
            			outBuf[len-1] = '\0'; // Remove newline
            			len--;
        		}

			if(write(sfd, outBuf, len) != len){
				//perror("Error on message send");
				return 1;
			}	

		}
		else{	
			perror("Error reading input");
            		return 22;
        	}


			
		//int n = read(sfd, inBuf, INBUFSIZE-1);
        	//if(n <= 0) {
            	//	perror("Error on read or connection closed");
            	//	break;
        	//}
        
        	//inBuf[n] = '\0';
	      
		//printf("%s\n", inBuf);
	}

	return 0;
}
