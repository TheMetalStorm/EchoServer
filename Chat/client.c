// TODO: Find alternative way to print error and dont crash 
// TODO: handle resize with SIGWINCH
// TODO: print multi line chat
// TODO: display multi line chat
// TODO: on new message move existing chat messages up by actual new number of lines

// TODO: when user AA connects, disconnects and connects again, his words dont get transmitted (could also be same FD problem)

#include <ctype.h>
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
	// cbreak();           // Disable line buffering
	noecho();    
    keypad(stdscr, TRUE); // Enable special keys

	char c = getch();
	if(c == -1) {
		echo();	
		return -1;
	}

	else if(c == (char)KEY_BACKSPACE){
		
		echo();
		buf[strlen(buf)-1] = '\0';
		return -1;
	}
	
	else if(isprint(c)){
		buf[strlen(buf)] = c;
		buf[strlen(buf)+1] = '\0';
	}

	
	

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
	bzero(outBuf, OUTBUFSIZE);

	for(;;){		

		int userRes = getstrnb(outBuf);//fgets(outBuf, USERNAMESIZE, stdin);

		move(row-1, 0);
		clrtoeol();
		printw("Input Username: %s", outBuf);

	
		if (userRes != -1) {	

			len = strlen(outBuf);
        	if(len != 0) {

					if (outBuf[len-1] == '\n') {
							outBuf[len-1] = '\0'; // Remove newline
							len--;
					}

					if(strlen(outBuf) == 0) continue;

					if(write(sfd, outBuf, len) != len){
						//perror("Error on message send");
						//continue;
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
	chtype lineCopyBuf[col];
	for(;;){
		int getStrRes = getstrnb(outBuf );
		
		move(row-1, 0);
		clrtoeol();
		printw("%s%s", inputMessage, outBuf);

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

        

		for (int i = 0; i<=row-2; i++){
			
			move(i, 0);
			inchnstr(lineCopyBuf, sizeof(lineCopyBuf));
			clrtoeol();
			for(int j = 0; j<col; j++){
				move(i-1, j);
				char c = lineCopyBuf[j] & A_CHARTEXT;
				printw("%c", c);
			}
			
			
			
			bzero(lineCopyBuf, sizeof(lineCopyBuf));
		}

        inBuf[n] = '\0';  
		mvprintw(row-2, 0, "%s\n", inBuf);
		refresh();
	
	}
	return 0;
}
