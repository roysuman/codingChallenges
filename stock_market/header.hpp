//  header.hpp
//lase modified: 15/5/2013
// developed by : suman roy (email.suman.roy@gmail.com )
//
// Defination : contains all header files needed by this socket prg
#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <netdb.h>
#include <fstream>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include<time.h>
using namespace std;

#define prnt_hex(x, n) { for( int i=0; i<n; i++ ) printf("0x%x ", (int)x[i]);}
#endif//_HEADER_H_l
