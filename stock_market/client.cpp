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
template<class T1, class T2>
bool Client<T1,T2>::stop_worker = false;

template < class T1, class T2>
market_map LimitBook<T1,T2>::markets;
template < class T>
bool special_compare(const T& elem_container_, const T& pattern){
	bool ret = elem_container_.price == pattern.price ? true: false;
	return ret;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  do_stable_book
 *  Description:  maintain the book
 *               after insert or update or dele operation..the price @sell side must be 
 *               in increasing order and decreasing order for bye side.. this function 
 *               maintains that properties...if there is a packet loss also ..market 
 *               will be in stable state
 *            @vec : vector reference for buy/sid
 *            return void
 *            TODO : should I check all indices sequentialy? if index + 1 is mismatch..
 *                  then all indices from index +1 to end will be mismatched..same as
 *                  for top.. think about this
 * =====================================================================================
 */
template<class T1,class T2>
void LimitBook<T1,T2>::do_stable_book (std::vector<T1>& vec,
		size_t level, bool flag){
	ssize_t index;
	if ( flag ){ //if bid siide
		/* check bottom indices(towards end of vector) of recent accessed index */
		index = level + 1;
	       while (	index < vec.size() ){
		       if ( vec [index ].price >= vec[level].price || vec[index].price < 0){
			       vec.erase( vec.begin() + index );
			}
			else break;
		}
	       /* check top  indices (towards top of vector) of recent accessed index*/
		index = level ; 
		while ( --index >=0 && index < vec.size()){
			if ( vec [index ].price <= vec[level].price || vec[ index].price < 0 ){
				vec.erase( vec.begin() + index );
				level --;
			}else break;
		}
	}
	
	else{ /* sell */
		/* check bottom indices(towards end of vector) of recent accessed index */
		index = level + 1;
//		std::cout<<"\nSIZE"<<vec.size()<<std::endl;
	       while (	index < vec.size() ){
		       if ( vec [index ].price <= vec[level].price || vec[index].price < 0){
			       vec.erase( vec.begin() + index );
			}
			else break;
		}
	       /* check top  indices (towards top of vector) of recent accessed index*/
		index = level ; 
		while ( --index >=0 && index < vec.size()){
			if ( vec [index ].price >= vec[level].price || vec[ index].price < 0 ){
				vec.erase( vec.begin() + index );
				level --;
			}else break;
		}
	}
}

template<class T1, class T2>
bool LimitBook<T1,T2>::perform_delete( std::vector<T1>&vec,
		T1& elem, ssize_t level){

	bool   return_val;
	/* check whether the vector level has same value of elem or not */

	if ( elem.price != vec[level].price ){
		client_log<<"\nINFO.. The price [ "
		       <<vec[level].price
			<<" ] present in vector level ["
		       <<level
		       <<"  DOES NOT MATCH with delete req Price [ "
		       <<elem.price
		       <<" ] Searching vector for PRICE [ "<<elem.price
		       <<" ] "<<std::endl;
		typename std::vector<T1>::iterator it;
		it = find_if(vec.begin(),vec.end(), std::bind(special_compare<T1>,std::placeholders::_1 , elem));
		level = std::distance ( vec.begin() , it );
	}
	if ( elem.price == vec [level].price &&
		       level < vec.size()){
		if ( elem.size >= vec[level].size){
			vec.erase( vec.begin() + level);
		}
		else{
			vec[level].size -= elem.size;
		}
		return_val = true;
	}else{
		client_log<<"INFO .. Price "
		    <<elem.price
		    <<"Does not present in Vector \
		    Delete request failed"
		    <<std::endl;
	       	return_val = false;
	}
	return true;
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
template <class T1, class T2>
void LimitBook<T1,T2>::push_at_level( std::vector< T1 >& vec_, 
	       T1& elem_, ssize_t level_ , 
	       bool mismatch = false) {/* TODO remove mismatch */
/* 
	if ( mismatch ){
		std::cout<<"Are you kalu\n";
		auto  index = 
			find_if( vec_.begin(),vec_.end(),
		 		std::bind( special_compare<T>, std::placeholders::_1, elem_));
		if ( index != vec_.end() )
			vec_.erase(index + 1);
	}*/
	/*check whether the capacity of vector is less than
	  requested level or not.. if yes then increase capacity*/
	if( vec_.size() < level_  ){
		vec_.resize(level_ +1 , T1() );
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
template <class T1, class T2>
void LimitBook<T1,T2>::print_file ( std::vector<T1>& sell_order_book, 
	    std::vector<T1> &buy_order_book ) {
	
	client_log<<"----------------------------------------------------------------------\n";
	client_log<<"\t\t<-----BID----->\t<----SELL---->"<<std::endl;
	for ( size_t i = 0; i<sell_order_book.size() || 
			i < buy_order_book.size() ; i ++ ){

		if ( i <sell_order_book.size() && i < buy_order_book.size() ){
			/* TODO remove constant val 65535 and use price value
			 * to determine wrong value */

			client_log <<"INDEX [ "<<i <<" ] "<< sell_order_book[i].size << "\t" 
				<< sell_order_book[i].price 
			     <<"\tXXX\t"
			      << buy_order_book[i].price 
			      <<"\t"<<  buy_order_book[i].size
			     <<std::endl;
		}else if ( i < sell_order_book.size() ){
			client_log <<"INDEX [ "<<i <<" ] "<<sell_order_book[i].size << "\t" 
				<< sell_order_book[i].price 
				<<"\tXXX\tXXX\tXXX"<<std::endl;
		}else{
			client_log<<"INDEX [ "<<i <<" ] "<<"XXX\tXXX\tXXX\t"
				<< buy_order_book[i].price 
				<<"\t"<<buy_order_book[i].size 
				<<std::endl;
		}
	}
	client_log<<"----------------------------------------------------------------------\n\n\n";
	return;
}

template <class T1, class T2>
void LimitBook<T1,T2>::maintain_limit_book ( T2& obj ){

	++count;

	client_log<<"\nPROCESSING PACKET NO -- "<<count<<std::endl;
	client_log<<"sequence no  ["<<obj.seqno_
		<<" ] msg type [ "<<obj.msg_
		<<" ] side [  "<<obj.side_
		<<" ] level [ "<<(int)obj.level_
		<<" ] size ["<<(int)obj.size_
		<<" ] price [ "<<obj.price_
		<<" ] Contract [ "<<obj.contract_
		<<" ] "<<std::endl;


	T1 mkt_update(obj);
	market_map_iterator it;


	/*  check whether the security is already mapped 
	*  or not... TODO rather of using std::String as maped key
	try to use some int..for better performance */

	buy_sid *var;
	std::string secu = std::string(obj.contract_);
	it = markets.find( secu );
	if (it == markets.end() ){
		var = new (std::nothrow) buy_sid();
		if (!markets.insert(std::pair<std::string,buy_sid*>(secu , var)).second) {
			std::cerr<<"ERROR on creating new market book\n";
			 exit(EXIT_FAILURE);//FIXME don't want exit..kill me properly
			 
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
			  (void)push_at_level( var->sell_order_book, mkt_update, obj.level_  );
			  (void)do_stable_book(var->sell_order_book , obj.level_ , false);
			  break;
		  case 'B':/* bid side */
		  case 'b':
#ifdef DEBUG
			  std::cout << "Insert buy Level:" << (int)obj.level_ << std::endl;
#endif
			  (void)push_at_level(  var->buy_order_book, mkt_update, obj.level_ );
			  (void)do_stable_book(var->buy_order_book , obj.level_ , true);
			  break;
		  default:
			  client_log<<"ERROR wrong side,insert request..."<<std::endl;
			  break;
	      }
	      break;
	   case 1:/* delete/remove */
	      switch(obj.side_){
		 case 'S':/* sell side */
		 case 's':
			 if ( (size_t)obj.level_   < var->sell_order_book.size () ){
				 (void)perform_delete( var->sell_order_book, mkt_update , obj.level_ ); 
			 }
			 else {
				 client_log<<"ERROR Delete failed(Sell side)... Level [ " << (size_t)obj.level_ 
					 << " ] VecSize [ " << var->sell_order_book.size()
					<<" ] PRICE [ "<<obj.price_
					<<std::endl;
			 }
			 break;
		 case 'B':
		 case 'b':
			 if ((size_t)obj.level_ < var->buy_order_book.size() ){
				 (void)perform_delete( var->buy_order_book, mkt_update , obj.level_ ); 
			 }
			 else{ 
				 client_log<<"ERROR Delete failed(Bid side)... Level [ " << (size_t)obj.level_ 
					 << " ] VecSize [ " << var->sell_order_book.size()
					<<" ] PRICE [ "<<obj.price_
					<<std::endl;
			 }
			 break;
		 default:
			 client_log<<"ERROR ... wrong side, delete/remove request"
				 <<std::endl;
			 break;
	      }
	      break;

	   case 2:/* update */
	      switch(obj.side_){
		 case 'S':/*sell side*/
		 case 's':
			 if ( (size_t)obj.level_ < var->sell_order_book.size() ) {
				 var->sell_order_book[obj.level_].price=obj.price_;
				 var->sell_order_book[obj.level_].size=obj.size_;
			 }else {
				 /* that level is not present in the market book
				    * insert this data */
				 client_log << "INFO.. inserting the data while level"
					 <<"is not present for update(S)Level [  "
					 << (size_t)obj.level_ << " ] VecSize:"
					<<  var->sell_order_book.size()
					<<" ] "<< std::endl;
			 
#ifdef DEBUG_DELETE
				 client_log<<"##$$$DELETE-SELL-DEBUG$$$##"<<std::endl;
				 client_log<<"\nBOOK before UPDATE operation"<<std::endl;
				 (void)print_file( var->buy_order_book , var->sell_order_book );
#endif
				 (void)push_at_level( var->sell_order_book, mkt_update, obj.level_);
#ifdef DEBUG_DELETE
				 client_log<<"\nBOOk JUST After UPDATE"<<std::endl; 
				 (void)print_file( var->buy_order_book , var->sell_order_book );
				 (void)do_stable_book(var->sell_order_book ,obj.level_, false);
				 client_log<<"\nBOOk AFTER DO ADJUST"<<std::endl; 
				 (void)print_file( var->buy_order_book , var->sell_order_book );
				 client_log<<"##$$$END$$$##"<<std::endl;
#endif
			 }
			 (void)do_stable_book(var->sell_order_book ,obj.level_, false);
			 break;

		 case 'B':/*bid side*/
		 case 'b':
			 if ( (size_t)obj.level_  < var->buy_order_book.size() ) {
				 var->buy_order_book[obj.level_].price=obj.price_;
				 var->buy_order_book[obj.level_].size=obj.size_;
			 }else {
				 client_log << "INFO.. inserting the data while level"
					 <<"is not present for update(B) LEVEL [ "
					 << " ] " <<(size_t)obj.level_ << " ]  VecSize [ "
							   <<  var->buy_order_book.size()
							   <<" ] "<< std::endl;
#ifdef DEBUG_DELETE
				 client_log<<"##$$$DELETE-BID-DEBUG$$$##"<<std::endl;
				 client_log<<"\nBOOK before UPDATE operation"<<std::endl;
				 (void)print_file( var->buy_order_book , var->sell_order_book );
#endif
				 (void)push_at_level(  var->buy_order_book, mkt_update, obj.level_ );
#ifdef DEBUG_DELETE
				 client_log<<"\nBOOk JUST After UPDATE"<<std::endl; 
				 (void)print_file( var->buy_order_book , var->sell_order_book );
				 (void)do_stable_book<MktBook_t>(var->buy_order_book , obj.level_ , true);
				 client_log<<"\nBOOk AFTER DO ADJUST"<<std::endl; 
				 (void)print_file( var->buy_order_book , var->sell_order_book );
				 client_log<<"##$$$END$$$##"<<std::endl;
#endif
			 }
			 (void)do_stable_book(var->buy_order_book , obj.level_ , true);
			 break;
		 default:
			 client_log<<"ERROR .. wrong side for Update..."<<std::endl;
	      }
	      break;
	}/*end of outer switch */
	/* order book crossed ?*/
	do_market_analysis(var);
	/* print market stat after certain interval */
	if(  ( count % INTERVAL_COUNT ) == 0  ) {
		client_log<<"\nPrinting Limit Book after processing Packet [ "
			<<count<<" ] "<<std::endl;
		/* print each market book */
		
		for ( it = markets.begin(); it != markets.end(); ++it){
			client_log<<"\n PRINTING Stock of [ "
				<<it->first<<"] \n"<<std::endl;
			(void)print_file ( it->second->buy_order_book ,
					it->second->sell_order_book);
		}
	}
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


	  while( 1 ){
          
	  if ( cl_ptr->queue_empty && stop_worker  )return nullptr;

	  if( cl_ptr->pop_queue( &obj) ){
#ifdef DEBUG

		  cl_ptr->client_log<<"###sequence no ="<<obj.seqno_
			   <<"msg type="<<obj.msg_
			   <<"side = "<<obj.side_
			   <<"level="<<(int)obj.level_
			   <<"size= "<<(int)obj.size_
			   <<"price"<<obj.price_
			   <<"Sec"<<obj.contract_
			   <<std::endl;
#endif

		  (void)cl_ptr->maintain_limit_book(obj);
//		  MktBook_t mkt_update ( obj );

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
bool LimitBook<T1, T2>::do_market_analysis( buy_sid *var){


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
	LimitBook<T1,T2>::client_log.open( info.client_log , ios::out );
	if ( !LimitBook<T1,T2>::client_log.is_open()) return false;
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
  uint16_t prev_seqno = 0;
  int seq_no= 0, no=1,i=1,sockfd[2], portno, n, fd,flag=1;
  unsigned char buffer[256]="First ping\n";
  struct ExchangeA_MD  obj;
  pthread_mutex_t lock;
  struct sockaddr_in serv_addr[2];
  
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
		 
#ifdef DEBUG
		 cl_ptr->client_log<<"sequence no  ["<<obj.seqno_
			   <<" ] msg type [ "<<obj.msg_
			   <<" ] side [  "<<obj.side_
			   <<" ] level [ "<<(int)obj.level_
			   <<" ] size ["<<(int)obj.size_
			   <<" ] price [ "<<obj.price_
			   <<" ] "<<std::endl;
#endif
//		 obj.level_ -=1;
		 if ( obj.seqno_ <= prev_seqno );
		 else {
			 cl_ptr->push_queue(obj);
			 prev_seqno = obj.seqno_;
		 }
	    }
  }while(i!=0);
  try{
	  if ( sockfd[0]>0 )
		  close(sockfd[0]);
	  if ( sockfd[1] > 0 )
		  close(sockfd[1]);
  }
  catch ( const std::exception &e){
	  std::cerr<<"\nException cought at "
		  <<__FUNCTION__<<std::endl;
  }
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
