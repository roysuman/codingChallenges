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
#include "connection.hpp"
#include "structure.hpp"
#include "function.hpp"
#include "header.hpp"
#include "file.hpp"
#include <algorithm>
#include<functional>
#include<mutex>
#include<queue>
#include<thread>
#include<unordered_map>
typedef struct buy_sid_{
  std::vector <MktBook_t> buy_order_book,sell_order_book;

}buy_sid;
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
	    std::vector<MktBook_t> &buy_order_book ,
	    fstream &file ) {
	
	file<<"----------------------------------------------------------------------\n";
	for ( size_t i = 0; i<sell_order_book.size() || i < buy_order_book.size() ; i ++ ){
		if ( i <sell_order_book.size() && i < buy_order_book.size() ){
			file << ( (sell_order_book[i].size == 65535 )? 0 : sell_order_book[i].size) << "\t" << ( ( sell_order_book[i].price < 0 )? 0 :sell_order_book[i].price)
			     <<"\tXXX\t"
			      << ( (buy_order_book[i].price < 0) ? 0 : buy_order_book[i].price)
			      <<"\t"<<  ((buy_order_book[i].size== 65535 )? 0 : buy_order_book[i].size)
			     <<std::endl;
		}else if ( i < sell_order_book.size() ){
			file << ( (sell_order_book[i].size == 65535 )? 0 : sell_order_book[i].size) << "\t" << ( ( sell_order_book[i].price < 0 )? 0 :sell_order_book[i].price)
				<<"\tXXX\tXXX\tXXX"<<std::endl;
		}else{
			file<<"XXX\tXXX\tXXX\t"<< ((buy_order_book[i].price < 0) ? 0 : buy_order_book[i].price)<<"\t"<<( (buy_order_book[i].size== 65535 )? 0 : buy_order_book[i].size) <<std::endl;
		}
	}
	return;
}

