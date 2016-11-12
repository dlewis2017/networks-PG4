/* Jack Ryan - jryan13, David Lewis - dlewis12, Chris Beaufils - cbeaufil
 * CSE 30264
 * 11/14/2016
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


#define MAX_LINE 4096
#define TEST_PORT 41004

using namespace std;

//unordered_set<string> fileNames;    // list of filenames which are the message boards

void createBoard(int new_s);
void print_usage(); //prints usage to stdout if program invoked incorrectly
int handle_request(char buf[MAX_LINE], int tcp_s, int udp_s, struct sockaddr_in udp_sin);

void error(string msg) {
  perror(msg.c_str());
  exit(1);
}

int main(int argc, char *argv[]) {
    int server_running = 1; //flags for while conditions
    int client_active = 0;
    int port, udp_s, tcp_s, optval, user_len, pwd_len, n;
    socklen_t len;    // size of udp message
    struct sockaddr_in sin; // udp server socket address
    char buf[MAX_LINE];
    string admin_pwd, pwd, user_name;

    if (argc != 3) {
        print_usage();
        exit(1);
    }
    port = atoi(argv[1]);
	admin_pwd = argv[2];

    //* build address data structure */  
    bzero((char *)&sin, sizeof(sin));   
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    /* setup passive UDP open */  
    if ((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("myfrmd: socket\n");
        exit(1);
    }

    /* setup TCP open */
    if ((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("myfrmd: socket\n");
        exit(1);
    }

    /*set tcp socket option*/
    if((setsockopt(tcp_s,SOL_SOCKET,SO_REUSEADDR, (char*)&optval,sizeof(int)))<0) error("myftpd:setscokt\n");

    /* bind UDP socket */
    if ((bind(udp_s, (struct sockaddr *)&sin, sizeof(sin))) < 0) error("myfrmd: bind\n");

    /* bind TCP socket */
    if ((bind(tcp_s, (struct sockaddr *)&sin, sizeof(sin))) < 0) error("myfrmd: bind\n");



    if((listen(tcp_s,1))<0){
        perror("myftpd:listen failed");
        exit(1);
    } else {
        printf("Welcome to TCP Server. \n");
    }

    udp_cinLength = sizeof(udp_cin);


    socklen_t tcp_len;
    int tcp_comm_s; //communication socket to be used in comm with the client
    if((tcp_comm_s = accept(tcp_s,(struct sockaddr*)&tcp_sin,&tcp_len))<0){
        error("myfrmd: error in accept");
    }
    else {
        cout << "Connected to client" << endl;
    }

    while (server_running) {

        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "username");
        n = sendto(tcp_comm_s, buf, strlen(buf), 0, (struct sockaddr *) &udp_cin, udp_cinLength);
        if (n < 0)
            error("ERROR in sendto");

        // receive a datagram from a client
        memset(buf, '\0', sizeof(buf));
        userlen = recvfrom(tcp_comm_s, buf, MAX_LINE, 0,(struct sockaddr *) &udp_sin, &udp_cinLength);
        if (n < 0)
            error("ERROR in sendto");

        string username = string(buf, userlen);
        cout << username << endl;
        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "password");
        n = sendto(tcp_comm_s, buf, strlen(buf), 0, (struct sockaddr *) &udp_cin, udp_cinLength);
        if (n < 0)
            error("ERROR in sendto");

        // receive a datagram from a client
        memset(buf, '\0', sizeof(buf));
        passwordlen = recvfrom(tcp_comm_s, buf, MAX_LINE, 0,(struct sockaddr *) &udp_sin, &udp_cinLength);
        if (n < 0)
            error("ERROR in sendto");

        string password = string(buf, passwordlen);
        cout << password << endl;
        //if password valid, set client_active = 1 and jump into while loop
        client_active = 1;
        while (client_active) {
        }
    }


}

void print_usage() {
    cout << "myfrmd: requires 2 arguments (e.x. ./myfrmd 41004 mysecretpassword" << endl;
}

/*Wrapper function to handle oepration requests*/
int handle_request(char buf[MAX_LINE], int tcp_s, int udp_s, struct sockaddr_in udp_sin) {
    if (strncmp(buf, "LIS", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "MSG", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "DLT", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "RDB", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "EDT", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "APN", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "DWN", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "DST", 3) == 0) {
        return 1;
    } else if (strncmp(buf, "XIT", 3) == 0) {
        return 0;
    } else if (strncmp(buf, "SHT", 3) == 0) {
        return 1;
    } 

}
/*
void createBoard(int new_s, struct sockaddr_in udp_cin) {

    int recvlen, userlen, sendlength, n;
    char buf[MAX_LINE];
    socklen_t udp_cinLength = sizeof(udp_cin);


    recvlen = recvfrom(new_s, buf, MAX_LINE, 0, (struct sockaddr *)&udp_cin, &udp_cinLength);
    if (recvlen < 0) {
        error("ERROR in recvfrom");
    } 
    string boardName = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    userlen = recvfrom(new_s, buf, MAX_LINE, 0, (struct sockaddr *)&udp_cin, &udp_cinLength);
    if (userlen < 0) {
        error("ERROR in recvfrom");
    }
    string userName = string(buf, userlen);
    memset(buf, '\0', sizeof(buf));

    if (fileNames.count(boardName) == 0) {
        ofstream messageBoard;
        messageBoard.open(boardName);
        messageBoard << userName;
        messageBoard.close(); 

        fileNames.insert(boardName);
        sprintf(buf,"Board creation confirmation: 1");
    } else {
        sprintf(buf,"Board creation confirmation: 0");

    }

    recvlen = recvfrom(new_s, buf, MAX_LINE, 0, (struct sockaddr *)&udp_cin, &udp_cinLength);
    if (recvlen < 0)
        error("ERROR in recvfrom");
    memset(buf, '\0', sizeof(buf));

    n = sendto(new_s, buf, MAX_LINE, 0, (struct sockaddr *) &udp_cin, udp_cinLength);
    if (n < 0)
        error("ERROR in sendto");
    memset(buf, '\0', sizeof(buf));

}*/
