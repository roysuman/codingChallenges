/*
 * =====================================================================================
 *
 *       Filename:  disaster_recovery.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  Thursday 24 December 2015 11:11:54  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SIGCONT (suman roy), email.suman.roy@gmail.com
 *   Organization:  OPEN SOURCE
 *
 * =====================================================================================
 */

#include<iostream>
#include<vector>
#include<sstream>
#include<iterator>
#include<algorithm>
#include<cstring>

//size_t **array;//[no_server][no_server];
size_t res( std::vector<int> &data_set , int start, int end , int pow,std::vector<std::vector<size_t> >&array){

	if( start > end ) return 0;
	if ( array[start][pow - 1] != 0  ){ return array[start][pow - 1];}
	size_t sum1 = (data_set[start] * pow ) + res( data_set , start + 1 , end , pow +1 ,array);
	size_t sum2 = (data_set[end] * pow ) + res( data_set , start  , end - 1 , pow +1 ,array);
	array[start][pow - 1] = std::min( sum1,sum2);
	
	return array[start][pow - 1];
}

int main(){
	std::string line;
	int no_server;
	std::vector<int>data;
	std::cin>>no_server;
	std::istream_iterator<int> eos;
	std::istream_iterator<int> iit(std::cin);
	// No loop is necessary, because you can use copy()
	std::vector<std::vector<size_t>> array2d(no_server, std::vector<size_t>(no_server,0));
//	std::vector<std::vector<std::vector<size_t> > > vec (no_server,std::vector<std::vector<size_t> >(no_server,std::vector <size_t>(no_server)));
//	array2d.clear();
	std::copy(iit, eos, std::back_inserter(data));
	res(data,0,data.size() - 1 , 1 ,array2d);
	std::cout<<array2d[0][0];
}
