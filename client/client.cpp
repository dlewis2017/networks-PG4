/*Chris Beaufils - cbeaufil, David Lewis - dlewis12, Jack Ryan - jryan13
  PG4
  client.c
  Client for connecting with username and password and running commands
*/
//TODO:SHT should delete everything
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

int pre_reqs(struct sockaddr_in sin, int udp_s, int tcp_s);
int handle_request(char buf[MAX_LINE], struct sockaddr_in sin, int tcp_s, int udp_s); 
void crt_operation(int s, struct sockaddr_in sin);
void msg_operation(int s, struct sockaddr_in sin);
void dst_operation(int s, struct sockaddr_in sin);
void dlt_operation(int s, struct sockaddr_in sin);
void lis_operation(int s, struct sockaddr_in sin);
void edt_operation(int s, struct sockaddr_in sin);
//void dwn_operation(int s);

int sht_operation(int s);
void error(string msg){
    perror(msg.c_str());
    exit(1);
}
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
    
    /*build address data structure*/
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr,(char*)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port_num);

    /*active open*/
    if((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0) error("myfrm: tcp socket");
    if((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) error("myfrm: udp socket");
   
    printf("Welcome to Client! To quit type \'XIT\'\n");

    /*TCP connect on port*/
    if(connect(tcp_s,(struct sockaddr*)&sin,sizeof(sin))<0){
        perror("simple-talk:connect failed");
        close(tcp_s);
        exit(1);
    }

    cout << "Connected to server" << endl;
    if(!pre_reqs(sin,udp_s,tcp_s)) error("Problem with username and password. Goodbye");


    //main loop: get and send lines of text
    while(fgets(buf, sizeof(buf),stdin)){
        cout << "Please enter your desired operation (CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT)" << endl;
        //send command to server
        //buf[MAX_LINE-1]='\0';
        ibytes = strlen(buf) + 1;
        if(send(tcp_s,buf,ibytes,0) == -1) error("client operation send error!");
        //handle operations
        if( handle_request(buf, sin, tcp_s, udp_s) < 1) break;
        bzero((char*)&buf, sizeof(buf));
    }
    close(udp_s);
    close(tcp_s);
    exit(0);
}

int pre_reqs(struct sockaddr_in sin, int udp_s, int tcp_s){
    char buf[MAX_LINE];
    string username, password;
    int ibytes, obytes;
    while (1) {
        //wait for username request
        if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
            perror("Receive username client error!\n");
            return -1;
        }
        string server_uname_req = string(buf, ibytes);
        //check for username being sent
        if(server_uname_req == "username"){
            cout << "Please enter your username: ";
        } else {
            cout << "username not sent" << endl;
            return -1;
        }
        cin >> username;
        //send username back
        if(send(tcp_s,username.c_str(),strlen(username.c_str()),0) == -1){
            perror("client sending username error\n");
            return -1;
        }
        bzero((char*)&buf,sizeof(buf));
        //wait for password request
        if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
            perror("Recieve password client error");
            return -1;
        }
        //check for password being sent
        string server_pw_req = string(buf, ibytes);
        if(server_pw_req == "password"){
            cout << "Please enter the password: ";
        } else {
            cout << "password request not sent" << endl;
            return -1;
        }
        cin >> password;
        //send password
        if(send(tcp_s,password.c_str(),strlen(password.c_str()),0) == -1){
            perror("client sending password error\n");
            return -1;
        }
        bzero((char*)&buf,sizeof(buf));
        //wait for acknowledgement
        if((ibytes = recv(tcp_s,buf,sizeof(buf),0)) == -1){
            perror("Recieve acknowledgement client error");
            return -1;
        }
        string status = string(buf, ibytes);
        if (status == "success") break;
        cout << "Invalid credentials, please try again" << endl;
    }
    return 1;
}

