/*
 * =====================================================================================
 *
 *       Filename:  createcluster.cpp
 *
 *       Description:  
 *
 *       Version:  1.0
 *       Created:  Saturday 27 February 2016 11:10:30  IST
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

#include "createcluster.h"
#include<string.h>


/*
 * trim: get rid of trailing and leading whitespace...
 *       ...including the annoying "\n" from fgets()
 */
static inline char *
trim (char * s)
{
  /* Initialize start, end pointers */
  char *s1 = s, *s2 = &s[strlen (s) - 1];

  /* Trim and delimit right side */
  while ( (isspace (*s2)) && (s2 >= s1) )
    s2--;
  *(s2+1) = '\0';

  /* Trim left side */
  while ( (isspace (*s1)) && (s1 < s2) )
    s1++;

  /* Copy finished string */
  strcpy (s, s1);
  return s;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ~File
 *  Description:  destructor of file handler...close the file if previously
 *                opened
 * =====================================================================================
 */
File::~File(){
	if ( file_handler.is_open()){
		file_handler.close ();
	}
	return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  open_file
 *  Description:  open the file
 *              
 * =====================================================================================
 */
bool 
File::open_file( void ){
	file_handler.open(file_name.c_str(),std::ifstream::in);
	return file_handler.is_open();

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  read_line
 *  Description:  read next line from file
 *              @line: reference to hold the read line
 *              @return true on successfull read else returns false.
 * =====================================================================================
 */
bool
File::read_line( std::string &line){
	return getline( file_handler, line);
}

void
File::close_file( void ){
	file_handler.close();
}


/*-----------------------------------------------------------------------------
 *  container class implementation
 *-----------------------------------------------------------------------------*/
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  insert_data
 *  Description:  insert data into container. each container list can have another container
 *                hashing....to maintain relationship
 * =====================================================================================
 */


#include<sstream>
bool
Container::insert_data(container_typedef& container, std::string word,std::string& line, bool pass){
	container_iterator   it;
	list_data    *popularity_met;//= new list_data();
	list_head  *current_list;
	list_data* my_data,*current_data;
	bool      return_val = true;
	std::istringstream iss(line);
	std::string word2;
#ifdef DEBUG
	std::cout<<"INSERT DATA INTO CONTAINER Word :\""<<word<<"\" Line : "<<line<<std::endl;
	std::cout<<"CONTAINER SIZE"<<container.size()<<std::endl;
#endif
	//check word already present or not
	it = container.find(word);
#ifdef DEBUG
	std::cout<<"PRINT CONTAINER KEYWORD"<<std::endl;
	for ( container_iterator it = container.begin(); it!= container.end();++it){
		std::cout<<it->first<<std::endl;
	}
#endif
	if ( it ==container.end()){//new entry
#ifdef DEBUG
		std::cout<<"New entry"<<std::endl;
		std::cout<<"SIZE"<<popularity_met->child_container.size()<<std::endl;
#endif
		list_data    *popularity_met= new list_data();
		popularity_met->popularity = 1;
		popularity_met->word = word;
		popularity_met->data.push_back(line);
		(void)list_add_tail( &(popularity_met->list_linkage),&HEAD);
		return_val = container.insert(std::pair<std::string,list_data*>(word,popularity_met)).second;
		 while(iss>>word2 && return_val && !pass){
#ifdef DEBUG
			std::cout<<"ROUND 2"<<word2<<std::endl;
			std::cout<<popularity_met->child_container.size()<<std::endl;
#endif
			return_val= insert_data( popularity_met->child_container , word2,line,true);
		}

	}else{
#ifdef DEBUG
		std::cout<<"Existing entry"<<std::endl;
#endif
		popularity_met =(list_data*) it->second;
		my_data = get_member_struct( &popularity_met->list_linkage,list_data,list_linkage);
		popularity_met->popularity++;
		popularity_met->data.push_back(line);
		current_list = popularity_met->list_linkage.prev;
#ifdef DEBUG
		std::cout<<"Current List = "<<popularity_met->popularity<<std::endl;
#endif
		
		bool found = false;
		while ( current_list != &HEAD && !found){
			current_data = get_member_struct( current_list,list_data,list_linkage);
			if ( current_data->popularity < my_data->popularity){
				current_list= current_list->prev;
			}else found = true;
		}
		popularity_met->list_linkage.prev->next =popularity_met->list_linkage.next;
		popularity_met->list_linkage.next->prev= popularity_met->list_linkage.prev;
		
		popularity_met->list_linkage.prev = current_list;
		popularity_met->list_linkage.next = current_list->next;
		current_list->next->prev = &popularity_met->list_linkage;
		current_list->next = &popularity_met->list_linkage;
		
		while(iss>>word2 && return_val && !pass){
#ifdef DEBUG
			std::cout<<"ROUND 2"<<word2<<std::endl;
			std::cout<<popularity_met->child_container.size()<<std::endl;
#endif
			return_val= insert_data( popularity_met->child_container , word2,line,true);
		}
	}
	return return_val;
}

size_t
Container::print_n_data( size_t count , bool print_cluster_data ){
	list_head *attr_list_temp, *temp_list_store;
	size_t count_cluster = 0;
	list_data* current_data, *c_data;
	list_for_each_safe ( attr_list_temp, temp_list_store, &HEAD){
		current_data =(list_data*) get_member_struct( attr_list_temp,list_data,list_linkage);
		if ( current_data->child_container.size() == 0 ) continue;
		std::cout<<"----------------\n";
		std::cout<<"#CLUSTER \""<<current_data->word
			<<"\" RANNK "<<current_data->popularity
			<<" KEYWORDS :"<<current_data->data.size()
			<<std::endl;
		for( container_iterator it = current_data->child_container.begin();
				it!= current_data->child_container.end();++it){
			c_data = it->second;

			std::cout<<" SUB_CLUSTER \""<<c_data->word
				<<"\" RANNK "<<c_data->popularity
				<<" KEYWORDS :"<<c_data->data.size()
				<<std::endl;
			
		}
		count_cluster++;
		if ( print_cluster_data){
			std::cout<<"Data.....\n";
			for ( size_t loop = 0; loop< current_data->data.size() ;++loop){
				std::cout<<loop+1<<". "<<current_data->data[loop]<<std::endl;
			}
		}
		if ( count_cluster == count )
			break;
		
	}
		return count_cluster;

}

bool
Container::search_keyword(std::string& key_word, std::vector<std::string>& match_string){
	bool   return_val = false;
	container_typedef::const_iterator it;
#ifdef DEBUG
	for ( it = container.begin();it!= container.end();++it){
		std::cout<<it->first<<std::endl;
	}
#endif
       	it = container.find(key_word);
	if (it != container.end()){
		match_string = it->second->data;
		return_val = true;
	}
	return return_val;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  process_data
 *  Description:  read data from file and store into container
 * =====================================================================================
 */
bool
ClusterManager::process_data( void ){
	bool          return_val;
	std::string   line;
	std::string   word;
	std::string   double_keyword;
	size_t        word_count=0;

	File*    file_manager;
	file_manager = new File(input_file_name);

	return_val = file_manager->open_file();
	if ( return_val ){
		while ( file_manager->read_line( line )  && return_val ){
			//process each word
			std::istringstream iss(line);
			while(iss >> word && return_val) {
				if ( filter.find(word) == filter.end()){
					/* process double keyword */
					word_count++;
					if ( word_count > 1){
						double_keyword =  double_keyword + " " + word;
					return_val = container_ins->insert_data(container_ins->container, double_keyword ,line, false);
						double_keyword.erase(double_keyword.begin(),double_keyword.end());
					}
					double_keyword = word;
					return_val = container_ins->insert_data(container_ins->container, word ,line, false);
				}
			}
		}
	}
	delete file_manager;
	return return_val;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_filter
 *  Description:  ther are few filters for processing line..
 *                we should not consider these filters during
 *                clauster creation.. see filter.txt file
 * =====================================================================================
 */
bool
ClusterManager::init_filter( void){
	File*          file_manager;
	bool           return_val ;
	std::string    line;
	file_manager = new File(filter_file_name);
	return_val = file_manager->open_file();
	if ( return_val ){
		while ( file_manager->read_line( line )  && return_val ){
#ifdef DEBUG
			std::cout<<"FILTER"<<line<<std::endl;
#endif
			return_val = filter.insert(std::pair<std::string,bool>(line,true)).second;
		}
	}
	delete file_manager;
	return return_val;
}

bool
ClusterManager::search_keyword(std::string& key_word, std::vector<std::string>& match_string){
	return container_ins->search_keyword( key_word,match_string);
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_top_n_cluster
 *  Description:  print top "count" cluster with string
 * =====================================================================================
 */
size_t
ClusterManager::print_top_n_cluster( size_t count, bool print_cluster_data ){
	return container_ins->print_n_data( count, print_cluster_data);
}

bool
Config::parse_config(){
	File*          file_manager;
	bool           return_val ;
	std::string    line;
	char *s, buff[MAXLEN];
	char *temp1,*temp2;
	file_manager = new File( config_file_name);
	return_val = file_manager->open_file();
	if ( return_val ){
		while ( file_manager->read_line( line )  && return_val ){
#ifdef DEBUG
			std::cout<<"LINE:"<<line<<std::endl;
#endif
			std::copy(line.begin(), line.end(), buff);
			buff[line.size()] = '\0'; // don't forget the terminating 0

			/* Skip blank lines and comments */
			if (buff[0] == '\n' || buff[0] == '#'){
				continue;
			}
			if ( flag & 0x08){
				search_keywords.push_back(line);
				continue;
			}
			/* Parse name/value pair from line */
			char name[MAXLEN], value[MAXLEN];
			s = strtok (buff, "=");
			if (s==NULL)
				continue;
			else
				strncpy (name, s, MAXLEN);
			s = strtok (NULL, "=");
			if (s==NULL)
				continue;
			else
				strncpy (value, s, MAXLEN);
			trim (value);
#ifdef DEBUG
			std::cout<<"NAME:"<<name<<std::endl;
#endif
			/* Copy into correct entry in parameters struct */
			if (strcmp(name, "DATA_FILE")==0){
				temp1 = strtok(value,"[");
				temp2=strtok(temp1,"]");
				input_data_file_name = std::string(temp2);
				flag |= 0x01;
			}
			else if (strcmp(name, "FILTER_FILE")==0){
				temp1 = strtok(value,"[");
				temp2=strtok(temp1,"]");
				input_filter_file_name= temp2;
				flag |= 0x02;
			}
			else if (strcmp(name, "CLUSTER_COUNT")==0){
				cluster_count_to_print = size_t( atoi(value));
				flag |= 0x04;
			}
			else if (strcmp(name, "SEARCH_COUNT")==0){
				keyword_search_cout = size_t( atoi(value));
				flag |= 0x08;
			}
			
		}
	}
	delete file_manager;
	return return_val;

}



#include <unistd.h>
#include <chrono>
#include <sstream>
#include<iostream>
#include <ratio>


class my_ostream
{
public:
  my_ostream() : my_fstream("some_file.txt") {}; // check if opening file succeeded!!
  // for regular output of variables and stuff
  template<typename T> my_ostream& operator<<(const T& something)
  {
    std::cout << something;
    my_fstream << something;
    return *this;
  }
  // for manipulators like std::endl
  typedef std::ostream& (*stream_function)(std::ostream&);
  my_ostream& operator<<(stream_function func)
  {
    func(std::cout);
    func(my_fstream);
    return *this;
  }
private:
  std::ofstream my_fstream;
};
 
int
main(void){
	std::string config_file_name = "config.txt";
	Config* conf= new Config(config_file_name);
	conf->parse_config();
	conf->print_config();
	
	std::string data_file_name, filter_file_name;
	size_t count;
	ClusterManager* c_managr;

	/* create cluster */
	if ( conf->get_data_file( data_file_name) & conf->get_filter_file( filter_file_name) ){
		c_managr= new ClusterManager(data_file_name, filter_file_name);
		c_managr->process_data();
	}


//	print top n cluster */
 	
	if ( conf->get_cluster_max_count(&count)){
		std::cout<<"--------------------------\n"
			<<"PRINT TOP "<<count<<" CLUSTER"<<std::endl;
		c_managr->print_top_n_cluster( count,false );
	}
	// search keyword 
	std::vector<std::string> key_words;
	std::vector<std::string> searched_data;
	if ( conf->get_keywords(key_words) ){
		for ( size_t loop = 0; loop< key_words.size(); ++ loop){
			std::cout<<"------------\n";
			std::cout<<"MATCHED DATA FOR WORD : "<<key_words[loop]<<std::endl;
			if ( c_managr->search_keyword(key_words[loop],searched_data) ){
				for( std::vector<std::string>::iterator it= searched_data.begin(); it!= searched_data.end();++it)
					std::cout<<*it<<std::endl;
			}
		}
	}
	delete c_managr;
	delete conf;
	return 0;
}
