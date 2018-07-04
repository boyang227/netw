#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "netw.h"

char usage[] = "usage: %s -ltu -i ip -p port\n";

void createTcpClient(char *ipaddr, int port) {
    printf("tcp client to %s:%d\n", ipaddr, port);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ipaddr, &addr.sin_addr);

	if (-1 == connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {
		perror("connect: ");
		return;
	} 

	char hello[] = "hello world!";
	send(sockfd, hello, sizeof(hello), 0);

	char input[80];
	recv(sockfd, input, sizeof(input), 0);

	printf("recv:%s\n", input);

}

void createTcpServer(char *ipaddr, int port) {
    printf("tcp server to %s:%d\n", ipaddr, port);

	int sfd, addl, nsock;
	//address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ipaddr, &addr.sin_addr);
	
	addl = sizeof(addr);
	
	//socket
	//int socket(int domain, int type, int protocol);
	if(0 == (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket: ");
		return;
	};

	//sockopt
	//int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
	int rc = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	if (-1 == rc) {
		perror("setsockopt");
		return;
	}


	//bind
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (-1 == bind(sfd, (struct sockaddr *)&addr, (socklen_t)addl)) {
		perror("bind: ");
		return;
	}

	//listen
	//int listen(int sockfd, int backlog);
	listen(sfd, 3);
	
	//accept
	//int new_socket= accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	nsock = accept(sfd, (struct sockaddr *)&addr, (socklen_t*)&addl);
	if (-1 == nsock) {
		perror("accept: ");
		return;
	}
	char buf[] = "hello world!";
	char input[80];

	recv(nsock , input, sizeof(input), 0);
	printf("recv:%s\n", input);

	send(nsock , input, sizeof(input) , 0);
}

void createUdpClient(char *ipaddr, int port) {
    printf("udp client to %s:%d\n", ipaddr, port);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ipaddr, &addr.sin_addr);

	char hi[] = "hello world!";
	sendto(sockfd, hi, sizeof(hi), 0, (struct sockaddr *)&addr, sizeof(addr));

	char input[80];
	recvfrom(sockfd, input, sizeof(input), 0, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr));

	printf("recv:%s\n", input);
}

void createUdpServer(char *ipaddr, int port) {
    printf("udp server to %s:%d\n", ipaddr, port);

	int sfd, addl, caddl, l;
	//address
	struct sockaddr_in addr, caddr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ipaddr, &addr.sin_addr);
	
	addl = sizeof(addr);
	caddl = sizeof(caddr);
	
	//socket
	//int socket(int domain, int type, int protocol);
	if(0 == (sfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("socket: ");
		return;
	};

	//sockopt
	//int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

	//bind
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (-1 == bind(sfd, (struct sockaddr *)&addr, (socklen_t)addl)) {
		perror("bind: ");
		return;
	}
	
	//recvfrom
	//int new_socket= accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	
	char buf[] = "hello world!";
	char input[80];

	l = recvfrom(sfd, input, sizeof(input), MSG_WAITALL, (struct sockaddr *)&caddr, &caddl);
	input[l] = '\0';	
	printf("recv: %s\n", input);

	sendto(sfd , input, sizeof(input), MSG_CONFIRM, (struct sockaddr *)&caddr, caddl);
}

int main (int argc, char **argv)
{
    typedef enum {CLIENT, SERVER} Mode;
    typedef enum {TCP, UDP} Type;

    Mode mode = CLIENT; //default mode is CLIENT
    Type type = TCP;    //default type is TCP
    char *ipaddr = NULL;
    int port = 0;
    int c;

    while ((c = getopt (argc, argv, "ltui:p:")) != -1)
        switch (c)
        {
            case 'l':
                mode = SERVER;
                break;
            case 't':
                type = TCP;
                break;
            case 'u':
                type = UDP;
                break;
            case 'i':
                ipaddr = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                printf(usage, argv[0]);
                exit(-1);
        }

    if (optind < argc || NULL == ipaddr || 0 == port) {
        printf(usage, argv[0]);
        exit(-1);
    }
        
    if (CLIENT == mode && TCP == type)
        createTcpClient(ipaddr, port);
    else if (SERVER == mode && TCP == type)
        createTcpServer(ipaddr, port);
    else if (CLIENT == mode && UDP == type)
        createUdpClient(ipaddr, port);
    else if (SERVER == mode && UDP == type)
        createUdpServer(ipaddr, port);
    else {
        printf(usage, argv[0]);
        exit(-1);
    }

    return 0;
}
