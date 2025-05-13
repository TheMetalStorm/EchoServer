// TODO: sometimes server doesnt recognize client connection --> seems to be port connected
// TODO: term resize sometimes dont work so good

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
#include <signal.h>

#include "SimiTCP.h"

#define OUTBUFSIZE 1024
#define INBUFSIZE (OUTBUFSIZE * 2)
#define USERNAMESIZE 64


#define INFOROW 0
#define INFOBORDERROW (INFOROW + 1)
#define UPPERTEXTMAX (INFOBORDERROW + 1) 

#define INPUTMESSAGEROW (row - 1)
#define INPUTBORDERROW (INPUTMESSAGEROW - 1)
#define LOWERTEXTBOUND (INPUTBORDERROW - 1) 

int col, row;
void drawHorizontalLine(int y_pos){
	for (int x = 0; x < col; x++) {
        mvaddch(y_pos, x, ACS_HLINE); // Using ncurses' horizontal line character
    }
}
    
static void sigwinch_handler(int sig) {
	getmaxyx(stdscr, row, col); 
	resize_term(row, col); 
	clear();
}

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

	signal(SIGWINCH, sigwinch_handler);
	if (argc < 3) {
        	fprintf(stderr,"usage: %s ip_addr port_number\n",argv[0]);
        	exit(1);
    	}	
	
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
		drawHorizontalLine(INFOBORDERROW);
		drawHorizontalLine(INPUTBORDERROW);
		
		int userRes = getstrnb(outBuf);//fgets(outBuf, USERNAMESIZE, stdin);

		move(INPUTMESSAGEROW, 0);
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
					move(INFOROW, 0);
					clrtoeol();
					printw("Error on Username send! Please try again!");
					refresh();
					continue;
				}

				break;
			}
			else {
				move(INFOROW, 0);
				clrtoeol();
				printw("Username is empty!");
   			}
				
		}	
		
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
		drawHorizontalLine(INFOBORDERROW);
		drawHorizontalLine(INPUTBORDERROW);


		int getStrRes = getstrnb(outBuf );
		
		move(INPUTMESSAGEROW, 0);
		clrtoeol();
		
		int messagLen  = strlen(outBuf);
		int spaceForMessage = col - inputMessageLen;
		//mvprintw(20 , 20, "Space for message %d", spaceForMessage);
		if(messagLen > spaceForMessage){
			char visible[INPUTMESSAGEROW];
			int diff = messagLen - spaceForMessage;
			memcpy(visible, outBuf + diff, messagLen - diff );
			printw("%s%s", inputMessage, visible);
		}
		else{
			printw("%s%s", inputMessage, outBuf);
		}

		if (getStrRes != -1) {

			len = strlen(outBuf);
			if(len != 0) {
				move(INFOROW, 0);
				clrtoeol();
				if (outBuf[len-1] == '\n') {
					outBuf[len-1] = '\0'; // Remove newline
					len--;
				}
				
				if(write(sfd, outBuf, len) != len){
					move(INFOROW, 0);
					clrtoeol();
					printw("Error on Message send! Please try again!");
					refresh();
					continue;
				}
					
				move(INPUTMESSAGEROW, inputMessageLen);
				clrtoeol();
				bzero(outBuf, OUTBUFSIZE);
				refresh();

			}
			else {
				move(INFOROW, 0);
				clrtoeol();
				printw("Message is empty!");
   			}
		}



		int n = read(sfd, inBuf, INBUFSIZE-1);
		if(n <= 0) {
		  	refresh();
		  	continue;
         }


		// Message is fine and will be sent

        inBuf[n] = '\0';  
		int numNewMessageRows = (( strlen(inBuf) + col - 1) / col ) ; 


		for (int i = UPPERTEXTMAX; i<=LOWERTEXTBOUND ; i++){			
			move(i, 0);
			inchnstr(lineCopyBuf, sizeof(lineCopyBuf));
			clrtoeol();
			for(int j = 0; j<col; j++){
				move(i - numNewMessageRows, j);
				char c = lineCopyBuf[j] & A_CHARTEXT;
				printw("%c", c);
			}
			bzero(lineCopyBuf, sizeof(lineCopyBuf));
		}
  
		mvprintw(LOWERTEXTBOUND + 1 - numNewMessageRows , 0, "%s", inBuf);
		
		move(INFOROW, 0);
		clrtoeol();
		
		refresh();
	
	}
	return 0;
}
