/*
 * =====================================================================================
 *
 *       Filename:  logical_hub_controller.cpp
 *
 *    Description:  Vmware challenge
 *
 *        Version:  1.0
 *        Created:  Thursday 27 August 2015 10:23:09  IST
 *       Revision:  none
 *       Compiler:  g++4.7
 *
 *         Author:  Suman Roy (), email.suman.roy@gmail.com
 *   Organization:  GNU
 *
 * =====================================================================================
 */

#include <iostream>
#include<map>
#include<sstream>
#include <vector>

//#define DEBUG 1
typedef std::multimap< std::string  , unsigned int> hubDestMap;
typedef std::map< std::string , hubDestMap > conMap;
typedef std::vector < std::string > retVec;
class Controller{
	public:
		void getData();
		void process( void );
		void displayData();
		Controller(){}
		~Controller(){}
	private:
		conMap netMap;
		void printToPort (std::string , int array[] , size_t n);
                void combinationUtil( std::string ,int arr[], size_t n, size_t r, size_t index, int data[], size_t i);
		void process2( std::string , hubDestMap);
		retVec isPresent ( std::string );
};
void
Controller::printToPort( std::string host ,int arr[], size_t n){
    int data[2];
 
    combinationUtil ( host , arr, n, 2, 0, data, 0);
}
void 
Controller::combinationUtil( std::string host ,int arr[], size_t	n, size_t r, size_t index, int data[], size_t i){
    if (index == r)
    {
	    std::cout<<"PORT_TO_PORT "<<host<<" "<<data[0]<<" " <<data[1]<<std::endl;
	    std::cout<<"PORT_TO_PORT "<<host<<" "<<data[1]<<" " <<data[0]<<std::endl;
	    return;
    }
 
    if (i >= n)
        return;
 
    data[index] = arr[i];
    combinationUtil(host , arr, n, r, index+1, data, i+1);
 
    combinationUtil( host ,arr, n, r, index, data, i+1);
}
retVec
Controller::isPresent( std::string key){
	retVec  rVec;
	conMap::iterator it;
	for ( it = netMap.begin(); it!= netMap.end(); ++ it ){
#ifdef DEBUG
		std::cout<<"HostName [ "<<it->first<<" ] ";
#endif
		hubDestMap hDM = it->second;
		hubDestMap::const_iterator found = hDM.find(key);
		if ( found == hDM.end() );
		else rVec.push_back( std::string(it->first ) );
	}
	return rVec;
}



void 
Controller::process2( std::string host , hubDestMap hMap){
	std::pair < hubDestMap::iterator , hubDestMap::iterator > ret;
	for ( hubDestMap::iterator it = hMap.begin() , end = hMap.end() ; it != end ; it = hMap.upper_bound(it->first)){
		size_t count;
		count = hMap.count( it->first);
		ret = hMap.equal_range(it->first);
		unsigned int d[ count ];
		size_t loop = 0;
		for ( hubDestMap::iterator it2 = ret.first ; it2 != ret.second ; ++it2 ){
			d [loop ] = it2->second;
			retVec rVec = isPresent( std::string(it->first) );
			retVec::iterator it3;
			bool flag = false;
		       for ( it3 = rVec.begin() ; it3!= rVec.end() ; ++ it3 ){
			       if ( (*it3) != host ){
				       std::cout<<"PORT_TO_TUNNEL "<<host<<" "<<d[loop]<<" "<<(*it3) <<" "<< it->first<<std::endl;
				       flag = true;
			       }
		       }
		       if (flag)
		       std::cout<<"TUNNEL_TO_PORT "<<host<<" "<<it->first<<" "<<d[loop]<<std::endl;
		       flag = false;
		       loop++;
		}
		printToPort( host , (int*)d , loop );
	}
}

void
Controller::process( void ){
	conMap::iterator conIt;
	for ( conIt = netMap.begin() ; conIt != netMap.end() ; ++conIt){
#ifdef DEBUG
		std::cout<<"PROCESS2 [ "<<conIt->first;//<<" ]  [ "<<conIt->second<<" ]\n";
#endif
		process2 ( conIt->first , conIt->second );

	}
}
void
Controller::displayData( void ){
	conMap::iterator it ;
	for ( it = netMap.begin(); it!= netMap.end(); ++ it ){
#ifdef DEBUG
		std::cout<<"HostName [ "<<it->first<<" ] ";
#endif

	}
}
void
Controller::getData(void ){
	std::string line;
	std::string host;
	std::string token;
	bool fst = true;
	while ( std::getline ( std::cin , line ) ){
		std::istringstream iss(line );
		fst = true;
		unsigned  int index =0;
		hubDestMap tempMap;
		while ( iss >> token ){
			if ( fst ){ fst = false ; host = token;}
			else{
				tempMap.insert ( std::pair < std::string , unsigned int >( token , index++) );
			}
		}
		netMap.insert(std::pair< std::string , hubDestMap>( host , tempMap ));
	}
	return;
}


int main(){
	//get data
	Controller ins;
	ins.getData();
#ifdef DEBUG
	ins.displayData();
#endif
	ins.process();
	return 0;
}


