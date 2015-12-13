// function.hpp
// last-modified :30/5/2013
//developed by : suman roy (email.suman.roy@gmail.com )
//
//
// Description   : This header file contains imp functions for serialize and deserialize data
// 
#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include"structure.hpp"
#include"header.hpp"
#define POWER7 0x7f
#define END 0x80
#define CONTINUE 0x00
#define prnt(s,n) for( int i=0; i< n; i++ ) std::cout<< (int) s[i] << "-"
int buffer_point=0;	
template<typename T>
T decode ( uint8_t* s, int& pos ) ;

template< typename T>
int encode ( T x, uint8_t* s ) {
	int num_ints = (sizeof(x)-1)/4 + 1;
	int si=buffer_point, res = 0;
	int *p = (int*)&x;
#ifdef DEBUG
	std::cout << __PRETTY_FUNCTION__ << " :\t" << x << " NumInts: " << num_ints ;
	printf( "Hex: \t0x%x 0x%x\n", *((uint32_t*)&x), *((uint32_t*)&x + 1));
#endif
	for ( int i=0; i<num_ints; i++ ) {
		int t = p[i];
		if ( i == num_ints - 1 ) {/*last int-byte may not be of 4 bytes. so copying data*/
			t=0;
			memcpy((void*)&t, (void*)&p[i], sizeof(x) - i*sizeof(int));
		}
#ifdef DEBUG
		std::cout << "# t: " << t << "\t";
#endif
		if ( !t ) s[si++] = (END | t) ;
		else 
			while ( t > 0 ){
				res = t & POWER7;
				s[si++] = ( t>POWER7? CONTINUE : END) | res;
				t>>=7;
#ifdef DEBUG
				printf( "%d-",s[si-1]);
#endif
			}
	}
#ifdef DEBUG
	std::cout << endl;
#endif
	int t=buffer_point;
#ifdef DEBUG
	std::cout << "On Decode We get: " 
		<< decode<T>(s, t) << endl; 
#endif
	return si;
}


template<typename T>
T decode ( uint8_t* s, int& pos ) {
	T x = 0; int num_ints = (sizeof(x)-1)/4 + 1;
	int *r=(int *)&x;
	int i=pos, mp=0;
	for ( int j=0; j<num_ints; j++ ) {
#ifdef DEBUG
		printf("s[i]: (%d) %d\n", i, s[i] );
#endif
		while ( ! (s[i] & END) ) {
			r[j] |= (s[i++] & ~END)<<mp;
			mp+=7;
		}
		r[j] |= (s[i++] & ~END)<<mp;
	}
	pos=i;
	return x;
}

int 
serialize(struct ExchangeA_MD &temp_str_, uint8_t * buffer){
	buffer_point=2;
#ifdef DEBUG
	std::cout<<"inside func_msg: MsgType: "
		<<temp_str_.msg_<<std::endl;
#endif
	switch(temp_str_.msg_){
		case 0: /* for insert new packet */
			buffer[1]=temp_str_.msg_ ;
			buffer_point = encode<uint16_t>(temp_str_.seqno_,buffer);
			memcpy((void*)(buffer+buffer_point), (void*)temp_str_.contract_, strlen(temp_str_.contract_));
			buffer_point += strlen(temp_str_.contract_);
			buffer[buffer_point++] = '\0';
			buffer[buffer_point++]=temp_str_.level_;
			buffer_point = encode<double>(temp_str_.price_,buffer);
			buffer_point = encode<uint16_t>(temp_str_.size_, buffer);
			buffer[buffer_point++]=temp_str_.side_;
			buffer[0] = buffer_point;
			break;

		case 1:/* for delete packet */
			buffer[1]=temp_str_.msg_;
			buffer_point = encode<uint16_t>(temp_str_.seqno_,buffer);
			memcpy((void*)(buffer+buffer_point), (void*)temp_str_.contract_, strlen(temp_str_.contract_));
			buffer_point += strlen(temp_str_.contract_);
			buffer[buffer_point++]='\0';
			buffer[buffer_point++]=temp_str_.level_;
			buffer[buffer_point++]=temp_str_.side_; 
			buffer[0]=buffer_point;  
			break;

		case 2:/* for update packet */
			buffer[1]=temp_str_.msg_ ;
			buffer_point = encode<uint16_t>(temp_str_.seqno_,buffer);
			memcpy((void*)(buffer+buffer_point), (void*)temp_str_.contract_, strlen(temp_str_.contract_));
			buffer_point += strlen(temp_str_.contract_);
			buffer[buffer_point++] = '\0';
			buffer[buffer_point++]=temp_str_.level_;
			buffer_point = encode<double>(temp_str_.price_,buffer);
			buffer_point = encode<uint16_t>(temp_str_.size_, buffer);
			buffer[buffer_point++]=temp_str_.side_;
			buffer[0] = buffer_point;
			break;
	}
	return buffer_point;
}

struct ExchangeA_MD 
deserialize(unsigned char a[]){
	int i=2;/* 0 is size of the char */
	ExchangeA_MD recv_md;
	int size = (int)a[0] ;
#ifdef DEBUG
	std::cout<< "+="<< __FUNCTION__ << "\t\tSize: " << size << std::endl;
#endif
	switch(a[1]){
		case 0: /*  for insert new packet */
			recv_md.msg_=(ExchangeA_MsgType)a[1]; 
			recv_md.seqno_= decode<uint16_t>(a, i);
			strcpy( (char*)recv_md.contract_, (char*)(a+i) );
			i += ( strlen( (char*)&a[i] ) + 1 );
			recv_md.level_= (uint8_t)a[i++];
			recv_md.price_= decode<double>(a, i);
			recv_md.size_ = decode<uint16_t>(a, i);
			recv_md.side_=(char)a[i];
			break;

		case 1:/*  for delete packet */
			recv_md.msg_=(ExchangeA_MsgType)a[1];
			recv_md.seqno_=decode<uint16_t>(a,i);
			strcpy( (char *)recv_md.contract_,(char *)(a+i) );
			i +=( strlen( (char *)&a[i] ) + 1);
			recv_md.level_=(uint8_t)a[i++];
			recv_md.side_=(char)a[i];
			break;

		case 2:/*  for update packet */
			recv_md.msg_=(ExchangeA_MsgType)a[1];
			recv_md.seqno_= decode<uint16_t>(a, i);
			strcpy( (char*)recv_md.contract_, (char*)(a+i) );
			i += ( strlen( (char*)&a[i] ) + 1 );
			recv_md.level_= (uint8_t)a[i++];
			recv_md.price_= decode<double>(a, i);
			recv_md.size_ = decode<uint16_t>(a, i);
			recv_md.side_=(char)a[i];
			break;
			
	}
#ifdef DEBUG
	stru_print(recv_md);
#endif
	return recv_md;
}
#endif //_EUNCTION_H_
