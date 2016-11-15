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
#include <functional>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream> 
#include <map>
#include <vector>
#include <sstream>
#define MAX_LINE 4096
#define TEST_PORT 41004

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::map;
using std::fstream;
using std::hash;
using std::to_string;
using std::vector;
using std::stringstream;

//unordered_set<string> fileNames;    // list of filenames which are the message boards
map<string,string> user_table;
map<string,string> active_boards;
string currentUser;


//void createBoard(int new_s);
void print_usage(); //prints usage to stdout if program invoked incorrectly
int handle_request(char buf[MAX_LINE], int tcp_s, int udp_s, struct sockaddr_in udp_sin);
void createBoard(int new_s, struct sockaddr_in udp_cin);
void create_message(int s, struct sockaddr_in sin);
void edt_operation(int s, struct sockaddr_in sin);
void dst_operation(int s, struct sockaddr_in sin);
void lis_operation(int s, struct sockaddr_in sin);
void dlt_operation(int s, struct sockaddr_in sin);

void error(string msg) {
  perror(msg.c_str());
  exit(1);
}

int main(int argc, char *argv[]) {
    int server_running = 1; //flags for while conditions
    int new_client = 1;
    int port, udp_s, tcp_s, optval, user_len, pwd_len, n, client_active, op_len, outcome, admin_pwd_len;
    int tcp_comm_s; //to be used as new socket for tcp connection
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
    if ((udp_s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) error("myfrmd: socket\n");

    /* setup TCP open */
    if ((tcp_s = socket(PF_INET, SOCK_STREAM, 0)) < 0) error("myfrmd: socket\n");

    /*set tcp socket option*/
    if((setsockopt(tcp_s,SOL_SOCKET,SO_REUSEADDR, (char*)&optval,sizeof(int)))<0) error("myftpd:setscokt\n");

    /* bind UDP socket */
    if ((bind(udp_s, (struct sockaddr *)&sin, sizeof(sin))) < 0) error("myfrmd: bind\n");

    /* bind TCP socket */
    if ((bind(tcp_s, (struct sockaddr *)&sin, sizeof(sin))) < 0) error("myfrmd: bind\n");

    /*listen for client connection*/
    if((listen(tcp_s,1))<0) error("myftpd:listen failed");

    printf("Welcome to TCP Server. \n");

    //udp_cinLength = sizeof(sin);

    while (server_running) {
        if(new_client){
            if((tcp_comm_s = accept(tcp_s,(struct sockaddr*)&sin,&len))<0) error("myfrmd: error in accept");
            cout << "Connected to client" << endl;
            new_client = 0;
        }

        client_active = 0;
        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "username");
        if( send(tcp_comm_s, buf, strlen(buf), 0) < 0) error("ERROR in sendto\n");

        // receive a datagram from a client
        memset(buf, '\0', sizeof(buf));
        if((user_len = recv(tcp_comm_s, buf, MAX_LINE, 0)) < 0) error("ERROR in sendto\n");

        string username = string(buf, user_len);
        //check for username in user_table
        memset(buf, '\0', sizeof(buf));
        sprintf(buf, "password");
        if( send(tcp_comm_s, buf, strlen(buf), 0) < 0) error("ERROR in sendto\n");

        // receive a datagram from a client
        memset(buf, '\0', sizeof(buf));
        if((pwd_len = recv(tcp_comm_s, buf, MAX_LINE, 0)) < 0) error("ERROR in sendto\n");

        string pwd = string(buf, pwd_len);
        memset(buf, '\0', sizeof(buf));

        //if username in user table check for password and if correct send ack, else add new username with password
        if (user_table.count(username) == 0) 
            user_table[username] = pwd;
        if (user_table[username] == pwd) {
            client_active = 1;
            currentUser = username;
            sprintf(buf,"success");
        }else
            sprintf(buf,"failed"); 
        if( send(tcp_comm_s, buf, strlen(buf),0) < 0) error("Error in sending success message after usr/pwd\n");
        
        //handles client operations 
        while (client_active) {
            new_client = 1;
            //wait for operation
            memset(buf, '\0', sizeof(buf));
            if((op_len = recv(tcp_comm_s, buf, MAX_LINE, 0)) < 0) error("Server error in receiving operation\n");
            string operation = string(buf,op_len);
            //if outcome is 0, return to outer while loop and wait for new client
            //if outcome is 1, continue inner while loop for operations
            if((outcome = handle_request(buf,tcp_s,udp_s,sin)) == 0){
                client_active = 0;
                break;
            }else if (outcome == -1){
                //if outcome is -1, compare admin passwords, if the same, shut down, if not, wait for new operation
                memset(buf, '\0', sizeof(buf));
                if((admin_pwd_len = recv(tcp_comm_s, buf, MAX_LINE, 0)) < 0) error("Sever error in receiving admin password from client\n");
                string admin_pwd_client = string(buf, admin_pwd_len);
                if (admin_pwd == admin_pwd_client){
                    memset(buf,'\0',sizeof(buf));
                    sprintf(buf,"correct");
                    if( send(tcp_comm_s, buf, strlen(buf),0) < 0) error("Server error in sending confirmation\n");
                    //delete_everything();
                    close(tcp_s);
                    close(udp_s);
                    exit(0); 
                } else {
                    memset(buf,'\0',sizeof(buf));
                    sprintf(buf,"incorrect");
                    if( send(tcp_comm_s, buf, strlen(buf),0) < 0) error("Server error in sending confirmation\n");
                    continue; 
                }
            }else continue;
            
        }
    }
}

