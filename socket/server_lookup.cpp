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
#include<vector>
#define CONTAINER_CAPACITY 100
#define NO_CONTAINER 2
template< class T>
class LookupManager{
	private:
		typedef struct lookup_index{
			//std::mutex lock;
			std::vector<T> look_up_table;
			bool is_available_to_write;/*can data can be flushed*/
			size_t start_index;
			size_t end_index;
			size_t index_pow;
		}look_up_container_;
		look_up_container_ container[ NO_CONTAINER ];
		bool active_write_lokup_index;
		bool active_read_lokup_index;
	public:
		 LookupManager();
		virtual ~LookupManager(){}
		bool push_at_container(  T& elem);
		bool read_from_container( T* elem, size_t seq_no = 0); 
		bool make_empty_container( size_t seq_no);
};

template<class T>
LookupManager<T>::LookupManager(){
	for ( size_t index =1 ; index <= NO_CONTAINER ; ++index ){
		container[index - 1].resize(CONTAINER_CAPACITY);
		container[index - 1].start_sequence =0;
		container[index - 1].end_sequence =1;
		container[index - 1].index_pow =1;
	}
	active_write_lokup_index = 0;
	return;
}
template<class T>
bool LookupManager<T>::make_empty_container( size_t seq){
		int next_index = active_read_lokup_index == 0? 1: 0;
		if ( (container [ next_index ].look_up_table.size() != 0) &&
				(container [ next_index].look_up_table[CONTAINER_CAPACITY - 1 ] .seq_no == seq ) );
		else{
			next_index = next_index == 0? 1: 0;
			if ( (container [ next_index ].look_up_table.size() != 0) &&
					(container [ next_index].look_up_table[CONTAINER_CAPACITY - 1 ] .seq_no == seq ) );
			else return false;
		}
		
		/*TODO delete next_index point vector*/
		return true;
}
template<class T>
bool LookupManager<T>::read_from_container( T* elem, size_t _seq_no ){
	/* when the active read container is empty 
	   situation: return false*/
	/* if theasked seq no belongs to current container then read and return*/
//	size_t _seq_no = _seq_no % ( container[active_read_lokup_index].index_pow *  CONTAINER_CAPACITY );

	if ( (container [ active_read_lokup_index].look_up_table.size() != 0) &&
			(container [ active_read_lokup_index].look_up_table[0].seq_no *   container[active_read_lokup_index].index_pow >= _seq_no ) &&
			(container [ active_read_lokup_index].look_up_table[container[ active_read_lokup_index].look_up_table.size() - 1 ].seq_no * container[active_read_lokup_index].index_pow <=_seq_no ) ){;}
	else{
		int next_index = active_read_lokup_index == 0? 1: 0;
		if ( (container [ next_index ].look_up_table.size() != 0) &&
				(container [ next_index].look_up_table[0].seq_no *  container[next_index ].index_pow >=_seq_no ) &&
				(container [ next_index].look_up_table[container[ active_read_lokup_index].look_up_table.size() - 1 ].seq_no *  container[next_index].index_pow <= _seq_no ) ){
			active_read_lokup_index = next_index;
		}
		else return false;
	}
	(*elem)=container [ active_read_lokup_index].look_up_table[_seq_no ];
	return true;
}


template<class T>
bool LookupManager<T>::push_at_container( T& elem){
	if ( container [ active_write_lokup_index].look_up_table.size() == CONTAINER_CAPACITY ){
		/*try to switch container */
		int next_index = active_write_lokup_index == 0? 1: 0;
		if ( container [ next_index ].look_up_table.size!= 0 ){
			return false;
		}
		else{
			active_write_lokup_index = next_index;
		}
	}
	container[ active_write_lokup_index].look_up_table.push_back(elem);
	return true;
}


template<class T>
class Server{
	private:
		struct IpInfoConfig info;
		std::fstream server_log;
		int sockfd, sockfd1;
		sockaddr_in there_addr; /*client address*/
		bool sock_created[2];/*help to close socket*/
		struct timeval tv1,tv2;
		int count=0 , count_p1_prio=0 , count_p2_prio=0;  

	public:
		explicit Server(std::string &config_file_name_){
			File_ fp(config_file_name_);
			info = fp.value();
			server_log.open(info.server_log,ios::out);/* server log */
			if(!server_log.is_open()){
				std::cerr<<"file "<<info.server_log
					<<"  can't be open..exiting"
					<<std::endl;
				exit (EXIT_FAILURE);
	}
#ifdef DEBUG
			ipprint( info );
#endif
			
		}
		virtual ~Server(){}
		bool create_servers();
		static void* communicate_with_client(void *);
		static void* read_and_store_market_data( void *);
		static void* maintain_lookup( void *);
		void start_server_action(void);
};

template<class T>
void* Server<T>::maintain_lookup( void *ptr){
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	response_packet rp;
	ssize_t n;
	size_t length = sizeof( rp);

	/* a recv method will ocntiniously read response from client 
	 if client sends a packet sequence number with miss flag
	 check the lookup table and start sending packet from that seq_no*/
	n = recvfrom(srvr_ptr->socket1,(void*)rp,rp, 0, NULL, 0);
	if ( n< 0 ){
		std::cerr<<"ERROR receving response from server"<<std::endl;
		/*TODO handle this situation */
	}
	if ( rp.is_resend){
		/*TODO change the read head from vector*/
	}
}


