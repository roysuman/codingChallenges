/* client.cpp
 * last-modified : 30/5/2013
 * developed by : Suman roy (email.suman.roy@gmail.com)
 * this is client side programm
 * used by client to connect with server
 * takes a file containg info like IP,port of server and client log file path
 *it receives server data and maintain a Mktbook
 */

#include "connection.hpp"
#include "structure.hpp"
#include "function.hpp"
#include "header.hpp"
#include "file.hpp"
#include <algorithm>
#include<functional>
#define TEST 1
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
void 
push_at_level( std::vector< T >& vec_, T& elem_, int level_ , bool mismatch) {

	if ( mismatch ){
		/* check whether the same value is present or not */
		/*if present then delete*/
		auto  index = find_if( vec_.begin(),vec_.end(),std::bind( special_compare<T>, std::placeholders::_1, elem_));
		if ( index != vec_.end() )
			vec_.erase(index);
	}
	/*check whether the capacity of vector is less than
	  requested level or not.. if yes then increment capacity*/
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
void 
print_book ( std::vector<MktBook_t>& sell_order_book, std::vector<MktBook_t> buy_order_book ) {

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
void 
print_file ( std::vector<MktBook_t>& sell_order_book, 
	    std::vector<MktBook_t> buy_order_book ,
	    fstream &file ) {
	
	file<<"----------------------------------------------------------------------\n";
	for ( size_t i = 0; i<sell_order_book.size() || i < buy_order_book.size() ; i ++ ){
		if ( i <sell_order_book.size() && i < buy_order_book.size() ){
			file << sell_order_book[i].size << "\t" << sell_order_book[i].price
			     <<"\tXXX\t"
			     << buy_order_book[i].size << "\t" << buy_order_book[i].price
			     <<std::endl;
		}else if ( i < sell_order_book.size() ){
			file<<sell_order_book[i].size<<"\t"
			    <<sell_order_book[i].price
			    <<"\tXXX\tXXX\tXXX"<<std::endl;
		}else{
			file<<"XXX\tXXX\tXXX\t"<<buy_order_book[i].size
			    <<"\t"<<buy_order_book[i].price<<endl;
		}
	}
	return;
}


int main(int argc, char *argv[])
{
  fstream client_log;
  int count=1; 
  uint16_t prev_seqno;
  int seq_no=0, no=1,i=1,sockfd[2], portno, n, fd,flag=1;
  unsigned char buffer[256]="First ping\n";
  struct ExchangeA_MD  obj;
  std::vector <MktBook_t> buy_order_book,sell_order_book;
  if (argc < 2) {
	  std::cerr<<"usage "<<argv[0]<<"configuration file\n";
	  exit(0);  
  }

  /*-----------------------------------------------------------------------------
   *  opena nd process configuration file
   *-----------------------------------------------------------------------------*/
  File_ fp(argv[1]); /* this file has defined at header file file.hpp*/
  struct ClientInfo info=fp.ClientCredentials();/*to open client cofiguration file*/
  struct sockaddr_in serv_addr[2]; 
  for ( int i=0; i<2; i++ ) {
    bzero((char *) &serv_addr[i], sizeof(serv_addr));
    serv_addr[i].sin_family = AF_INET;
    serv_addr[i].sin_addr.s_addr = inet_addr((i==0?info.ip1:info.ip2));
    serv_addr[i].sin_port = htons((i==0?info.port1:info.port2)); // set these fields will be used later
    sockfd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd[i] < 0) 
	    std::cerr<<"ERROR opening socket...";
  }
  /*open client log file*/
  client_log.open( info.client_log , ios::out );
 
  socklen_t struct_size = 0;
 
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
  do {
      bool mismatch = false;
      memcpy(&working_set, &master_set, sizeof(master_set));
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
      if ( rc < 0 ) { 
	   perror ( " selec() failed.\n");
	   exit(0);
	}else if (rc == 0){
	   perror("No data received in last 1 minute..");
	   break;
	}
      else
	 for ( int i=0; i<max_sd; i++ ) 
   	    if ( FD_ISSET(i, &working_set) ) {
		 int curr_sockfd = i==sockfd[0] ? sockfd[0]
			 :i==sockfd[1] ? sockfd[1] : -1;
		 uint8_t buf_size_to_read = 0;
		 n = recvfrom(curr_sockfd, (void*)&buf_size_to_read, 1, 0, NULL, 0);
		 n = recvfrom(curr_sockfd,(void*)buffer,(int) buf_size_to_read, 0, NULL, 0);
		 if (n < 0) { std::cerr << "ERROR reading from socket\n"; exit(0); }
		 obj=deserialize(buffer);
#ifdef DEBUG
		 std::cout<<"Print recv data after deserialized\n";
		 stru_print(obj);
#endif
		 MktBook_t mkt_update_ ( obj );
		 
		 if(  obj.seqno_ > seq_no ){
			 if ( seq_no + 1 != obj.seqno_ ){
				 print_file ( buy_order_book , sell_order_book , client_log );
				 std::cerr<<seq_no<<"MISMATCH"<<obj.seqno_;
				 mismatch = true;
			 }
		   count++;
#ifdef DEBUG
		   std::cout<<"count="<<count<<std::endl;
		   std::cout<<"seq"<<obj.seqno_<<std::endl;
#endif
		   seq_no=obj.seqno_;
		   obj.level_-= 1;//FIXME why level starting from 1?
#ifdef TEST
		   client_log<<"sequence no ="<<obj.seqno_
			   <<"msg type="<<obj.msg_
			   <<"side = "<<obj.side_
			   <<"level="<<(int)obj.level_
			   <<"size= "<<(int)obj.size_
			   <<"price"<<obj.price_
			   <<std::endl;
#endif
		   switch(obj.msg_){
		      case 0:/*insert*/
			   switch(obj.side_){
			      case 'S':/* sell side */
			      case 's':
#ifdef DEBUG
				      std::cout << "Insert Sell Level:" << (int)obj.level_ << endl;
#endif
				      push_at_level<MktBook_t> ( sell_order_book, mkt_update_, obj.level_ ,  &mismatch );
				      break;
			      case 'B':/* bid side */
			      case 'b':
#ifdef DEBUG
				      std::cout << "Insert buy Level:" << (int)obj.level_ << std::endl;
#endif
				      push_at_level<MktBook_t> (  buy_order_book, mkt_update_, obj.level_ , mismatch );
				      break;
			      default:
				      std::cerr<<"ERROR wrong side,insert request..."<<std::endl;
				      break;
			   }
			   break;

		      case 1:/* delete/remove */
			   switch(obj.side_){
				   case 'S':/* sell side */
				   case 's':
					   if ( obj.level_ <= sell_order_book.size () ){
					//	   search_and_remove(
						   sell_order_book.erase(sell_order_book.begin()+obj.level_);
					   }
					   else 
						   std::cerr<<"ERROR Delete failed(Sell side)... Level" << (int)obj.level_ 
							   << " VecSize:" << sell_order_book.size() 
							   <<std::endl;
					   break;
				   case 'B':
				   case 'b':
					   if ( obj.level_ <= buy_order_book.size() )
						   buy_order_book.erase(buy_order_book.begin()+obj.level_);
					   else 
						   std::cerr << "ERROR Delete failed(bid side)...Level " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  buy_order_book.size() 
							   <<std::endl;
					   break;
				   default:
					   std::cerr<<"ERROR wrong side, delete/remove request"
						   <<std::endl;
					   break;
			   }
			   break;
		      case 2:/* update */
			   switch(obj.side_){
				   case 'S':/*sell side*/
				   case 's':
					   if ( obj.level_ <= sell_order_book.size() ) {
						   sell_order_book[obj.level_].price=obj.price_;
						   sell_order_book[obj.level_].size=obj.size_;
					   }
					   else {
						   /* that level is not present in the market book
						    * insert this data */
						   push_at_level<MktBook_t> ( sell_order_book, mkt_update_, obj.level_ , mismatch );
						   std::cerr << "Error.. inserting the data while level \
							   is not present for update(S) " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  sell_order_book.size()<< std::endl;
					   }
					   break;
				   case 'B':/*bid side*/
				   case 'b':
					   if ( obj.level_ <= buy_order_book.size() ) {
						   buy_order_book[obj.level_].price=obj.price_;
						   buy_order_book[obj.level_].size=obj.size_;
					   }
					   
					   else {
						   push_at_level<MktBook_t> (  buy_order_book, mkt_update_, obj.level_ , mismatch );
						   std::cerr << "Error.. inserting the data while level \
							   is not present for update(B) "
							   << (int)obj.level_ << " VecSize:"
							   <<  buy_order_book.size()<< std::endl;
					   }
					   break;
				   default:
					   std::cerr<<"wrong side for Update..."<<std::endl;
			   }
			   break;
		   }
		   //std::cout<<buy_order_book.size()<<"::"<<sell_order_book.size()<<std::endl;
		   
		   /* do the market analysis  */
		   if (  buy_order_book.size() >= 1 && sell_order_book.size()>= 1  && ( buy_order_book[0].price >= sell_order_book[0].price )) {
			   size_t count = buy_order_book[0].size;
			   buy_order_book[0].size -= sell_order_book[0].size;
			   sell_order_book[0].size -= count;
			   if ( buy_order_book[0].size <= 0 )
				   buy_order_book.erase( buy_order_book.begin() + 0 );
			   if ( sell_order_book[0].size<= 0 )
				   sell_order_book.erase( sell_order_book.begin() + 0);
		   }
		 }
		   if ( mismatch ){
		   print_file ( buy_order_book , sell_order_book , client_log );
		   mismatch = false;
			   

		   } 

		 /*-----------------------------------------------------------------------------
		  *print the book info after certain processing count
		  *-----------------------------------------------------------------------------*/
		 if( count%0 == 0  ) {
#ifdef DEBUG
			 std::cout<<"count=\t"<<count<<std::endl;
#endif
		//	 print_file ( buy_order_book , sell_order_book , client_log );
		 }
		 /* un-comment if you want to print @stdout */
		 // print_book ( buy_order_book, sell_order_book );
	    }
  } while(i!=0);
  close(sockfd[0]);close(sockfd[1]);
  client_log.close();
  return 0;
}
  
  
