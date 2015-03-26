/*
 * SessionFilter.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: root
 */

#include "SessionFilter.h"
#include<arpa/inet.h>
#include<iostream>

SessionFilter::SessionFilter() {
	// TODO Auto-generated constructor stub
	addBadIP();
}

SessionFilter::~SessionFilter() {
	// TODO Auto-generated destructor stub
}

void SessionFilter::addIPRangeAccess(const char* ipStart, const char* ipStop,
		bool bPermit) {

	IPRange ipRange;

	ipRange.startIP.s_addr = inet_addr(ipStart);
	ipRange.endIP.s_addr = inet_addr(ipStop);

	if (!bPermit) {

		vectIPBlocked.push_back(ipRange);
	}
}


bool SessionFilter::isAddressAllowed(std::string ipStr) {
	//unsigned long inet_addr (const char *cp);
	std::cout<<ipStr<<std::endl;
	long ip = inet_addr(ipStr.c_str());
    bool bIsInList = true;

	for(VectIPRangeT::iterator it = vectIPBlocked.begin(); it != vectIPBlocked.end(); it++) {

		IPRange& ipRange = *it;

		long start = ipRange.startIP.s_addr;
		long stop = ipRange.endIP.s_addr;

		if (ip <= stop && ip >= start)
		{
			bIsInList = false;
			break;
		}
	}

	return bIsInList;
}

void SessionFilter::addBadIP(){
	 static  char*  badIP [IPSize][2]= {"192.168.1.2","192.168.1.9"};
	for(int i = 0; i < IPSize; i++){
		addIPRangeAccess(badIP[i][0],badIP[i][1]);
	}
}