/*Wrapper function to handle oepration requests*/
int handle_request(char buf[MAX_LINE], struct sockaddr_in sin, int tcp_s, int udp_s) {
    if (strncmp(buf, "CRT", 3) == 0) {
        crt_operation(udp_s,sin);
    } else if (strncmp(buf, "LIS", 3) == 0) {
       lis_operation(udp_s, sin);
    } else if (strncmp(buf, "MSG", 3) == 0) {
        msg_operation(udp_s, sin);
    } else if (strncmp(buf, "DLT", 3) == 0) {
        dlt_operation(udp_s, sin);
    } else if (strncmp(buf, "RDB", 3) == 0) {
    } else if (strncmp(buf, "EDT", 3) == 0) {
        edt_operation(udp_s, sin);
    } else if (strncmp(buf, "APN", 3) == 0) {
    } else if (strncmp(buf, "DWN", 3) == 0) {
        // dwn_operation(tcp_s);
    } else if (strncmp(buf, "DST", 3) == 0) {
        dst_operation(udp_s,sin);
    } else if (strncmp(buf, "XIT", 3) == 0) {
        return 0;
    } else if (strncmp(buf, "SHT", 3) == 0) {
        return sht_operation(tcp_s);
    }else{
        cout << "Wrong command" << endl;
        return 1;
    }

    return 1;    

}

