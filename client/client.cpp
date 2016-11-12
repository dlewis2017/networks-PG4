/*Chris Beaufils - cbeaufil, David Lewis - dlewis12, Jack Ryan - jryan13
  PG4
  client.c
  Client for connecting with username and password and running commands
*/

#include <sys/time.h>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>	
#include <string.h>	
#include <stdlib.h>	
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>
#include <netdb.h>		
#define	MAX_LINE 4096
using namespace std;

void pre_reqs(struct sockaddr_in sin, int udp_s, int tcp_s);
int handle_request(char buf[MAX_LINE], struct sockaddr_in sin, int tcp_s, int udp_s); 
void crt_operation(int s);

int main(int argc, char* argv[])
{
    /* variable initialization */
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in client_addr;
    struct sockaddr_in sin;
    struct sockaddr_in udp_sin;
    struct sockaddr_in tcp_sin;
    char *host;
    char buf[MAX_LINE]; 
    int ibytes, tcp_s, udp_s, port_num;


    if(argc != 3){
        fprintf(stderr,"myfrm: requires 2 arguments (./myfrm server_name port_num)\n");
        exit(1);
    }
    host = argv[1];
    port_num = atoi(argv[2]);
    
    /*translate host name into peer's IP address*/
    hp = gethostbyname(host);
    if(!hp){
        fprintf(stderr,"myftp:unkown host: %s\n", host);
        exit(1);
    }

    /*build TCP address data structure
    bzero((char *)&tcp_sin, sizeof(tcp_sin));
    tcp_sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&tcp_sin.sin_addr, hp->h_length);
    tcp_sin.sin_port = htons(port_num);

    build UDP address data structure
    bzero((char *)&udp_sin, sizeof(udp_sin));
    udp_sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&udp_sin.sin_addr, hp->h_length);
    udp_sin.sin_port = htons(port_num);
    */
    
    /*build address data structure*/
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port_num);

    /*active open*/
    if((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("myfrm: tcp socket");
        exit(1);
    }
    if((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("myfrm: udp socket");
        exit(1);
    }
   
    printf("Welcome to Client! To quit type \'XIT\'\n");

    /*TCP connect on port*/
    if(connect(tcp_s,(struct sockaddr*)&sin,sizeof(sin))<0){
        perror("simple-talk:connect failed");
        close(tcp_s);
        exit(1);
    }
    /*UDP doesn't need to connect on port*/
    /*if(connect(udp_s,(struct sockaddr*)&udp_sin,sizeof(udp_sin))<0){
        perror("simple-talk:connect failed");
        close(upd_s);
        exit(1);
    }*/

    cout << "Connected to server" << endl;
    pre_reqs(sin,udp_s,tcp_s);

    cout << "Please enter your desired operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT)" << endl;
    //main loop: get and send lines of text
    while(fgets(buf, sizeof(buf),stdin)){
        //send command to server
        buf[MAX_LINE-1]='\0';
        ibytes = strlen(buf) + 1;
        if(send(udp_s,buf,ibytes,0) == -1){
            perror("client operation send error!");
            exit(1);
        }
        //handle operations
        if( handle_request(buf, sin, udp_s, tcp_s) == 0) break;
        bzero((char*)&buf, sizeof(buf));
    }
    close(udp_s);
    close(tcp_s);
    exit(0);
}

void pre_reqs(struct sockaddr_in sin, int udp_s, int tcp_s){
    char buf[MAX_LINE];
    string username, password;
    int ibytes, obytes;

    //wait for username request
    if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
    	perror("Receive username client error!\n");
    	exit(1);
    }

    //check for username being sent
    if(strcmp(buf,"username")){
        cout << "Please enter your username: "; 
        cin >> username;
    } else {
        cout << "username not sent" << endl;
        exit(1);
    }
    //send username back
    if(send(tcp_s,username.c_str(),strlen(username.c_str()),0) == -1){
        perror("client sending username error\n");
        exit(1);
    }
    bzero((char*)&buf,sizeof(buf));
    //wait for password request
    if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
        perror("Recieve password client error");
        exit(1);
    }
    //check for password being sent
    if(strcmp(buf,"password")){
        cout << "Please enter the password: ";
        cin >> password;
    } else {
        cout << "password request not sent" << endl;
        exit(1);
    }
    //send password
    if(send(tcp_s,password.c_str(),strlen(password.c_str()),0) == -1){
        perror("client sending password error\n");
        exit(1);
    }
    bzero((char*)&buf,sizeof(buf));
    //wait for acknowledgement
    if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
        perror("Recieve acknowledgement client error");
        exit(1);
    }

    return;
}

/*Wrapper function to handle oepration requests*/
int handle_request(char buf[MAX_LINE], struct sockaddr_in sin, int tcp_s, int udp_s) {
    if (strncmp(buf, "CRT", 3) == 0) {
        crt_operation(udp_s);
        return 1;
    } else if (strncmp(buf, "LIS", 3) == 0) {
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
    }else{
        cout << "Wrong command" << endl;
        cout << "Please enter your desired operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT)" << endl;
        return 1;
    }

}

/*does something*/
void crt_operation(int s){
    

    cout << "Please enter your desired operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT)" << endl;

    return;
}

