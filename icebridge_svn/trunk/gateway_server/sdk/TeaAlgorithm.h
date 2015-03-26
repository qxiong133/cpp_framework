/*
 * TeaAlgorithm.h
 *
 *  Created on: Aug 17, 2013
 *      Author: root
 */

#ifndef TEAALGORITHM_H_
#define TEAALGORITHM_H_
#include <arpa/inet.h>
#define BYTES_TO_ENCRYPT 32
#define KEY_ENCRYPT "0123456789abcdef"
#include <cstring>

class TeaAlgorithm {

public:

	typedef  unsigned char byte;
    typedef unsigned long ulong;
    static TeaAlgorithm *sharedInstance(){
    	static TeaAlgorithm *pSharedInstance;

    	if(pSharedInstance == NULL){
    		pSharedInstance = new TeaAlgorithm();
    	}

    	return pSharedInstance;
    }

    TeaAlgorithm();
	//TeaAlgorithm(const byte *key = KEY_ENCRYPT, int round = 32, bool isNetByte = false);
	TeaAlgorithm(const TeaAlgorithm &rhs);
	TeaAlgorithm& operator=(const TeaAlgorithm &rhs);
	void encrypt( byte *in, unsigned int length,byte *box);
	bool decrypt( byte *in,unsigned int length, byte *box);


private:

	ulong ntoh(ulong netlong) { return _isNetByte ? ntohl(netlong) : netlong; }
	ulong hton(ulong hostlong) { return _isNetByte ? htonl(hostlong) : hostlong; }
private:
	int _round; //iteration round to encrypt or decrypt
	bool _isNetByte; //whether input bytes come from network
	byte _key[16]; //encrypt or decrypt key

};

#endif /* TEAALGORITHM_H_ */



