/*
 * =====================================================================================
 *
 *       Filename:  createcluster.h
 *
 *       Description:  
 *
 *       Version:  1.0
 *       Created:  Saturday 27 February 2016 11:05:28  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *       Author:  SIGCONT (suman roy), email.suman.roy@gmail.com
 *       Organization:  OPEN SOURCE
 *       LICENSE: GNU GPL
 *  
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * =====================================================================================
 */
#ifndef CREATE_CLUSTER_H
#define CREATE_CLUSTER_H

#define MAXLEN 256
#include<iostream>
#include<fstream>


#include "double_list.h"

class File{
	public:
		explicit File(std::string file_name_):file_name(file_name_){}
		virtual ~File();
		bool open_file( );
		bool read_line( std::string& line);
		void close_file( void );
		
	private:
		std::ifstream file_handler;
		std::string file_name;
};

#include<vector>
#include<unordered_map>
typedef struct list_data_ list_data;
typedef std::unordered_map<std::string,list_data*> container_typedef;
typedef container_typedef::iterator container_iterator;
struct list_data_{
	size_t                     popularity;
	std::string                word;
	//FIXME now i am storing full line..
	//better idea is to store lines in some global
	//storage and list will contain indixes of global
	//storage of correspoonding data
	std::vector<std::string>   data; 
	container_typedef          child_container;
	list_head                  list_linkage; 
};
class Container{
	public:
		explicit Container(){
			INIT_LIST_HEAD( &HEAD);
		}
		virtual ~Container(void){}
	private:
		list_head     HEAD;
	public:
		bool insert_data( container_typedef& container, std::string word, std::string& line, bool pass);
		size_t print_n_data( size_t count, bool print_cluster_data = false);
		bool search_keyword( std::string& word, std::vector<std::string>& );
		container_typedef   container;
};

#include<map>
class ClusterManager{
	public:
		explicit ClusterManager(std::string& input_file_name_, std::string& filter_file_name_){
			container_ins = new Container( );
			input_file_name = input_file_name_;
			filter_file_name = filter_file_name_;
			init_filter();
		}
		virtual ~ClusterManager(void){
			delete container_ins;
		}
		bool process_data( void );
		size_t print_top_n_cluster( size_t count=0, bool print_cluster_data = false);
		bool search_keyword( std::string& word, std::vector<std::string>& );
	private:
		std::string    input_file_name;
		std::string    filter_file_name;
		Container*     container_ins;
		std::map<std::string,bool> filter; //no use of bool`
		bool init_filter(void);
};


class Config{
	private:
		std::string config_file_name;
	public:
		Config( std::string& config_file_name_):config_file_name(config_file_name_),flag(0x00){}
		virtual ~Config(void){}
		bool parse_config();
		void print_config( void ){
			std::cout<<"DATA_FILE_NAME : "<<input_data_file_name
				<<" \nFILTER_FILE_NAME : "<<input_filter_file_name
				<< "\nPRINT CLUSTER COUNT: "<<cluster_count_to_print
				<<"\nSEARCH COUNT: "<<keyword_search_cout
				<<"\nKEYWORDS to search...\n";
			size_t count =1; 
			for ( std::vector<std::string>::iterator it=search_keywords.begin(); it!= search_keywords.end();++it)
				std::cout<<(count++)<<". "<<*it<<"\n";
		std::cout<<std::endl;
		}
		inline bool get_data_file( std::string& file_name){
			file_name = input_data_file_name;
			return flag & 0x01;
		}
		inline bool get_filter_file( std::string& file_name){
			file_name = input_filter_file_name;
			return flag & 0x02;
		}
		inline bool get_cluster_max_count( size_t* count){
			*count = cluster_count_to_print;
			return flag & 0x04;
		}
		inline bool get_keywords( std::vector<std::string>& key){
			key = search_keywords;
			return flag & 0x08;
		}

	private:
		std::string input_data_file_name;
		std::string input_filter_file_name;
		size_t cluster_count_to_print;
		size_t keyword_search_cout;
		std::vector<std::string> search_keywords;
		unsigned int flag;
};
#endif
