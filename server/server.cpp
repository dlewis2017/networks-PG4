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
#include <unordered_set> 

#define MAX_LINE 4096
#define TEST_PORT 41004

using namespace std;

unordered_set<string> fileNames;

void createBoard(int new_s);
void print_usage(); //prints usage to stdout if program invoked incorrectly


int main(int argc, char *argv[]) {
	int port;      // port to connect
    int udp_s;  // socket descriptor for udp
    int tcp_s;  // socket descriptor for tcp
    int input_buf_len;  // length of buffer received from client
    struct sockaddr_in udp_sin; // udp socket address
    struct sockaddr_in tcp_sin; // tcp socket address
    string password;    // password for user
    char buf[MAX_LINE]; // buffer to store and send messages
    if (argc != 3) {
        print_usage();
        exit(1);
    }
    port = atoi(argv[1]);
	password = argv[2];
    //* build UDP address data structure */  
    bzero((char *)&udp_sin, sizeof(udp_sin));   
    udp_sin.sin_family = AF_INET;
    udp_sin.sin_addr.s_addr = INADDR_ANY;
    udp_sin.sin_port = htons(port);
    //* build TCP address data structure */  
    bzero((char *)&tcp_sin, sizeof(tcp_sin));   
    tcp_sin.sin_family = AF_INET;
    tcp_sin.sin_addr.s_addr = INADDR_ANY;
    tcp_sin.sin_port = htons(port);
    /* setup passive UDP open */  
    if ((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("myfrmd: socket\n");
        exit(1);
    }
    /* bind UDP socket */
    if ((bind(udp_s, (struct sockaddr *)&udp_sin, sizeof(udp_sin))) < 0) {
        perror("myfrmd: bind\n");
        exit(1);
    }
    /* setup TCP open */
    if ((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("myfrmd: socket\n");
        exit(1);
    }
    /* bind TCP socket */
    if ((bind(tcp_s, (struct sockaddr *)&tcp_sin, sizeof(tcp_sin))) < 0) {
        perror("myfrmd: bind\n");
        exit(1);
    }


}

void print_usage() {
    cout << "myfrmd: requires 2 arguments (e.x. ./myfrmd 41004 mysecretpassword" << endl;
}

void createBoard(int new_s) {

    int recvlen = recvfrom(new_s, buf, MAX_LINE, 0, (struct sockaddr *)&clientaddr, &clientlen);
    if (recvlen < 0) {
        error("ERROR in recvfrom");
        exit(1);
    } 
    string boardName = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    int userlen = recvfrom(new_s, buf, MAX_LINE, 0, (struct sockaddr *)&clientaddr, &clientlen);
    if (userlen < 0) {
        error("ERROR in recvfrom");
        exit(1);
    }
    string userName = string(buf, userlen);
    memset(buf, '\0', sizeof(buf));

    if (filenames.count(boardName) == 0) {
        ofstream messageBoard;
        messageBoard.open(boardName);
        messageBoard << userName;
        messageBoard.close(); 

        fileNames.insert(boardName);
        sprintf(buf,"Board creation confirmation: 1");
    } else {
        sprintf(buf,"Board creation confirmation: 0");

    }

    int recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
    if (recvlen < 0)
        error("ERROR in recvfrom");
    memset(buf, '\0', sizeof(buf));

    int n = sendto(new_s, buf, bufLength, 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0)
        error("ERROR in sendto");
    memset(buf, '\0', sizeof(buf));

}
