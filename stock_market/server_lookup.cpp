
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

	struct timeval       timeout;

	fd_set master_set, working_set;
	FD_ZERO(&master_set);
	int max_sd = srvr_ptr->sockfd1; /* the last created socket to listen */
	FD_SET(srvr_ptr->sockfd, &master_set);
	FD_SET(srvr_ptr->sockfd1, &master_set);
	timeout.tv_sec  = 60; /* 1 min waiting time*/
	timeout.tv_usec = 0;
	unsigned char buffer[10240];
	size_t store_prev_req_seq =-1;

	int rc;
	do{
		memcpy(&working_set, &master_set, sizeof(master_set));
		rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
		if ( rc < 0 ) { 
			perror ( " selec() failed.\n");
			break;
		}else if (rc == 0){
			perror("No data received in last 1 minute..");
			//TODO to keep the connection..implement heartbeat..
			break;
		}
		else
			for ( int i=0; i<max_sd; i++ ) 
				if ( FD_ISSET(i, &working_set) ) {
					int curr_sockfd = i==srvr_ptr->sockfd ? srvr_ptr->sockfd
			 :i==srvr_ptr->sockfd1 ? srvr_ptr->sockfd1 : -1;
					/* a recv method will ocntiniously read response from client 
					 * if client sends a packet sequence number with miss flag
					 * check the lookup table and start sending packet from that seq_no*/
					/* TODO.. UDP server is not relaiable..YOU might use TCP*/
					n = recvfrom( curr_sockfd,(void*)&rp,sizeof(rp), 0, NULL, 0);
					if ( n <0 ){
						std::cerr<<"\nError on receiving from CLIENT\n";
						break;
					}
					/* rp.is_resend == true, indicates that there is a packet loss ..
					 * hence client sent a request to resend those lost packets */
					if ( rp.is_resend){/* start resending */
						if ( rp.seq_no != store_prev_req_seq ){
							store_prev_req_seq = rp.seq_no;
						/* set the following variables to give a signal to client */
						pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
						/* just set the value...Method communicate_with_client(void*)
						 * continiously does check whether resend of packets needed
						 * or not */
						srvr_ptr->start_sending_from_lookup= true;
						srvr_ptr->lookup_seq = rp.seq_no;
						pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
						/* read all packets from udp and clear buffer */
						timeout.tv_sec  = 1; /* 1 min waiting time*/
						rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
						if ( rc < 0 ) { 
							perror ( " selec() failed.\n");
							break;
						}else if (rc == 0){
							perror("No data received in last 1 minute..");
							//TODO to keep the connection..implement heartbeat..
							break;
						}else{
							n = recvfrom(curr_sockfd,(void*)buffer,(int) 10240 , 0, NULL, 0);
						}
					//	n = recvfrom(srvr_ptr->sockfd1 ,(void*)buffer,(int) 10240 , 0, NULL, 0);
						timeout.tv_sec  = 60; /* 1 min waiting time*/
#ifdef DEBUG
						std::cout<<"RESEND"<<"FROM SEQUENCE NUMBER "<<rp.seq_no<<std::endl;
						
#endif
						}
					}else{
						/*remove last 100 elements*/
#ifdef DEBUG
						std::cout<<"REMOVE LAST 100 elem\n"<<std::endl;
#endif
#ifdef CONSIDER_NO_PACKET_LOSS
						if ( (size_t)rp.seq_no == 1000 ){
							gettimeofday(&srvr_ptr->tv2,NULL);
							srvr_ptr->server_log<<"\n\n TIME TAKEN BY SERVER ...CONSIDERING NO PACKET LOSS.. RESEND\n"<<std::endl;
							srvr_ptr->calc_stat();
						}
#endif
						srvr_ptr->lookup_queue->update_read_head( 100 );
					}
				}
	}while(n);
	srvr_ptr->stop_server = true;

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


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  create_servers
 *  Description:  create two udp server
 * =====================================================================================
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
		/* TODO bring the service down */
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
				/* queue full... go for a nano sleep */
				usleep(10);
			}
		}
	}while(flag);/* untill binary file is not empty */

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
	int i=1,fd,n,flag=1;

	struct ExchangeA_MD st;
	size_t resent_send_seq =0,index,temp;
	unsigned char buffer[256];
	gettimeofday(&(srvr_ptr->tv1),NULL);
	bool i_am_done_once = false;
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
			}
			else{
				std::cout<<"\nREAD FROM LOOKUP\n"<<std::endl;
				index = srvr_ptr->lookup_seq;
				srvr_ptr->start_sending_from_lookup = false;
			//	pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
				/* now start sending packets sequentially from 
				 * the lookup array... */
				srvr_ptr->prev_send_seq = index;
				for( ; index <= resent_send_seq;++index ){
					/* TODO check that read request index available or not */
						
					
					srvr_ptr->lookup_queue->read_queue_from_position(&obj, index -1 );
//#ifdef DEBUG
					std::cout<<"\nREAD SEQ NO "<<obj.seqno_<<std::endl;
//#endif
					Server<T>::send(obj,(void*)srvr_ptr);
					/* there mighe be a situation..when packel loss ocurs
					 * during lost packet resending.....
					 * if that happens ..then again start resending from the
					 * lost index... */
			//		pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
					/* again there is a lost packet resend sequence?
					 * while resending lost packet.....then start resending 
					 * packet from lost packet seqence number */
				 	if (srvr_ptr->start_sending_from_lookup &&
							( srvr_ptr->lookup_seq < index )){


						index = srvr_ptr->lookup_seq;/* update the resend index */
						srvr_ptr->start_sending_from_lookup = false;
					}
				//	pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
				}
		//		pthread_mutex_lock(&(srvr_ptr->look_up_var_lock));
				/* resend done  */
				srvr_ptr->start_sending_from_lookup = false;
			}
		}
		pthread_mutex_unlock(&(srvr_ptr->look_up_var_lock));
		/* read from normal queue */
		if ( !srvr_ptr->lookup_queue->is_full() ){/* if lookup storage is not full.. */
			flag = srvr_ptr->global_storage_queue->de_queue(&obj);
			if(flag==0 );/* queue is empty */
				//std::cerr<<"\nQueue is empty\n";
			else{
				/* store the data into temp storage for lookup 
				 * this situation will never happen*/
				while ( srvr_ptr->lookup_queue->en_queue(obj)==0 ){
					std::cout<<"INFO...Lookup queue is full..\
						going to sleep for a while"<<std::endl;
					usleep(1);
				}
				resent_send_seq = obj.seqno_;
#ifdef TEST_PACKET_LOSS
				/* to test the logic.. forcefully do packet loss
				 * because sometimes UDP sends all packet successfulyi..
				 * it comment the beow line @production */
				if ( (obj.seqno_ ) != 200  ){
				//	srvr_ptr->server_log<<"SEQ NO"<<obj.seqno_<<std::endl;
				//	srvr_ptr->server_log<<"\nEither packe 100 | 854 \n"<<std::endl;
				        Server<T>::send(obj,(void*)srvr_ptr);
				}
#else
				Server<T>::send(obj,(void*)srvr_ptr);
#endif
#ifdef CONSIDR_PACKET_LOSS
				/* following logic just to check the time taken by server
				 * to send all packets without considering packet loss */
				if ( obj.seqno_ == 1000 && !i_am_done_once ){
					srvr_ptr->server_log<<"\n\n\nTIME TAKEN BY SERVER TO SEND ALL PACKETS...without considering packet loss\n"<<std::endl;
					gettimeofday(&srvr_ptr->tv2,NULL);
					srvr_ptr->calc_stat();
					i_am_done_once = true;
				}
#endif
			}
		}
	
	}while( !srvr_ptr->stop_server );//&& !srvr_ptr->lookup_queue->is_empty());/* TODO stoping condition */
#ifdef CONSIDER_SERVER_SHUT_SOWN
	srvr_ptr->server_log<<"\n\n\nTIME TAKEN BY THE SERVER ....TILL SERVER IS GOING DOWN\n"<<std::endl;
	gettimeofday(&srvr_ptr->tv2,NULL);
	srvr_ptr->calc_stat();
#endif

	
	return nullptr;
}
template <class T>
bool Server<T>::send(T &obj, void *ptr){
#ifdef DEBUG
	std::cout<<__FUNCTION__<<std::endl;
#endif
	Server<T> *srvr_ptr=(Server<T>*)ptr;
	
	if ( srvr_ptr->prev_send_seq >= obj.seqno_)
		return false;
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
	}else{
		srvr_ptr->prev_send_seq = obj.seqno_;
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
		 rc[1] = pthread_create(&threads[1], NULL, Server<struct ExchangeA_MD>::communicate_with_client, (void*)&server_ins);
		 rc[2] = pthread_create(&threads[2], NULL, Server<struct ExchangeA_MD>::maintain_lookup, (void*)&server_ins);
		 for ( size_t index = 0 ; index < 3 ; ++index){
			 (void) pthread_join(threads[index],nullptr);

		 }
	}
	return 0;
}