int main(int argc, char *argv[]){
	int prev = 0;
	bool mismatch = false;
	struct timeval tv1,tv2;
	int  count_p1_prio=0 , count_p2_prio=0;  
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
	fd=open(info.path, ios::in | ios::out | ios::binary);
	if(fd<0) {
		std::cerr<<"file cant open\n";
		exit ( EXIT_FAILURE ) ;
	}
	struct ExchangeA_MD  obj;
	sockaddr_in there_addr; /*client address*/
  int len=sizeof(there_addr);
  gettimeofday(&tv1,NULL);
  size_t count = 0;

  std::unordered_map<std::string,buy_sid*> markets;
  std::unordered_map<std::string,buy_sid*>::iterator it;
 


  do{
	  /*read data from bin file*/
	  flag=read(fd,(void *)&obj, sizeof(struct ExchangeA_MD));
	  if(flag<0)
		  std::cerr<<"\nreading error\n";
	  else{
		 uint8_t *temp_send=new uint8_t[sizeof(obj)];
		 /*serialize data */
		 //server_log<<obj.seqno_<<":"<<(int)obj.level_<<std::endl;


		  MktBook_t mkt_update_ ( obj );
		  
		  obj.level_ -=1;
		  if ( obj.seqno_ - 1 != prev){
			  std::cout<<obj.seqno_<<"::"<<prev<<std::endl;
			  std::cerr<<"MISMATCH\n";
			  exit(0);
		  }
		  prev = obj.seqno_;
#ifdef DEBUG
		  server_log<<"sequence no ="<<obj.seqno_
			   <<"msg type="<<obj.msg_
			   <<"side = "<<obj.side_
			   <<"level="<<(int)obj.level_
			   <<"size= "<<(int)obj.size_
			   <<"price"<<obj.price_
			   <<"Contract"<<obj.contract_
			   <<std::endl;
#endif
		  std::string secu = std::string(obj.contract_);
		  buy_sid *var;
		  it = markets.find(secu );
		  if ( it == markets.end() ){
			  var = new buy_sid();
			  markets.insert(std::pair<std::string,buy_sid*>( secu ,var));}
		  
		  else{
			  var = it->second; 
		  }

		  count++;
		  switch(obj.msg_){
		      case 0:/*insert*/
			   switch(obj.side_){
			      case 'S':/* sell side */
			      case 's':
#ifdef DEBUG
				      std::cout << "Insert Sell Level:" << (int)obj.level_ << endl;
#endif
				      push_at_level<MktBook_t> ( var->sell_order_book, mkt_update_, obj.level_ ,  &mismatch );
				      break;
			      case 'B':/* bid side */
			      case 'b':
#ifdef DEBUG
				      std::cout << "Insert buy Level:" << (int)obj.level_ << std::endl;
#endif
				      push_at_level<MktBook_t> ( var->buy_order_book, mkt_update_, obj.level_ , mismatch );
				      break;
			      default:
				      std::cerr<<"ERROR wrong side,insert request..."<<std::endl;
					   //exit(0);
				      break;
			   }
			   break;

		      case 1:/* delete/remove */
			   switch(obj.side_){
				   case 'S':/* sell side */
				   case 's':
					   if ( obj.level_ <= var->sell_order_book.size () ){
					//	   search_and_remove(
						   var->sell_order_book.erase(var->sell_order_book.begin()+obj.level_);
					   }
					   else {
						   std::cerr<<"ERROR Delete failed(Sell side)... Level" << (int)obj.level_ 
							   << " VecSize:" << var->sell_order_book.size() 
							   <<std::endl;
					  // exit(0);
					  }
					   break;
				   case 'B':
				   case 'b':
					   if ( obj.level_ <= var->buy_order_book.size() )
						   var->buy_order_book.erase( var->buy_order_book.begin()+obj.level_);
					   else{ 
						   std::cerr << "ERROR Delete failed(bid side)...Level " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  var->buy_order_book.size() 
							   <<std::endl;
					  // exit(0);
					  }
					   break;
				   default:
					   std::cerr<<"ERROR wrong side, delete/remove request"
						   <<std::endl;
					  // exit(0);
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
						   push_at_level<MktBook_t> ( var->sell_order_book, mkt_update_, obj.level_ , mismatch );
						   std::cerr << "Error.. inserting the data while level \
							   is not present for update(S) " 
							   << (int)obj.level_ << " VecSize:" 
							   <<  var->sell_order_book.size()<< std::endl;
					//	   exit(0);
					   }
					   break;
				   case 'B':/*bid side*/
				   case 'b':
					   if ( obj.level_ <= var->buy_order_book.size() ) {
						   var->buy_order_book[obj.level_].price=obj.price_;
						   var->buy_order_book[obj.level_].size=obj.size_;
					   }
					   
					   else {
						   push_at_level<MktBook_t> (  var->buy_order_book, mkt_update_, obj.level_ ,mismatch);
						   std::cerr << "Error.. inserting the data while level \
							   is not present for update(B) "
							   << (int)obj.level_ << " VecSize:"
							   << var-> buy_order_book.size()<< std::endl;
						  // exit(0);
					   }
					   break;
				   default:
					   std::cerr<<"wrong side for Update..."<<std::endl;
					  // exit(0);
			   }
			   break;
		   }
		   /* do the market analysis  */
		  if (  var->buy_order_book.size() >= 1 && var->sell_order_book.size()>= 1) 
		  std::cout<<var->buy_order_book[0].price<<"::"<<var->sell_order_book[0].price<<std::endl;
		   if (  (var->buy_order_book.size() >= 1) && (var->sell_order_book.size()>= 1 ) && ( var->buy_order_book[0].price >= var->sell_order_book[0].price )) {
			   std::cout<<"Atlast\n";
			// print_file ( buy_order_book , sell_order_book , server_log );
			 int dif1 = (int)var->buy_order_book[0].size - (int)var->sell_order_book[0].size;
			 if ( dif1 == 0 ){
				 var->sell_order_book.erase( var->sell_order_book.begin() + 0);
				   var->buy_order_book.erase( var->buy_order_book.begin() + 0 );

			 }else if ( dif1 > 0 ){
				 var->buy_order_book[0].size -=var->sell_order_book[0].size;
				 var->sell_order_book.erase( var->sell_order_book.begin() + 0);
			 }else if ( dif1 < 0 ){
				 var->sell_order_book[0].size -=var->buy_order_book[0].size;

				   var->buy_order_book.erase( var->buy_order_book.begin() + 0 );

			 }

		   }
		   if( count % 100 == 0  ) {
			   server_log<<"\n#######count=\t"<<count<<"  ##########"<<std::endl;
			 for ( it = markets.begin(); it != markets.end(); ++it){
				 server_log<<"\n PRINTING Stock of [ "<<it->first<<"] \n"<<std::endl;
				 print_file ( it->second->buy_order_book , it->second->sell_order_book , server_log );
			 }
		  } 
	  }
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

