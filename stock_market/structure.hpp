//  structure.hpp
//
//last modified: 30/5/2013
//developed by : suman roy (email.suman.roy@gmail.com)
// description : this header file contains definations of all structures needed by socket assignment and some functions to
// acess those structure data
//  Sttructure ExchangeA_MsgType : this structure is needed for storing exchange raw data sent by server (binary file)
//  Function stru_print : print exchange raw data read from raw binary file
// Structure MktBook  :  to create mkt book
// Structure IpinfoConfig : for storing server information like ip , port etc 
// Structure ClientInfo : for storing client info( Ip , port ,log file) to connectwith  server 

#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_
#include "header.hpp"
//#define DEBUG 1
//#define INFO
//structure_Exchange_message
enum ExchangeA_MsgType {NewLevel,DeleteLevel,ModifyLevel} ;
struct ExchangeA_MD{
  uint16_t seqno_;
  char contract_[4];
  uint8_t level_;
  double price_;
  uint16_t size_;
  char side_;
  ExchangeA_MsgType msg_;
};


/*print_structure_information_Exchange*/
void 
stru_print(ExchangeA_MD &sample){
	std::cout<<"\nSeqNo: "    << sample.seqno_
		<<"\nContract: " << sample.contract_
		<<"\nLevel: "    << (int)(sample.level_)
		<<"\nPrice: "    << sample.price_
		<<"\nSize: "     << sample.size_
		<<"\nSide: "     << sample.side_
		<<"\nMsgType: "  << sample.msg_ 
		<< std::endl;
  return;
}

typedef struct MktBook {
	double price;
	uint16_t size;
	char side;
	MktBook( ExchangeA_MD A )
		: price(A.price_), size(A.size_), side(A.side_) {}
	MktBook() {price=-1; size=-1; side='X';}
	void print(){
		printf("%f %d %c\n", price, size, side);
	}
} MktBook_t;

/*structure_input_configfile_for_server*/
struct IpInfoConfig{
	char ip1[20];
	int port1;
	char ip2[20]; 
	int port2;
	char path[30];
	int priority ;
	char server_log[30];
};
struct ClientInfo{
	char ip1[20];
	int port1;
	char ip2[20];
	int port2;
	char client_log[30];
};

void 
ipprint(IpInfoConfig &temp){
	std::cout<<"\n ip1: "   <<temp.ip1
		<<"\n port1:"  <<temp.port1
		<<"\n ip2:   " <<temp.ip2
		<<"\n port2  "<<temp.port2
		<<"\n path: " <<temp.path
		<<"\n priority "<<temp.priority
		<<"\n Server log"<<temp.server_log
		<<std::endl;
}

typedef  struct response_packet_{
	bool is_resend;
	uint16_t seq_no;
}response_packet;
#endif // _STRUCTURE_H_
