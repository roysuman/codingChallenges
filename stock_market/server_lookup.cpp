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
/* 
template<class T>
bool  Server<T>::srvr_ptr->start_sending_from_lookup = false;
template<class T>
ssize_t Server<T>::srvr_ptr->lookup_seq =-1;
*/

#include"server_lookup.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_server
 *  Description:  init the environmeny..read config file..
 *  		//TODO there must be one ret statement
 * =====================================================================================
 */
template< class T>
bool Server<T>::init_server( void){

	bool return_val;
	File_ fp(config_file_name);
	info = fp.Value();
	if ( info.port1 == 0 )
		return false;

	server_log.open(info.server_log,ios::out);/* server log */
	if(!server_log.is_open()){
		std::cerr<<"file "<<info.server_log
			<<"  can't be open..exiting"
			<<std::endl;
		return false;
	}
	global_storage_queue = new (std::nothrow)CircularQueue<T>(QUEUE_SIZE);
	lookup_queue= new (std::nothrow)CircularQueue<T>(QUEUE_SIZE);
	if ( global_storage_queue == nullptr || lookup_queue == nullptr)
		return false;
#ifdef DEBUG
	ipprint( info );
#endif
	return ( create_servers() );
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  maintain_lookup
 *  Description:  read incoming packets from client...
 *  		  if packet resend request id ON...init resend...
 *  		  else make room at temp_storage_que...which has used for lookup
 * =====================================================================================
 */
template<class T>
void* Server<T>::maintain_lookup( void *ptr){
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	response_packet rp;
	ssize_t n;
	size_t length = sizeof( rp);
	/* a recv method will ocntiniously read response from client 
	 if client sends a packet sequence number with miss flag
	 check the lookup table and start sending packet from that seq_no*/
	do{
		/* TOTO create a TCP server */
		n = recvfrom(srvr_ptr->sockfd,(void*)&rp,sizeof(rp), 0, NULL, 0);
		n = recvfrom(srvr_ptr->sockfd,(void*)&rp,sizeof(rp), 0, NULL, 0);
		if ( n <0 ){
			std::cerr<<"\nError on receiving from CLIENT\n";
			exit(0);
		}
		if ( rp.is_resend){/* start resending */
			/* set the following variables to give a signal to client */
				      
			pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
			srvr_ptr->start_sending_from_lookup= true;
			srvr_ptr->lookup_seq = rp.seq_no;

			pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
#ifdef DEBUG
			std::cout<<"RESEND"<<"FROM SEQUENCE NUMBER "<<rp.seq_no<<std::endl;
#endif
		}
		else{
			/*remove last 100 elements*/
#ifdef DEBUG
			std::cout<<"REMOVE LAST 100 elem\n"<<std::endl;
#endif
			srvr_ptr->lookup_queue->update_read_head( 100 );
		}
	}while(n>0);
	return nullptr;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  calc_stat
 *  Description:  calculate the server stat
 * =====================================================================================
 */

template<class T>
void Server<T>::calc_stat( void){
	float diff=(tv2.tv_sec-tv1.tv_sec)*(uint64_t)1000000+(tv2.tv_usec-tv1.tv_usec);
	server_log<<"Data send 1st by port(a)"<<info.port1
		<<" = "<<count_p1_prio
		<<std::endl
		<<"Data send 1st by port(z)"<<info.port2
		<<" = "<<count_p2_prio
		<<"\nTotal Time taken"
		<<diff<<std::endl;
	return;
}
	
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
 * ===  FUNCTION  ======================================================================
 *         Name:  read_and_store_market_data
 *  Description:  read data from binary file and store into a queue
 * =====================================================================================
 */
template<class T>
void* Server<T>::read_and_store_market_data( void *ptr){
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	struct ExchangeA_MD  obj;
	int fd,flag;
	fd=open(srvr_ptr->info.path, ios::in | ios::out | ios::binary);
	if(fd<0) {
		std::cerr<<"file cant open\n";
		goto END_local;
	}
	do{
#ifdef DEBUG
		std::cout<<__FUNCTION__<<std::endl;
#endif
		/*read data from bin file*/
		flag=read(fd,(void *)&obj, sizeof(struct ExchangeA_MD));
		if ( flag !=0 ){

			while ( srvr_ptr->global_storage_queue->en_queue(obj) == 0){
				usleep(1);
			}
		}
	}while(flag);

END_local:
	return nullptr;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  communicate_with_client
 *  Description:  Read from queue..and send data to client
 * =====================================================================================
 */

template<class T>
void * Server<T>::communicate_with_client  (void *ptr){

	Server<T> *srvr_ptr=(Server<T>*)ptr;
	struct ExchangeA_MD  obj;
	int i=1,fd,n,flag;

	struct ExchangeA_MD st;
	size_t resent_send_seq =0;
	unsigned char buffer[256];
	gettimeofday(&(srvr_ptr->tv1),NULL);
	do{
#ifdef DEBUG
		std::cout<<__FUNCTION__<<std::endl;
#endif
		pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
		if ( srvr_ptr->start_sending_from_lookup ){
			/*pause reading from queue...
			 * send the missed packet from lookup queue..
			 */
			/* is send_seq no is 0..which is invalid OR
			 * resent_send_seq number id lessthan asked seq no
			 * no need to perform resend operation */
			if ( resent_send_seq == 0 || resent_send_seq < srvr_ptr->lookup_seq ){
				std::cerr<<"Nothing to resend\n";
				srvr_ptr->start_sending_from_lookup = false;
				exit(0);

			}
			else{
				std::cout<<"\nREAD FROM LOOKUP\n"<<std::endl;
				srvr_ptr->start_sending_from_lookup = false;
				pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
				/* now start sending packets sequentially from 
				 * the lookup array... */
				for( size_t index = srvr_ptr->lookup_seq ; index <= resent_send_seq;++index ){
					/* TODO check that read request index available or not */
					std::cout<<"\nINFO...read from look up queue POSITION "<<std::endl;
						
					
					srvr_ptr->lookup_queue->read_queue_from_position(&obj, index -1 );
					std::cout<<"\nREAD SEQ NO "<<obj.seqno_<<std::endl;
					Server<T>::send(obj,(void*)srvr_ptr);
					/* there mighe be a situation..when packel loss ocurs
					 * during lost packet resending.....
					 * if that happens ..then again start resending from the
					 * lost index... */
					pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
					if (srvr_ptr->start_sending_from_lookup &&
							( srvr_ptr->lookup_seq <= index)&& 
							!(resent_send_seq<srvr_ptr->lookup_seq) ){

						index = srvr_ptr->lookup_seq;/* update the resend index */
						srvr_ptr->start_sending_from_lookup = false;
					}
					pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
				}
				pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
				srvr_ptr->start_sending_from_lookup = false;
			}
		}
		pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
		/* read from normal queue */
		if ( !srvr_ptr->lookup_queue->is_full() ){
			flag = srvr_ptr->global_storage_queue->de_queue(&obj);/* TODO check read or not*/ 
			if(flag==0 )
				std::cerr<<"\nQueue is empty\n";
			else{
				/* store the data into temp storage for lookup 
				 * this situation will never happen*/
				while ( srvr_ptr->lookup_queue->en_queue(obj)==0 ){
					std::cout<<"INFO...Lookup queue is full..\
						going to sleep for a while"<<std::endl;
					/*TODO remove the below line...it;s not his job
					 * before that implement the response send part @client*/
					//	srvr_ptr->lookup_queue->update_read_head( 100);
					usleep(1); /* TODO remove cmnt */
				}
			}
			resent_send_seq = obj.seqno_;
			/* to simulate packet loss env */
			if ( obj.seqno_ != 100  || obj.seqno_ != 854)
				Server<T>::send(obj,(void*)srvr_ptr);
		}
	}while(true);/* TODO stoping condition */
	/*TODO send signal that it has done */
	gettimeofday(&srvr_ptr->tv2,NULL);
	
	return nullptr;
}
template <class T>
bool Server<T>::send(T &obj, void *ptr){
#ifdef DEBUG
	std::cout<<__FUNCTION__<<std::endl;
#endif
	
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	uint8_t *temp_send=new uint8_t[sizeof(obj)];

	bool return_val = true;
#ifdef DEBUG
	std::cout<<"Sending seq No [ "<<obj.seqno_<<std::endl;
#endif
	size_t n_size = serialize(obj, temp_send);
	ssize_t n1_send,n2_send;
	int random;
	size_t len=sizeof(srvr_ptr->there_addr);
#ifdef TEST
	std::cout<<"Checking serialization logic\n";
	ExchangeA_MD test = deserialize( temp_send ); 
	std::cout <<n_size << "( " << sizeof(obj) << " )";
	stru_print( test);
#endif
	srvr_ptr->server_log<<"### SENDING TO CLIENT ###\n";
	srvr_ptr->server_log<<"Seq_No :"<<obj.seqno_
		<<" Level :"<<(int)obj.level_<<std::endl;
	srvr_ptr->server_log<<"##########################\n";
	/*send with priority*/
	random= rand() % 10 + 1;
	
	if(random>srvr_ptr->info.priority){
		srvr_ptr->count_p1_prio++;
		n1_send=::sendto(srvr_ptr->sockfd, (void*)temp_send, n_size, 0,
				(struct sockaddr*)&(srvr_ptr->there_addr), 
				sizeof(struct sockaddr));
		n2_send=::sendto(srvr_ptr->sockfd1,(void*)temp_send, n_size, 0,
				(struct sockaddr*)&(srvr_ptr->there_addr), len);
	}else{
		srvr_ptr->count_p2_prio++;
		n2_send=::sendto(srvr_ptr->sockfd1,(void*)temp_send, n_size, 0,
				(struct sockaddr*)&(srvr_ptr->there_addr), len);
		n1_send=::sendto(srvr_ptr->sockfd,(void*)temp_send, n_size, 0,
				(struct sockaddr*)&(srvr_ptr->there_addr),
				sizeof(struct sockaddr));
	}
	if(n1_send<0 && n2_send<0 ){
		srvr_ptr->server_log<< "can't write.\n"
			<< n1_send << " " << n2_send
			<< "\nWaiting for your response...:  "<<std::endl;
		return_val = false;
	}
	return return_val ;
}



int main(int argc, char**argv){
	 int rc[3];
	 pthread_t threads[3];
	if(argc<1){
		std::cerr<<"USAGE: " << argv[0] << " file name." << endl;
		exit(EXIT_FAILURE);
	}

	Server<struct ExchangeA_MD>server_ins(argv[1]);
	if ( server_ins.init_server()){

		 rc[0] = pthread_create(&threads[0], NULL, Server<struct ExchangeA_MD>::read_and_store_market_data, (void*)&server_ins);
		 rc[1] = pthread_create(&threads[0], NULL, Server<struct ExchangeA_MD>::communicate_with_client, (void*)&server_ins);
		 rc[2] = pthread_create(&threads[0], NULL, Server<struct ExchangeA_MD>::maintain_lookup, (void*)&server_ins);
		 (void) pthread_join(threads[0],nullptr);
		 (void) pthread_join( threads[1],nullptr);
		 (void) pthread_join( threads[2],nullptr);
	}
	return 0;
}