void print_usage() {
    cout << "myfrmd: requires 2 arguments (e.x. ./myfrmd 41004 mysecretpassword" << endl;
}

/*Wrapper function to handle oepration requests*/
int handle_request(char buf[MAX_LINE], int tcp_s, int udp_s, struct sockaddr_in sin) {
    if (strncmp(buf, "CRT", 3) == 0) {
        createBoard(udp_s, sin); 
    } else if (strncmp(buf, "LIS", 3) == 0) {
        lis_operation(udp_s, sin);
    } else if (strncmp(buf, "MSG", 3) == 0) {
        create_message(udp_s, sin);
    } else if (strncmp(buf, "DLT", 3) == 0) {
        dlt_operation(udp_s, sin);
    } else if (strncmp(buf, "RDB", 3) == 0) {

    } else if (strncmp(buf, "EDT", 3) == 0) {
        edt_operation(udp_s, sin);
    } else if (strncmp(buf, "APN", 3) == 0) {

    } else if (strncmp(buf, "DWN", 3) == 0) {

    } else if (strncmp(buf, "DST", 3) == 0) {
        dst_operation(udp_s,sin);
        return 1;
    } else if (strncmp(buf, "XIT", 3) == 0) {
        return 0;
    } else if (strncmp(buf, "SHT", 3) == 0) {
        return -1;
    } 

    return 1;

}