/*send crt operation to server, send name of new board, created board returns success or failure and prompts*/
void crt_operation(int s, struct sockaddr_in sin){
    char buf[MAX_LINE];
    string board_name, result;    
    int buf_len;
    socklen_t addr_len = sizeof(sin);     

    //ask for name of board to be created
    cout << "What is the name of the new board to be created?" << endl;
    cin >> board_name;
    if(sendto(s,board_name.c_str(),strlen(board_name.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending board name\n");
    //receive confirmation and print results
    if((buf_len = recvfrom(s,buf,sizeof(buf),0, (struct sockaddr *)&sin,&addr_len)) < 0) error("Client error in receiving confirmation\n");
    result = string(buf,buf_len);
    cout << "Creation of board was a " << result << endl;

    return;
}

/*user asks for password, if same as server admin, server shuts down and so does client
if password is different, server does not shut down and client is prompted for operation while server waits*/
int sht_operation(int s){
    char buf[MAX_LINE];
    string pwd, admin;
    int buf_len, ibytes;
    //ask for password
    cout << "Please enter the password: ";
    cin >> pwd;
    //send password for admin
    if(send(s,pwd.c_str(),strlen(pwd.c_str()),0) == -1) error("Client error in sending admin password");
    //receive correct or not from server
    if((buf_len = recv(s,buf,sizeof(buf),0)) == -1) error("client error in receiving admin password confirmation");
    //if admin return -1, if not return 1
    admin = string(buf, buf_len);
    if (admin == "correct") return 0;
    else {
        cout << "Incorrect admin password" << endl;
        return 1;
    }
}

/* on MSG, send name of board and message to server */
void msg_operation(int s, struct sockaddr_in sin) {
    string board_name, message;
    socklen_t addr_len = sizeof(sin);     
    char buf[MAX_LINE];
    int buf_len;
    cout << "Enter the name of the board to leave a message on: ";
    cin >> board_name;
    if (sendto(s,board_name.c_str(),strlen(board_name.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending board name\n");
    cout << "Enter the message to send: ";
    cin >> message;
    if (sendto(s,message.c_str(),strlen(message.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending message\n");
    if ((buf_len = recvfrom(s,buf,sizeof(buf),0, (struct sockaddr *)&sin,&addr_len)) < 0) error("Client error in receiving message acknowledgement\n");
    string result = string(buf, buf_len);
    memset(buf, '\0', sizeof(buf));
    if (result == "failed") {
        cout << "Failed: " << board_name << " does not exist on server" << endl;
        return;
    }
    cout << "Success: your message (identification number: " << result << ") has been posted to " << board_name << endl;
}

void lis_operation(int s, struct sockaddr_in sin) {
	int recv_len;
    socklen_t addr_len = sizeof(sin); 
	char buf[MAX_LINE];

	if (recv_len = recvfrom(s,buf,sizeof(buf),0, (struct sockaddr *)&sin,&addr_len) < 0) error("Client error in receiving board listing\n");
	string boardListing = string(buf);

	cout << boardListing;
}

/* on DLT, delete message and send response */
void dlt_operation(int s, struct sockaddr_in sin) {
    string board_name, message_id;
    socklen_t addr_len = sizeof(sin);     
    char buf[MAX_LINE];
    int buf_len;
    cout << "Enter the name of the board to delete a message from: ";
    cin >> board_name;
    if (sendto(s,board_name.c_str(),strlen(board_name.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending board name\n");
    cout << "Enter the message ID to delete: ";
    cin >> message_id;
    if (sendto(s,message_id.c_str(),strlen(message_id.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending message\n");
    if ((buf_len = recvfrom(s,buf,sizeof(buf),0, (struct sockaddr *)&sin,&addr_len)) < 0) error("Client error in receiving message acknowledgement\n");
    string result = string(buf, buf_len);
    memset(buf, '\0', sizeof(buf));
    if (result == "failed") {
        cout << "Failed: " << board_name << " does not exist on server" << endl;
        return;
    }
    cout << "Success: your message (identification number: " << message_id << ") has been deleted from " << board_name << endl;
}

/* on edt, edit message in a given board */
void edt_operation(int s, struct sockaddr_in sin) {
    string board_name, message_id, new_message;
    socklen_t addr_len = sizeof(sin);     
    char buf[MAX_LINE];
    int buf_len;
    cout << "Enter the name of the board to edit a message from: ";
    cin >> board_name;
    if (sendto(s,board_name.c_str(),strlen(board_name.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending board name\n");
    cout << "Enter the message ID to be edited: ";
    cin >> message_id;
    if (sendto(s,message_id.c_str(),strlen(message_id.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending message\n");
    cout << "Enter the new replacement message: ";
    cin >> new_message;
    if (sendto(s,new_message.c_str(),strlen(new_message.c_str()),0,(struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending message\n");
    if ((buf_len = recvfrom(s,buf,sizeof(buf),0, (struct sockaddr *)&sin,&addr_len)) < 0) error("Client error in receiving message acknowledgement\n");
    string result = string(buf, buf_len);
    memset(buf, '\0', sizeof(buf));
    if (result == "failed") {
        cout << "Failed: " << board_name << " does not exist on server" << endl;
        return;
    }
    cout << "Success: your message (identification number: " << message_id << ") has been deleted from " << board_name << endl;
}

/*Destroy a board (file)*/
void dst_operation(int s, struct sockaddr_in sin){
    string board_name, response;
    socklen_t addr_len = sizeof(sin);
    char buf[MAX_LINE];
    int buf_len;
   
    //ask for and then send board name to be destroyed 
    cout << "Please enter the name of the board you would like to destroy: " << endl;
    cin >> board_name;
    if(sendto(s,board_name.c_str(),strlen(board_name.c_str()),0,(struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) error("Client error in sending board name\n");
    //receive confirmation
    if ((buf_len = recvfrom(s,buf,sizeof(buf),0,(struct sockaddr*)&sin, &addr_len)) < 0) error("Client error in receiving dst operation acknowledgement\n"); 
    response = string(buf,buf_len);
    cout << "Message from server upon request to destroy board " << board_name << ": " << response << endl;
}

/*
/*Sends file in chunks to client
void dwn_operation(int s){
    string board_name;
    char buf[MAX_LINE];
    int buf_len;

    //ask for board to download
    cout << "Which board would you like to download?" << endl;
    cin >> board_name;
    if(send(s,board_name.c_str(),sizeof(board_name.c_str(),0)) == -1) error("Client error in sending board to download");



}
*/
