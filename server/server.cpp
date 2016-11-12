/* Jack Ryan
 * jryan13
 * CSE 30264
 * 9/23/2016
 */

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream> 
#include <unordered_set.h> 

#define MAX_LINE 4096
#define TEST_PORT 41004

using namespace std;

unordered_set<string> fileNames;

void createBoard(int new_s);
void print_usage(); //prints usage to stdout if program invoked incorrectly


int main(int argc, char *argv[]) {
	int port, key_len, addr_len, ret_len;
    int s, input_buf_len, i;
    struct sockaddr_in sin;
    struct sockaddr_in serveraddr; // server's addr
    struct sockaddr_in clientaddr; // client addr
    socklen_t addrlen = sizeof(clientaddr);            /* length of addresses */
    string password;
    char buf[MAX_LINE], ret_buf[MAX_LINE];
    if (argc != 3) {
        print_usage();
        exit(1);
    }
    port = atoi(argv[1]);
	password = argv[2];
    //* build address data structure */  
    bzero((char *)&sin, sizeof(sin));   
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    /* setup passive open */  
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("udpserver: socket");
        exit(1);
    }
}

void print_usage() {
    cout << "myfrmd: requires 2 arguments (e.x. ./myfrmd 41004 mysecretpassword" << endl;
}

void createBoard(int new_s) {

    int recvlen = recvfrom(fd, buf, MAX_LINE, 0, (struct sockaddr *)&clientaddr, &addrlen);
    if (recvlen < 0) {
        error("ERROR in recvfrom");
        exit(1);
    } 
    string boardName = string(buf, recvlen);

    int userlen = recvfrom(fd, buf, MAX_LINE, 0, (struct sockaddr *)&clientaddr, &addrlen);
    if (userlen < 0) {
        error("ERROR in recvfrom");
        exit(1);
    }
    string userName = string(buf, userlen);

    if (filenames.count(boardName) == 0) {
        ofstream messageBoard;
        messageBoard.open(boardName);
        messageBoard << userName;
        messageBoard.close(); 

        fileNames.insert(boardName);
        buf = 'Board creation confirmation: 1';
    } else {
        buf = 'Board creation confirmation: 0';
    }

    int n = sendto(sockfd, buf, bufLength, 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0)
        error("ERROR in sendto");

}