void createBoard(int s, struct sockaddr_in sin) {

    int recvlen, userlen, sendlength, n;
    char buf[MAX_LINE];
    socklen_t len = sizeof(sin);

    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Sever error in receving board name\n"); 
    string boardName = string(buf, recvlen);
//    boardName = boardName+".txt";
    memset(buf, '\0', sizeof(buf));

    //create file, write first line of file to be the user that create the file
    if (active_boards.count(boardName) == 0) {
        fstream outputFile;
        outputFile.open(boardName.c_str(), fstream::in | fstream::out | fstream::app);
        outputFile << currentUser;
        outputFile << "\n";
        outputFile.close(); 
        active_boards[boardName] = currentUser;
        sprintf(buf,"success");
    } else {
        sprintf(buf,"failure");

    }
    cout << buf << endl;
    if((sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending confirmation\n");
    memset(buf, '\0', sizeof(buf));

}

void create_message(int s, struct sockaddr_in sin) {
    hash<string> hash_fn;//hash function for string ids
    socklen_t len = sizeof(sin);
    char buf[MAX_LINE], ret_buf[MAX_LINE];
    int recvlen;

    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving board name\n"); 
    string board_name = string(buf, recvlen);
//    board_name += ".txt";
    memset(buf, '\0', sizeof(buf));

    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving message\n"); 
    string message = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));
    if (active_boards.count(board_name) == 0) {
        string fail_msg = "failed";
        int fail_msg_len = fail_msg.length();
        sprintf(ret_buf, "failed");
        if((sendto(s, fail_msg.c_str(), fail_msg_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending failure status\n");
        return;
    }
    /* compute hash, write message to board, return response with hash string */
    size_t hash = hash_fn(message);
    stringstream ss;
    ss << hash;
    string hash_str = ss.str();
    string message_for_board = hash_str + "|" + currentUser + "|" + message + "\n";

    fstream outputFile;
    outputFile.open(board_name.c_str(), fstream::in | fstream::out | fstream::app);
    outputFile << message_for_board;
    outputFile.close();

    int hash_str_len = hash_str.length();
    sprintf(ret_buf, hash_str.c_str());
    if((sendto(s, ret_buf, hash_str_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending hash\n");
}

/* delete a message given message ID and board */
void dlt_operation(int s, struct sockaddr_in sin) {
    vector<string> content_to_copy;
    hash<string> hash_fn;//hash function for string ids
    socklen_t len = sizeof(sin);
    char buf[MAX_LINE], ret_buf[MAX_LINE];
    int recvlen;

    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving board name\n"); 
    string board_name = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving message ID\n"); 
    string message_id = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));
    if (active_boards.count(board_name) == 0) {
        string fail_msg = "failed";
        int fail_msg_len = fail_msg.length();
        if((sendto(s, fail_msg.c_str(), fail_msg_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending failure status\n");
        return;
    }
    /* find message ID in board, delete line */
    string delimiter = "|";
    size_t pos = 0;
    string token;
    fstream in_file;
    in_file.open(board_name, fstream::in);
    for (string line; getline(in_file, line);) {
        pos = 0;
        if ((pos = line.find(delimiter)) != string::npos) {
            token = line.substr(0, pos);
            if (token != message_id) {
                content_to_copy.push_back(line);
            }
        }
        else {
            content_to_copy.push_back(line);
        }
    }
    in_file.close();
    /* delete file, copy contents into new file with same name */
    remove(board_name.c_str());
    fstream outputFile;
    outputFile.open(board_name.c_str(), fstream::in | fstream::out | fstream::app);
    for (int i=0; i < content_to_copy.size(); i++) {
        outputFile << content_to_copy[i];
        outputFile << "\n";
    }
    outputFile.close();
    string success_msg = "success";
    int success_msg_len = success_msg.length();
    if((sendto(s, success_msg.c_str(), success_msg_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending failure status\n");
}

/* edit a message given message ID and board */
void edt_operation(int s, struct sockaddr_in sin) {
    vector<string> content_to_copy;
    hash<string> hash_fn;//hash function for string ids
    socklen_t len = sizeof(sin);
    char buf[MAX_LINE], ret_buf[MAX_LINE];
    int recvlen;

    // receive the name of the board
    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving board name\n"); 
    string board_name = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    // receive the message ID
    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving message ID\n"); 
    string message_id = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    // receive the new message
    if((recvlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &len)) < 0) error("Server error in receving message ID\n"); 
    string new_message = string(buf, recvlen);
    memset(buf, '\0', sizeof(buf));

    if (active_boards.count(board_name) == 0) {
        string fail_msg = "failed";
        int fail_msg_len = fail_msg.length();
        if((sendto(s, fail_msg.c_str(), fail_msg_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending failure status\n");
        return;
    }

    /* compute hash and message to write to board */
    size_t hash = hash_fn(new_message);
    stringstream ss;
    ss << hash;
    string hash_str = ss.str();
    string message_for_board = hash_str + "|" + currentUser + "|" + new_message + "\n";

    /* find message ID in board, delete line */
    string delimiter = "|";
    size_t pos = 0;
    string token, originalUser;
    fstream in_file;
    in_file.open(board_name, fstream::in);
    for (string line; getline(in_file, line);) {
        pos = 0;
        if ((pos = line.find(delimiter)) != string::npos) {
            token = line.substr(0, pos);
            if (token != message_id) {
                content_to_copy.push_back(line);
            } else {
                content_to_copy.push_back(message_for_board);
            }
        }
        else {
            content_to_copy.push_back(line);
        }
    }
    in_file.close();
    /* delete file, copy contents into new file with same name */
    remove(board_name.c_str());
    fstream outputFile;
    outputFile.open(board_name.c_str(), fstream::in | fstream::out | fstream::app);
    for (int i=0; i < content_to_copy.size(); i++) {
        outputFile << content_to_copy[i];
        outputFile << "\n";
    }
    outputFile.close();
    string success_msg = "success";
    int success_msg_len = success_msg.length();
    if((sendto(s, success_msg.c_str(), success_msg_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending failure status\n");
}

/* list the names of the message boards */
void lis_operation(int s, struct sockaddr_in sin) {
    socklen_t len = sizeof(sin);
    string boardNames = "";

    for (auto it = active_boards.begin(); it != active_boards.end(); it++) {
        boardNames += it->first + '\n';
    }

    int boardNames_len = boardNames.length();
    if((sendto(s, boardNames.c_str(), boardNames_len, 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending boardNames\n");

}

/*receive board name and destroy(delete) it from all of the boards*/

void dst_operation(int s, struct sockaddr_in sin){
    hash<string> hash_fn;
    string board_name, delete_cmd;
    char buf[MAX_LINE];
    int buf_len;
    socklen_t len = sizeof(sin);

    //receive name of board
    if((buf_len = recvfrom(s,buf,sizeof(buf),0,(struct sockaddr*)&sin, &len)) < 0) error("Server error in receiving name of board in dst\n");
    board_name = string(buf,buf_len);
    delete_cmd = "exec rm -r ./" + board_name;
    memset(buf, '\0', sizeof(buf));
    //check if board exists, if it does, check if current user created board and delete it
    if (active_boards.find(board_name) == active_boards.end()) sprintf(buf,"Board not found\n");
    else{
        if (active_boards[board_name] == currentUser){
            active_boards.erase(board_name);
            system(delete_cmd.c_str());
            sprintf(buf,"Board has been deleted");
        }else{
            sprintf(buf,"Nacho board\n");
        }
    }
/*
    ifstream infile(board_name);
    if (infile.good()){
        getline(infile, creator);
        cout << "creator is " << creator << endl;
    }else sprintf(buf,"Table does not exist");
    infile.close();
    if (currentUser == creator){
        //delete from hash table and from directory
        active_boards.erase(board_name);
        
    } else sprintf(buf,"Nacho table");
*/
    if((sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, len)) == -1) error("Server error in sending response to dst table\n");

    cout << "Waiting for command" << endl;    

}

