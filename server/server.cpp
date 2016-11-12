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
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 4096
#define TEST_PORT 41004

using namespace std;

void createBoard(int new_s);

int main(int argc, char *argv[]) {
	int port, key_len, addr_len, ret_len;
    int s, input_buf_len, i;
    struct sockaddr_in sin;
    struct sockaddr_in serveraddr; // server's addr
    struct sockaddr_in clientaddr; // client addr
    socklen_t addrlen = sizeof(clientaddr);            /* length of addresses */
    string password;
    char buf[MAX_LINE], ret_buf[MAX_LINE];
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
    /* bind socket */
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("udpserver: bind\n");
        exit(1);
    }
}

void createBoard(int new_s) {

    recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
    if (recvlen < 0)
        error("ERROR in recvfrom");


}