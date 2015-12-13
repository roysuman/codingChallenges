// file.hpp
//
//last modified: 30/5/2013
// developed by : suman roy (email.suman.roy@gmail.com)
#ifndef __FILE_H__
#define __FILE_H__

#include <string>
#include "header.hpp"
#include <fstream>
#include "structure.hpp"

class File_ {
  std::ifstream file;
public:
  File_(std::string const& File_name){
	  file.open(File_name.c_str());
  }
  
  struct IpInfoConfig Value() {
	  struct IpInfoConfig info;
	  std::string line;
	  int flag;
	  if(!file.is_open()){ 
		  std::cout<<"cant open file :"<<file<<std::endl;
		  memset( &info, 0, sizeof( struct IpInfoConfig ) );
		  return info;
	  }
	  
	  while( std::getline(file,line)){
		  if(!line.length()) continue;
		  char ip1[20], ip2[20], path[100], tag[20] ,server_log[30];
		  int port1, port2;
		  sscanf(line.c_str(), "%s %s %d %s %d %s %d %s", tag, info.ip1,&info.port1,info.ip2,&info.port2,info.path,&info.priority,info.server_log);
		  ipprint(info);
		  return info;
	  }
  }
  struct ClientInfo ClientCredentials(){
	  struct ClientInfo temp;
	  std::string line;
	  int flag;
	  if( !file.is_open( ) ){
		  std::cout<<"can't open clent info file"<<std::endl;
		  memset( & temp, 0 ,sizeof ( struct ClientInfo ) );
		  return temp;
	  }
	  while( std::getline(file , line ) ){
		  if(!line.length( ) ) continue;
		  sscanf(line.c_str(), "%s %d %s %d %s ",temp.ip1,&temp.port1,temp.ip2,&temp.port2,temp.client_log);
		  return temp;
	  }
  }
};

#endif
