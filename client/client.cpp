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

int main(int argc, char* argv[])
{
    /* variable initialization */
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in client_addr;
    struct sockaddr_in udp_sin;
    struct sockaddr_in tcp_sin;
    char *host;
    char buf[MAX_LINE]; 
    int tcp_s, udp_s, ibytes, obytes, port_num;
    string username, password;

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

    /*build TCP address data structure*/
    bzero((char *)&tcp_sin, sizeof(tcp_sin));
    tcp_sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&tcp_sin.sin_addr, hp->h_length);
    tcp_sin.sin_port = htons(port_num);

    /*build UDP address data structure*/
    bzero((char *)&udp_sin, sizeof(udp_sin));
    udp_sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&udp_sin.sin_addr, hp->h_length);
    udp_sin.sin_port = htons(port_num);

    /*active open*/
    if((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("myfrm: tcp socket");
        exit(1);
    }
    if((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("myfrm: udp socket");
        exit(1);
    }
   
    printf("Welcome to Client! To quit type \'Exit\'\n");

    /*TCP connect on port*/
    if(connect(tcp_s,(struct sockaddr*)&tcp_sin,sizeof(tcp_sin))<0){
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
    //wait for username request
    socklen_t addr_len = sizeof(udp_sin);
    if((ibytes = recvfrom(udp_s,buf,sizeof(buf),0, (struct sockaddr *)&udp_sin,&addr_len)) == -1){
        perror("Recieve username client error");
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
    if((obytes=sendto(udp_s,username.c_str(),strlen(username.c_str()),0,(struct sockaddr *)&udp_sin,sizeof(struct sockaddr_in)))<0){
        perror("client sending username error\n");
        exit(1);
    }
    bzero((char*)&buf,sizeof(buf));
    //wait for password request
    if((ibytes = recvfrom(udp_s,buf,sizeof(buf),0, (struct sockaddr *)&udp_sin,&addr_len)) == -1){
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
    if((obytes=sendto(udp_s,password.c_str(),strlen(password.c_str()),0,(struct sockaddr *)&udp_sin,sizeof(struct sockaddr_in)))<0){
        perror("client sending password error\n");
        exit(1);
    }
    bzero((char*)&buf,sizeof(buf));
    //wait for acknowledgement
    if((ibytes = recvfrom(udp_s,buf,sizeof(buf),0, (struct sockaddr *)&udp_sin,&addr_len)) == -1){
        perror("Recieve acknowledgement client error");
        exit(1);
    }

    cout << "Please enter your desired operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT)" << endl;
    /*main loop: get and send lines of text*/
    while(fgets(buf, sizeof(buf),stdin)){
        if(strcmp(buf,"Exit") == 0){
            printf("Good Bye\n");
            break;
        }
        buf[MAX_LINE-1]='\0';
        ibytes = strlen(buf) + 1;
        if(send(udp_s,buf,ibytes,0) == -1){
            perror("client send error!");
            exit(1);
        }
        //handle operations
        //handle_request(buf, s);
        bzero((char*)&buf, sizeof(buf));
    }
    close(udp_s);
    close(tcp_s);
    exit(0);
}




