/* client.cpp
 * last-modified : 30/5/2013
 * developed by : Suman roy (email.suman.roy@gmail.com)
 * this is client side programm
 * used by client to connect with server
 * takes a file containg info like IP,port of server and client log file path
 *it receives server data and maintain a Mktbook
 */

#include"client.h"
/* define static var */
template<class T1 , class T2>
market_map Client<T1,T2>::markets;
template<class T1, class T2>
bool Client<T1,T2>::stop_worker = false;

template < class T>
bool special_compare(const T& elem_container_, const T& pattern){
	bool ret = elem_container_.price == pattern.price ? true: false;
	return ret;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  push_at_level
 *  Description:  push the data into the vector 
 *                @vec_ : reference of vector container
 *                @elem_ : element to insert into the container
 *                @level_: level indicated the index of vector container.
 *                @return void.
 * =====================================================================================
 */

template <class T>
static void 
push_at_level( std::vector< T >& vec_, 
	       T& elem_, int level_ , 
	       bool mismatch = false) {/* TODO remove mismatch */

	if ( mismatch ){
		std::cout<<"Are you kalu\n";
		/* check whether the same value is present or not */
		/*if present then delete*/
		auto  index = 
			find_if( vec_.begin(),vec_.end(),
		 		std::bind( special_compare<T>, std::placeholders::_1, elem_));
		if ( index != vec_.end() )
			vec_.erase(index);
	}
	/*check whether the capacity of vector is less than
	  requested level or not.. if yes then increase capacity*/
	if( vec_.capacity() < level_ ){
		vec_.resize(level_ );
	}
	vec_.insert( vec_.begin() + level_ , elem_);
	return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_book
 *  Description:  print the market book in standard i/o
 *                @sell_order_book: vector container of sell side
 *                @buy_order_book: vector container of buy side
 *                @return void
 * =====================================================================================
 */
/* TODO why you are so keen to use template?? */
void 
static rint_book ( std::vector<MktBook_t>& sell_order_book,
	      std::vector<MktBook_t> buy_order_book ) {
	for ( size_t i= 0; i<sell_order_book.size() && i < buy_order_book.size() ; i ++ ){
		printf ( "%5d \t %10.4f X %10.4f \t %5d\n", sell_order_book[i].size, 
				sell_order_book[i].price, buy_order_book[i].price, 
				buy_order_book[i].size );
	  printf ( "--------------------------------------------\n\n" );
  }
  return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_book
 *  Description:  print the market book in standard i/o
 *                @sell_order_book: vector container of sell side
 *                @buy_order_book: vector container of buy side
 *                @file: reference of log file handler
 *                @return void
 * =====================================================================================
 */
/* TODO why you are so keen to use template?? */
void 
static print_file ( std::vector<MktBook_t>& sell_order_book, 
	    std::vector<MktBook_t> &buy_order_book ,
	    fstream &file ) {
	
	file<<"----------------------------------------------------------------------\n";
	for ( size_t i = 0; i<sell_order_book.size() || 
			i < buy_order_book.size() ; i ++ ){

		if ( i <sell_order_book.size() && i < buy_order_book.size() ){
			/* TODO remove constant val 65535 and use price value
			 * to determine wrong value */

			file << ( (sell_order_book[i].size == 65535 )? 0 : 
					sell_order_book[i].size) << "\t" 
				<< ( ( sell_order_book[i].price < 0 )? 0 :
						sell_order_book[i].price)
			     <<"\tXXX\t"
			      << ( (buy_order_book[i].price < 0) ? 0 : 
					      buy_order_book[i].price)
			      <<"\t"<<  ((buy_order_book[i].size== 65535 )? 0 :
					      buy_order_book[i].size)
			     <<std::endl;
		}else if ( i < sell_order_book.size() ){
			file << ( (sell_order_book[i].size == 65535 )? 0 : 
					sell_order_book[i].size) << "\t" 
				<< ( ( sell_order_book[i].price < 0 )? 0 :
						sell_order_book[i].price)
				<<"\tXXX\tXXX\tXXX"<<std::endl;
		}else{
			file<<"XXX\tXXX\tXXX\t"
				<< ((buy_order_book[i].price < 0) ? 0 :
					       buy_order_book[i].price)
				<<"\t"<<( (buy_order_book[i].size== 65535 )? 0 :
					       buy_order_book[i].size) 
				<<std::endl;
		}
	}
	return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ~Client
 *  Description:  DESTRUCTOR
 * =====================================================================================
 */
template< class T1, class T2>
Client<T1, T2>::~Client(){
	market_map_iterator it;
	buy_sid *var = nullptr;
	for ( it = markets.begin() ; it != markets.end();++it ){
		var = it->second;
		if ( var != nullptr);// delete var; //TODO fix segmentation fault
	}
	return;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  worker
 *  Description:  static worker method. Reads packet from queue and process it
 *                @ptr: pointer of the class 
 *                return nullptr
 * =====================================================================================
 */
template<class T1,class T2>
  void*  Client<T1,T2>::worker(void *ptr){
	  Client<T1,T2> *cl_ptr=(Client<T1,T2>*)ptr;

	  struct ExchangeA_MD  obj;
	  market_map_iterator it;
	  bool mismatch = false;
	  size_t count = 0;

	  /* little bit of indentation issue :P */

	  while( 1 ){
          
	  if ( cl_ptr->queue_empty && stop_worker  )return nullptr;

	  if( cl_ptr->pop_queue( &obj) ){
#ifdef TEST

		  cl_ptr->client_log<<"sequence no ="<<obj.seqno_
			   <<"msg type="<<obj.msg_
			   <<"side = "<<obj.side_
			   <<"level="<<(int)obj.level_
			   <<"size= "<<(int)obj.size_
			   <<"price"<<obj.price_
			   <<std::endl;
#endif

		  MktBook_t mkt_update_ ( obj );

		 /*  check whether the security is already mapped 
		  *  or not */
		 buy_sid *var;
		 std::string secu = std::string(obj.contract_);
		 it = markets.find( secu );

		 if (it == markets.end() ){
			 var = new (std::nothrow) buy_sid();
			 //TODO handle if new returns nullptr
			 if (!markets.insert(std::pair<std::string,buy_sid*>(secu , var)).second) {
				 std::cerr<<"ERROR on creating new market book\n";
				 exit(EXIT_FAILURE);
				 /* TODO handle it */
			 }
		 }else{
			 var = it->second;
		 }
		 
		  switch(obj.msg_){
		      case 0:/*insert*/
			   switch(obj.side_){
			      case 'S':/* sell side */
			      case 's':
#ifdef DEBUG
				      std::cout << "Insert Sell Level:" << (int)obj.level_ << endl;
#endif
				      push_at_level<MktBook_t> ( var->sell_order_book, mkt_update_, obj.level_  );
				      break;
			      case 'B':/* bid side */
			      case 'b':
#ifdef DEBUG
				      std::cout << "Insert buy Level:" << (int)obj.level_ << std::endl;
#endif
				      push_at_level<MktBook_t> (  var->buy_order_book, mkt_update_, obj.level_ );
				  print_file ( it->second->buy_order_book , it->second->sell_order_book , cl_ptr->client_log );
				      break;
			      default:
				      cl_ptr->client_log<<"ERROR wrong side,insert request..."<<std::endl;
				      break;
			   }
			   break;

		      case 1:/* delete/remove */
			   switch(obj.side_){
				   case 'S':/* sell side */
				   case 's':
					   if ( obj.level_ <= var->sell_order_book.size () ){
						   var->sell_order_book.erase( var->sell_order_book.begin()+obj.level_);
					   }
					   else {
						   cl_ptr->client_log<<"ERROR Delete failed(Sell side)... Level" << (int)obj.level_ 
							   << " VecSize:" << var->sell_order_book.size() 
							   <<std::endl;
					   }
					   break;
				   case 'B':
				   case 'b':
					   if ( obj.level_ <= var->buy_order_book.size() )
						   var->buy_order_book.erase( var->buy_order_book.begin()+obj.level_);
						   else{ 
						   cl_ptr->client_log << "ERROR Delete failed(bid side)...Level " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  var->buy_order_book.size() 
							   <<std::endl;
					   }
					   break;
				   default:
					   cl_ptr->client_log<<"ERROR wrong side, delete/remove request"
						   <<std::endl;
					   break;
			   }
			   break;
		      case 2:/* update */
			   switch(obj.side_){
				   case 'S':/*sell side*/
				   case 's':
					   if ( obj.level_ <= var->sell_order_book.size() ) {
						   var->sell_order_book[obj.level_].price=obj.price_;
						   var->sell_order_book[obj.level_].size=obj.size_;
					   }
					   else {
						   /* that level is not present in the market book
						    * insert this data */
						   push_at_level<MktBook_t> ( var->sell_order_book, mkt_update_, obj.level_);

						   cl_ptr->client_log << "Error.. inserting the data while level \
							   is not present for update(S) " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  var->sell_order_book.size()
							   << std::endl;
					   }
					   break;
				   case 'B':/*bid side*/
				   case 'b':
					   if ( obj.level_ <= var->buy_order_book.size() ) {
						   var->buy_order_book[obj.level_].price=obj.price_;
						   var->buy_order_book[obj.level_].size=obj.size_;
					   }
					   
					   else {
						   push_at_level<MktBook_t> (  var->buy_order_book, mkt_update_, obj.level_ );
						   cl_ptr->client_log << "Error.. inserting the data while level \
							   is not present for update(B) "
							   << (int)obj.level_ << " VecSize:"
							   <<  var->buy_order_book.size()
							   << std::endl;
					   }
					   break;
				   default:
					   cl_ptr->client_log<<"wrong side for Update..."<<std::endl;
			   }
			   break;
		   }
		  /* do the market analysis  */
		  (void)cl_ptr->do_market_analysis(var);
		  /* print market stat after certain interval */
		  if( ++count % INTERVAL_COUNT == 0  ) {
			  cl_ptr->client_log<<"\n#######Received MSG count=\t"<<count<<"  ##########"<<std::endl;
			  /* print each market book */
			  for ( it = markets.begin(); it != markets.end(); ++it){
				  cl_ptr->client_log<<"\n PRINTING Stock of [ "
					  <<it->first<<"] \n"<<std::endl;
				  print_file ( it->second->buy_order_book , 
						  it->second->sell_order_book , 
						  cl_ptr->client_log );
			  }
		  }
	  }
	}
  }


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  do_marker_analysis
 *  Description: When the order book is part of a matching engine, orders are matched as 
 *               the interest of buyers and sellers can be satisfied. When there are 
 *               orders where the bid price is equal or higher than the lowest ask, 
 *               those orders can be immediately fulfilled and will not be part of 
 *               the open orders book. If this situation remains, 
 *               due to an error or a condition of the market, 
 *               the order book is said to be crossed. 
 *
 *               var@ a struct varible that stores buy ans sell vector of market
 * =====================================================================================
 */

template <class T1, class T2>
bool Client<T1, T2>::do_market_analysis( buy_sid *var){


#ifdef DEBUG
		  if (  var->buy_order_book.size() >= 1 && var->sell_order_book.size()>= 1) 
			  std::cout<<var->buy_order_book[0].price
				   <<"::"<<var->sell_order_book[0].price
				   <<std::endl;
#endif
		  if (  (var->buy_order_book.size() >= 1) && 
		        (var->sell_order_book.size()>= 1 ) && 
			( var->buy_order_book[0].price >= 
			   var->sell_order_book[0].price )) {

			  
			  // print_file ( var->buy_order_book , var->order_book , cl_ptr->client_log );
			  int dif1 = (int)var->buy_order_book[0].size - 
				  	(int)var->sell_order_book[0].size;
			  /* if diff is 0... exact match... delete both from bid and sell side */
			  if ( dif1 == 0 ){
				  var->sell_order_book.erase( var->sell_order_book.begin() + 0);
				  var->buy_order_book.erase( var->buy_order_book.begin() + 0 );

			 }
			  /* else Perform delete at appro side and update another one */
			  else if ( dif1 > 0 ){
				 var->buy_order_book[0].size -=var->sell_order_book[0].size;
				 var->sell_order_book.erase( var->sell_order_book.begin() + 0);
			 }else if ( dif1 < 0 ){
				 var->sell_order_book[0].size -=var->buy_order_book[0].size;
				 var->buy_order_book.erase( var->buy_order_book.begin() + 0 );
			 }
		  }
		  return true;
}
template<class T1, class T2>
bool Client< T1,  T2>::init_client( void ){
	/*-----------------------------------------------------------------------------
	 * opena nd process configuration file
	 * -----------------------------------------------------------------------------*/
	File_ fp(config_file); /* this file has defined at header file file.hpp*/
	info=fp.ClientCredentials();/*to open client cofiguration file*/
	if ( info.port2 == 0 ) return false;
	/*open client log file*/
	client_log.open( info.client_log , ios::out );
	if ( !client_log.is_open()) return false;
	return true;

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_data
 *  Description:  establish connection with the server. Recv packets from server and store
 *                into a queue. This method is not responsable to process/maintain market
 *                related info. TODO: think about packet loss logic..UDP is not reliable
 *                @ptr: pointer ref of the calling class
 *                return nullptr
 * =====================================================================================
 */

template<class T1,class T2>
void* Client<T1,T2>::get_data(void *ptr ){
  Client<T1,T2> *cl_ptr=(Client<T1,T2>*)ptr;
  uint16_t prev_seqno;
  int seq_no= 0, no=1,i=1,sockfd[2], portno, n, fd,flag=1;
  unsigned char buffer[256]="First ping\n";
  struct ExchangeA_MD  obj;
  pthread_mutex_t lock;
  struct sockaddr_in serv_addr[2];

  response_packet  res_pack;


  for ( int i=0; i<2; i++ ) {
    bzero((char *) &serv_addr[i], sizeof(serv_addr));
    serv_addr[i].sin_family = AF_INET;
    serv_addr[i].sin_addr.s_addr = inet_addr((i==0?cl_ptr->info.ip1:cl_ptr->info.ip2));
    serv_addr[i].sin_port = htons((i==0?cl_ptr->info.port1:cl_ptr->info.port2)); // set these fields will be used later
    sockfd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd[i] < 0) 
	    std::cerr<<"ERROR opening socket...";
  }
 
  socklen_t size = sizeof(ExchangeA_MD) * 10240;
  if ( ( setsockopt(sockfd[0], SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)&&
		  ( setsockopt(sockfd[1], SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)) {
	  std::cerr<<"ERROR setsockopt\n";
	  exit(0);
  }
 
  /*ping server to establish connection */
 
  if ( ( sendto(sockfd[0], buffer, sizeof(buffer), 0, 
	 (struct sockaddr*)&serv_addr[0], sizeof(serv_addr[0])) < 0 ) &&
       ( sendto(sockfd[1], buffer, sizeof(buffer), 0, 
	 (struct sockaddr*)&serv_addr[1], sizeof(serv_addr[1])) < 0) ) {
	  
	  std::cerr << "Could not write to Server socket\n";
	  exit(0);
  }else { /* testinf */
	  struct sockaddr_in name;
	  socklen_t len = sizeof(name);
	  accept4(sockfd[0], (struct sockaddr *)&name, &len, SOCK_NONBLOCK);
	  std::cout<< "Yes I have done it: Got conn from : "
		  << "\nIP: " << inet_ntoa(serv_addr[0].sin_addr)
		  << "\nPort: " << ntohs(serv_addr[0].sin_port) 
		  << "\nInfo: " << inet_ntoa(name.sin_addr) 
		  << " " << name.sin_port << endl;
  }
  // Now comes select qtiyappa..
  struct timeval       timeout;
  fd_set master_set, working_set;
  FD_ZERO(&master_set);
  int max_sd = sockfd[1]; /* the last created socket to listen */
  FD_SET(sockfd[0], &master_set);
  FD_SET(sockfd[1], &master_set);
  timeout.tv_sec  = 60; /* 1 min waiting time*/
  timeout.tv_usec = 0;
  int rc=0;
  size_t count =0;
  bool is_send_res = false;
  bool mismatch = false;
  std::fstream debug_log;
  debug_log.open("debug_client.log",std::ios::out);
unsigned char temp_buffer[10240];
  do {
      memcpy(&working_set, &master_set, sizeof(master_set));
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
      if ( rc < 0 ) { 
	   perror ( " selec() failed.\n");
	   exit(0);
	}else if (rc == 0){
	   perror("No data received in last 1 minute..");
	   cl_ptr->stop_worker = true;
	   std::cout<<"COUNT ["<<count<<"] "<<std::endl;
	   break;
	}
      else
	 for ( int i=0; i<max_sd; i++ ) 
   	    if ( FD_ISSET(i, &working_set) ) {
		 int curr_sockfd = i==sockfd[0] ? sockfd[0]
			 :i==sockfd[1] ? sockfd[1] : -1;
		 uint8_t buf_size_to_read = 0;

		 n = recvfrom(curr_sockfd, (void*)&buf_size_to_read, 1, 0, NULL, 0);
		 n = recvfrom(curr_sockfd,(void*)buffer,(int) buf_size_to_read , 0, NULL, 0);
		 if (n < 0) { std::cerr << "ERROR reading from socket\n"; exit(0); }
		 obj=deserialize(buffer);
#ifdef DEBUG
		 std::cout<<"Print recv data after deserialized\n";
		 stru_print(obj);
#endif
		 
//#ifdef DEBUG
		 debug_log<<"GOTDATA sequence no ="<<obj.seqno_
			   <<"msg type="<<obj.msg_
			   <<"side = "<<obj.side_
			   <<"level="<<(int)obj.level_
			   <<"size= "<<(int)obj.size_
			   <<"price"<<obj.price_
			   <<std::endl;
//#endif
		 obj.level_ -=1;
		
		 if(  obj.seqno_ > seq_no ){
			 if ( seq_no + 1 != obj.seqno_ ){
			//	 print_file (cl_ptr-> buy_order_book , cl_ptr->sell_order_book , cl_log->client_log );
				 debug_log<<seq_no<<"MISMATCH"<<obj.seqno_<<std::endl;
				 /* read all packets from buffer and make buffer empty */
				 timeout.tv_sec  = 1; /* 1 min waiting time*/
				 rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
				 if ( rc < 0 ) { 
					 perror ( " selec() failed.\n");
					 break;
				 }else if (rc == 0){
					 perror("No data received in last 1 sec..");
					 break;
				 }else{
					 n = recvfrom(sockfd[0],(void*)temp_buffer,(int) 10240, 0, NULL, 0);
					 std::cout<<"EMPTY"<<std::endl;
					// n = recvfrom(sockfd[1],(void*)temp_buffer,(int) 10240, 0, NULL, 0);
					 std::cout<<"EMPTY"<<std::endl;
					 //exit(0);
				 }
				 timeout.tv_sec  = 60; /* 1 min waiting time*/
				 /* request server to resend missed packes */
				 res_pack.is_resend = true;
				 res_pack.seq_no = (uint16_t) seq_no + 1; 
				 is_send_res = true;
			 }
			 else
			 {
				 seq_no = obj.seqno_;
				 cl_ptr->push_queue(obj);
				 /* update server to remove packets from his lookup storage */
				 if ( ++count % 100 == 0){
					 res_pack.is_resend = false;
					 res_pack.seq_no = obj.seqno_;
					 is_send_res = true;
				 }
			 }
		 }
	    }
      /* send packet to server 
       * TODO but UDP is not reliable....
       * suppose server sendding packets from seq NO 1....1000
       * and 1000th packet got lost...then client seds a request packet to 
       * server requesting 1000th packet...and this resquest packet also got loss
       * THEN WHAT TO DO??
       * */
      if ( is_send_res ){
	      if ( ( sendto(sockfd[0], (void*)&res_pack, sizeof(res_pack), 0, 
		(struct sockaddr*)&serv_addr[0], sizeof(serv_addr[0])) < 0 ) &&
		      ( sendto(sockfd[1], (void*)&res_pack, sizeof(res_pack), 0,
			  (struct sockaddr*)&serv_addr[1], sizeof(serv_addr[1])) < 0) ) {
	      std::cerr << "Could not write to Server socket\n";
	      }else is_send_res = false;
      }
  }while(i!=0);
  close(sockfd[0]);
  close(sockfd[1]);
  cl_ptr->client_log.close();
  debug_log.close();
  return 0;
}
  
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  initi the environment.... and then call two thread
 * =====================================================================================
 */
 int main(int argc, char**argv){
	 pthread_t threads[2];
	 int rc[2];
	 if (argc < 2) {
		 std::cerr<<"usage "<<argv[0]<<"configuration file\n";
		 exit(0);
	 }
	 Client<MktBook_t,ExchangeA_MD> cl(argv[1]);
	 if ( cl.init_client()){
		 rc[0] = pthread_create(&threads[0], NULL, Client<MktBook_t,ExchangeA_MD >::get_data, (void*)&cl);
		 rc[1] = pthread_create(&threads[1], NULL, Client<MktBook_t,ExchangeA_MD>::worker, (void*)&cl);
		 (void) pthread_join(threads[0],nullptr);
		 (void) pthread_join( threads[1],nullptr);
	 }
	 return 0;

 }
