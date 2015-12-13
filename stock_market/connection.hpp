// connection.hpp
//
//last modified : 30/5/2013
// developed by : suman roy ( email.suman.roy@gmail.com )  
//
// Description:this header files contains functions mainly used for creating connections
//
//////////////////////////////////////////////////////////////////////

#ifndef CONNECTION_H
#define CONNECTION_H 
#include "connection.hpp"
#include "header.hpp"
#include <arpa/inet.h>

int 
create_client(char *host_name,int port){
	int sock_fd;
	struct hostent *server;
	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd < 0) 
		std::cerr<"ERROR opening socket";
	return sock_fd;
}

int 
create_server(char *ip,int port){
	int sock_fd, newsock_fd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd < 0){
	       std::cerr<< "ERROR opening socket\n" ;
	       return sock_fd;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr =inet_addr(ip);
	serv_addr.sin_port = htons(port);
	if (bind(sock_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		std::cerr << "Could not bind server at IP: " << ip << " Port: " << port<< std::endl;
		return 0;
	}
	return sock_fd;
}

int 
send_udp(int sock_fd, void * buf, int buf_len){
	struct sockaddr_in there_addr;
	int len = sizeof(there_addr);
	return sendto(sock_fd, buf, buf_len, 0, (struct sockaddr *)&there_addr, len);
}

int 
recv_udp(int sock_fd, void * buf, int buf_len, sockaddr_in* there_addr ) {
	int len = sizeof(there_addr);
	return recvfrom(sock_fd, buf, buf_len, 0, (struct sockaddr *)there_addr, (socklen_t *)&len);
}

#endif