/*
  float diff=(tv2.tv_sec-tv1.tv_sec)*(uint64_t)1000000+(tv2.tv_usec-tv1.tv_usec);
  server_log<<"data send 1st by port(a)"<<info.port1
	    <<" = "<<count_p1_prio
	    <<std::endl
	    <<"data send 1st by port(z)"<<info.port2
	    <<" = "<<count_p2_prio
	    <<"\ntime taken"
	    <<diff<<std::endl;
*/
template<class T>
bool Server<T>::create_servers( void ){

	bool return_val;

	return_val = true;
	sockfd=create_server(info.ip1,info.port1);/* socket-1 */
	if(sockfd==0){
		std::cout<<"can't create the socket1 with\
		       	given specification."<<std::endl;
		return_val =  false;
	}

	sock_created[0]= sockfd == 0? false : true;

	sockfd1=create_server(info.ip2,info.port2);/* socket-2 */
	if(sockfd1==0){
		std::cout<<"can't create the sockfd2 with\
		       	given specification."<<std::endl;
		return_val = false;
	}
	sock_created[1]= sockfd1 == 0? false : true;
	if ( return_val ){
		struct ExchangeA_MD  obj;
		size_t len=sizeof(there_addr);
		/*establish connection*/
		if ( recvfrom(sockfd, (void*)&obj, sizeof(ExchangeA_MD), 0, 
					(struct sockaddr *)&there_addr, (socklen_t *)&len) < 0 ) {
			std::cerr << "Sorry I failed you...:P\n";
			return_val = false;
		}else {
			std::cerr<< "Yes I have done it: Got conn from :"
				<< "\nIP: " << inet_ntoa(there_addr.sin_addr)
				<< "\nPort: " << there_addr.sin_port
				<< "\nMssg: " << (char*)&obj << std::endl;
		}
	}
	return return_val;
}
/*
	if(argc<1){
		std::cerr<<"USAGE: " << argv[0] << " file name." << endl;
		exit(EXIT_FAILURE);
	}

*/
template<class T>
void* read_and_store_market_data( void *ptr){
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	struct ExchangeA_MD  obj;
	int fd,flag;
	fd=open(srvr_ptr->info.path, ios::in | ios::out | ios::binary);
	if(fd<0) {
		std::cerr<<"file cant open\n";
		/*TODO send signal to to stop--all successfull read data must be send*/
		goto END_local;
	}
	do{
		/*read data from bin file*/
		flag=read(fd,(void *)&obj, sizeof(struct ExchangeA_MD));
		if ( flag !=0 ){
			/*TODO write data to container*/
		}
	}while(flag);

END_local:
	return nullptr;
}


template<class T>
void *communicate_with_client  (void *ptr){

	Server<T> *srvr_ptr=(Server<T>*)ptr;
	struct ExchangeA_MD  obj;

	struct ExchangeA_MD st;
	int i=1,fd,n1_send,n2_send,random,n,flag;
	unsigned char buffer[256];
	size_t len=sizeof(srvr_ptr->there_addr);
	gettimeofday(&(srvr_ptr->tv1),NULL);
	do{
		/*TODO read data from container vec and manupulate the flag val*/
	  if(flag<0)
		  std::cerr<<"\nreading error\n";
	  else{
		 uint8_t *temp_send=new uint8_t[sizeof(obj)];
		 /*serialize data */
#ifdef DEBUG
		 srvr_ptr<<"###Read from binary file###\n";
		 srvr_ptr->server_log<<"Seq_No :"<<obj.seqno_<<" Level :"<<(int)obj.level_<<std::endl;
		 srvr_ptr<<"##########\n";
#endif
		 size_t n_size = serialize(obj, temp_send);
#ifdef TEST
		 std::cout<<"Checking serialization logic\n";
		 ExchangeA_MD test = deserialize( temp_send ); 
		 std::cout <<n_size << "( " << sizeof(obj) << " )";
		 stru_print( test);
#endif
		 //TODO maintain lookup table
		 /*send with priority*/
		 random= rand() % 10 + 1;
		 if(random>srvr_ptr->info.priority){
			 srvr_ptr->count_p1_prio++;
			 n1_send=sendto(srvr_ptr->sockfd, (void*)temp_send, n_size, 0, 
					(struct sockaddr*)&(srvr_ptr->there_addr), 
					sizeof(struct sockaddr));
			 n2_send=sendto(srvr_ptr->sockfd1,(void*)temp_send, n_size, 0,
					(struct sockaddr*)&(srvr_ptr->there_addr), len);
		 }else{
			 srvr_ptr->count_p2_prio++;
			 //TODO understand why sequence is differ from prev.
			 n2_send=sendto(srvr_ptr->sockfd1,(void*)temp_send, n_size, 0,
					 (struct sockaddr*)&(srvr_ptr->there_addr), len);
			 n1_send=sendto(srvr_ptr->sockfd,(void*)temp_send, n_size, 0,
					 (struct sockaddr*)&(srvr_ptr->there_addr),
					 sizeof(struct sockaddr));
		 }
		 
		 if(n1_send<0 && n2_send<0 ){
			 std::cerr<< "can't write.\n"
				<< n1_send << " " << n2_send
				<< "\nWaiting for your response...:  "<<std::endl;
		 }
	  }
	}while(flag!=0);
	/*TODO send signal that it has done */
	gettimeofday(&srvr_ptr->tv2,NULL);
	
	return nullptr;
}



