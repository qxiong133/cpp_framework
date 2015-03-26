/*
 * SessionFilter.h
 *
 *  Created on: Aug 3, 2013
 *      Author: root
 */

#ifndef SESSIONFILTER_H_
#define SESSIONFILTER_H_
#include <netinet/in.h>
#include <vector>
#include <string>
typedef in_addr IN_ADDR;

typedef struct IPRange
{
    IN_ADDR startIP;
    IN_ADDR endIP;
} IPRange;

class SessionFilter {
public:
	typedef std::vector<IPRange> VectIPRangeT;
	SessionFilter();
	virtual ~SessionFilter();

	static SessionFilter *sharedInstance(){
		static SessionFilter *pSharedInstance ;
		if(pSharedInstance ==  NULL){
			pSharedInstance = new SessionFilter();
		}

		return pSharedInstance;
	}

	void addIPRangeAccess(const char* ipStart, const char* ipStop, bool bPermit = false);
    bool isAddressAllowed(std::string	ipStr);



private:

    void addBadIP();
    VectIPRangeT vectIPBlocked;
    static const unsigned int IPSize = 1;


};

#endif /* SESSIONFILTER_H_ */
