/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  header for client
 *
 *        Version:  1.0
 *        Created:  Friday 11 December 2015 11:16:28  IST
 *       Revision:  none
 *       Compiler:  g++4.7
 *
 *         Author:  Suman Roy (), suman.roy@cognizant.com
 *   Organization:  Cognizant Technological Solutions
 *
 * =====================================================================================
 */

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
#define TEST 1
//#define DEBUG 1
#ifdef DEBUG
#define INTERVAL_COUNT 1
#else
#define INTERVAL_COUNT 100
#endif
typedef struct buy_sid_{
  std::vector <MktBook_t> buy_order_book,sell_order_book;

}buy_sid;

typedef std::unordered_map<std::string,buy_sid*> market_map;
typedef market_map::iterator market_map_iterator;
/*-----------------------------------------------------------------------------
 *  queue implementation
 *-----------------------------------------------------------------------------*/
template <class T2>
class Queue{
	private:
		std::queue<T2> data_queue;
		pthread_mutex_t queue_lock;
	protected:
		bool queue_empty;
	public:
		explicit Queue():queue_empty(false),queue_lock(PTHREAD_MUTEX_INITIALIZER){}
		virtual inline bool push_queue( T2 &data){
			pthread_mutex_lock(&queue_lock);
			queue_empty = false;
			data_queue.push(data);
			pthread_mutex_unlock(&queue_lock);
			return true;
		}
		bool pop_queue( T2* data){
			bool return_val;
			return_val = false;

			pthread_mutex_lock(&queue_lock);

			if ( !data_queue.empty() ){
				(*data) = data_queue.front();
				data_queue.pop();
				pthread_mutex_unlock(&queue_lock);
#ifdef DEBUG
				std::cout<<"POPED FROM QUEUE\n";
				std::cout<<"sequence no ="<<data->seqno_
					<<"msg type="<<data->msg_
					<<"side = "<<data->side_
					<<"level="<<(int)data->level_
					<<"size= "<<(int)data->size_
					<<"price"<<data->price_
					<<std::endl;
#endif
				return_val =  true;
			}
			if(!return_val){
				pthread_mutex_unlock(&queue_lock);
				queue_empty = true;
			}
			return return_val;
		}
};

template< class T1, class T2>
class LimitBook{
	private:
		static market_map markets;
		void do_stable_book( std::vector<T1>&, size_t ,bool);
		bool perform_delete( std::vector<T1>&, T1&,ssize_t);
		void push_at_level( std::vector<T1>&,T1&,ssize_t,bool);
		void print_file( std::vector<T1>&,std::vector<T1>&);
		size_t count;
		bool do_market_analysis(buy_sid *var);
	public:
		std::fstream client_log;
		void maintain_limit_book( T2&);
		explicit LimitBook( ):count(0){}//:client_log(fd){}
		virtual ~LimitBook(){
			markets.erase( markets.begin() , markets.end() );
			if ( client_log.is_open())
				client_log.close();
		}
};



template <class T1,class T2>
class Client:private Queue<T2>,private LimitBook<T1,T2>{
	private:
		std::string config_file;
		Queue<T2> data_store;
		//std::fstream client_log;
		struct ClientInfo info;
		static bool stop_worker;
	public:
		explicit Client(std::string config_file_):config_file(config_file_){}
		virtual ~Client(){};

		static void* get_data(void*);
		static void* worker(void*);
		bool init_client( void );
};
