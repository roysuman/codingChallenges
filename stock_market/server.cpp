/*
 *server.cpp
 *
 * last modified : 30/5/2013
 *
 * developed by :suman roy (email.suman.roy@gmail.com)
 *
 * this is a server programm, mainly used to create server at two different port
 *
 *it sends data to client after creating UDP connections
 */

#include "file.hpp"
#include "connection.hpp"
#include "structure.hpp"
#include "function.hpp"
#include <sys/time.h>
#include<unistd.h>
int main(int argc, char *argv[]){
	struct timeval tv1,tv2;
	int count=0 , count_p1_prio=0 , count_p2_prio=0;  
	struct ExchangeA_MD st;
	struct IpInfoConfig info;
	int i=1,fd,n1_send,n2_send,random,n,sockfd,sockfd1,flag;
	unsigned char buffer[256];
	if(argc<1){
		std::cerr<<"USAGE: " << argv[0] << " file name." << endl;
		exit(EXIT_FAILURE);
	}
	/* open and process config file */
	File_ fp(argv[1]);
	info=fp.Value();
	ipprint(info);
	
	fstream server_log;
	server_log.open(info.server_log,ios::out);/* server log */
	if(!server_log.is_open()){
		std::cerr<<"file "<<info.server_log
			<<"  can't be open..exiting"
			<<std::endl;
		exit (EXIT_FAILURE);
	}
	sockfd=create_server(info.ip1,info.port1);/* socket-1 */
	if(sockfd==0){
		std::cout<<"can't create the socket1 with\
		       	given specification."<<std::endl;
		exit ( EXIT_FAILURE);
	}

	sockfd1=create_server(info.ip2,info.port2);/* socket-2 */
	if(sockfd1==0){
		std::cout<<"can't create the sockfd2 with\
		       	given specification."<<std::endl;
		exit (EXIT_FAILURE);
	}
	socklen_t size = sizeof(ExchangeA_MD) * 10240;
	if ( ( setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)&&
			( setsockopt(sockfd1, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)) {
	  std::cerr<<"ERROR setsockopt\n";
	  exit(0);
  }
	fd=open(info.path, ios::in | ios::out | ios::binary);
	if(fd<0) {
		std::cerr<<"file cant open\n";
		exit ( EXIT_FAILURE ) ;
	}
	struct ExchangeA_MD  obj;
	sockaddr_in there_addr; /*client address*/
  int len=sizeof(there_addr);
  /*establish connection*/
  if ( recvfrom(sockfd, (void*)&obj, sizeof(ExchangeA_MD), 0, 
       (struct sockaddr *)&there_addr, (socklen_t *)&len) < 0 ) {
	  std::cerr << "Sorry I failed you...:P\n";
	  exit(EXIT_FAILURE); /*TODO use select to make it better*/
  }else {
	  std::cerr<< "Yes I have done it: Got conn from :"
		  << "\nIP: " << inet_ntoa(there_addr.sin_addr)
		  << "\nPort: " << there_addr.sin_port
		  << "\nMssg: " << (char*)&obj << std::endl;
  }
  gettimeofday(&tv1,NULL);
  do{
	  /*read data from bin file*/
	  flag=read(fd,(void *)&obj, sizeof(struct ExchangeA_MD));
	  if(flag<0)
		  std::cerr<<"\nreading error\n";
	  else{
		 uint8_t *temp_send=new uint8_t[sizeof(obj)];
		 /*serialize data */
		 server_log<<obj.seqno_<<":"<<(int)obj.level_<<std::endl;
		 int n_size = serialize(obj, temp_send);
#ifdef TEST
		 ExchangeA_MD test = deserialize( temp_send ); 
		 std::cout <<n_size << "( " << sizeof(obj) << " )";
		 stru_print( test);
#endif
		 /*send with priority*/
		 random= rand() % 10 + 1;
		 if(random>info.priority){
			 count_p1_prio++;
			 n1_send=sendto(sockfd, (void*)temp_send, n_size, 0, 
					(struct sockaddr*)&there_addr, 
					sizeof(struct sockaddr));
			 n2_send=sendto(sockfd1,(void*)temp_send, n_size, 0,
					(struct sockaddr*)&there_addr, len);
		 }else{
			 count_p2_prio++;
			 //TODO understand why sequence is differ from prev.
			 n2_send=sendto(sockfd1,(void*)temp_send, n_size, 0,
					 (struct sockaddr*)&there_addr, len);
			 n1_send=sendto(sockfd,(void*)temp_send, n_size, 0,
					 (struct sockaddr*)&there_addr,
					 sizeof(struct sockaddr));
		 }
		 
		 if(n1_send<0 && n2_send<0 ){
			 std::cerr<< "can't write.\n"
				<< n1_send << " " << n2_send
				<< "\nWaiting for your response...:  "<<std::endl;
		 }
	  }
	  usleep( 20000 );
	  
  }while(flag!=0);
  gettimeofday(&tv2,NULL);
  close(sockfd);
  //end_time=clock();
  // float diff = ((float)start_time - (float)end_time) / 1000000.0F; // calculating microsecond takenn by server
  float diff=(tv2.tv_sec-tv1.tv_sec)*(uint64_t)1000000+(tv2.tv_usec-tv1.tv_usec);
  /* write informatiuon in server log file*/
  server_log<<"data send 1st by port(a)"<<info.port1
	    <<" = "<<count_p1_prio
	    <<std::endl
	    <<"data send 1st by port(z)"<<info.port2
	    <<" = "<<count_p2_prio
	    <<"\ntime taken"
	    <<diff<<std::endl;
  return 0; 
}

