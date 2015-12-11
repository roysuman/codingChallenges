/*
 * =====================================================================================
 *
 *       Filename:  server_lookup.h
 *
 *    Description:  implementation of server...which handles packet loss
 *
 *        Version:  1.0
 *        Created:  Saturday 12 December 2015 01:04:50  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SIGCONT (suman roy), email.suman.roy@gmail.com
 *   Organization:  OPEN SOURCE
 *
 * =====================================================================================
 */

/*-----------------------------------------------------------------------------
 *Thread-1 is responsable to read packet from binary file and update into a Queue, say QUEUE1.
 *Thread-2 reads packet from  QUEUE1, and keep into a temporary storage{untill it get's a
 * response from client(client says that he has processed the packe)} before sending to client.
 * Thread-3 reads response from Clint....if client request to resnd lost packets...then thread-3 updates
 * this info to Thread-2.
 *
 * Client sends a response to server ufter processing each 100 packets.//TODO implementation at client end 
 *-----------------------------------------------------------------------------*/
 



#include "file.hpp"
#include "connection.hpp"
#include "structure.hpp"
#include "function.hpp"
#include <sys/time.h>
#include<vector>
#include<unistd.h>
#include<mutex>
#include<thread> /* next time use std::mutex lock */
#include<string>

#define QUEUE_SIZE 1000

/*-----------------------------------------------------------------------------
 *  A circuler queue class.... to maintain the data storage.....in client.cpp also 
 *  A queue has been created....TODO use only one queue class...u can define a 
 *  abstract class also....
 *-----------------------------------------------------------------------------*/

template<class T> 
class CircularQueue {

	private:
		ssize_t front, rear, max_size;
		std::vector<T> storage;
		pthread_mutex_t queue_lock;

	public:
		explicit CircularQueue(ssize_t size):queue_lock(PTHREAD_MUTEX_INITIALIZER) {
			this->front = this->rear = -1;
			this->max_size = size;
			this->storage.reserve(size);

		}
		virtual ~CircularQueue(){}
		int en_queue( T& value) {
			int return_val;
			if (this->is_full()) {
				std::cout << "queue is overflow\n";
				return_val = 0;
				
			} else {
				pthread_mutex_lock(&queue_lock);
				this->rear = (this->rear + 1) % this->max_size;
				this->storage[this->rear] = value;
				if (this->front == -1)
					this->front = this->rear;
				return_val = 1;
				pthread_mutex_unlock(&queue_lock);
			}
			return return_val;
		}

		int de_queue(T *value) {
			int return_val;
			if (this->is_empty()) {
				std::cout << "Queue is Empty\n";
				return_val = 0;
				
			} else {
				pthread_mutex_lock(&queue_lock);
				(*value) = this->storage[this->front];
				if (this->front == this->rear)
					this->front = this->rear = -1;
				else
					this->front = (this->front + 1) % this->max_size;
				pthread_mutex_unlock(&queue_lock);
				return_val= 1;
			}

			return return_val ;
		}
		inline int read_queue_from_position( T *value,ssize_t position){
			int return_val;
			if (this->is_empty()) {
				std::cout << "Queue is Empty\n";
				return_val = 0;
				
			} else {
				pthread_mutex_lock(&queue_lock);
				(*value) = this->storage[position % this->max_size ];
				pthread_mutex_unlock(&queue_lock);
				return_val = 1;
			}
			return return_val;;
		}
		inline void update_read_head( ssize_t position){
			pthread_mutex_lock(&queue_lock);
			this->front = (this->front + position ) % this->max_size;
			if (this->front == this->rear)
				this->front = this->rear = -1;
			pthread_mutex_unlock(&queue_lock);
		}
		inline ssize_t size() {
			pthread_mutex_lock(&queue_lock);
			return (this->max_size - this->front + this->rear + 1) % this->max_size;
			pthread_mutex_unlock(&queue_lock);
		}

		inline bool is_empty() {
			bool ret_val;
			pthread_mutex_lock(&queue_lock);
			ret_val = (this->front == -1)?true:false;
			pthread_mutex_unlock(&queue_lock);
			return ret_val;
		}

		inline bool is_full() {
			bool ret_val;
			pthread_mutex_lock(&queue_lock);
			ret_val = ((this->rear + 1) % this->max_size == this->front)?true:false;
			pthread_mutex_unlock(&queue_lock);
			return ret_val;
		}
 
};
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
		std::string config_file_name;
		CircularQueue<T> *global_storage_queue, *lookup_queue;
		bool start_sending_from_lookup;
		ssize_t lookup_seq;
		pthread_mutex_t look_up_var_lock ;
		void calc_stat(void);

	public:
		explicit Server(std::string config_file_name_):config_file_name(config_file_name_),
			start_sending_from_lookup(false),lookup_seq(-1),look_up_var_lock(PTHREAD_MUTEX_INITIALIZER),
			lookup_queue(nullptr),global_storage_queue(nullptr){}

		virtual ~Server(){
			if ( global_storage_queue != nullptr ) delete global_storage_queue;
			if ( lookup_queue != nullptr ) delete lookup_queue;
			if ( server_log.is_open())server_log.close();
		}
		bool create_servers();
		static void* communicate_with_client(void *);
		static void* read_and_store_market_data( void *);
		static void* maintain_lookup( void *);
		bool init_server(void);
		static bool send( T&,void*);
};
