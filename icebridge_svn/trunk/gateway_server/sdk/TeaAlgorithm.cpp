/*
 * TeaAlgorithm.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: root
 */

#include "TeaAlgorithm.h"
#include <stdio.h>

//for memcpy,memset
using namespace std;
/*
TeaAlgorithm::TeaAlgorithm(const byte *key , int round ,
		bool isNetByte ) :
		_round(round), _isNetByte(isNetByte) {

	if (key != 0)
		memcpy(_key, key, 16);
	else
		memset(_key, 0, 16);
}
*/
TeaAlgorithm::TeaAlgorithm(){

}

TeaAlgorithm::TeaAlgorithm(const TeaAlgorithm &rhs) :
		_round(rhs._round), _isNetByte(rhs._isNetByte) {
	memcpy(_key, rhs._key, 16);
}

TeaAlgorithm& TeaAlgorithm::operator=(const TeaAlgorithm &rhs) {
	if (&rhs != this) {
		_round = rhs._round;
		_isNetByte = rhs._isNetByte;
		memcpy(_key, rhs._key, 16);
	}
	return *this;
}

void TeaAlgorithm::encrypt( byte *in, unsigned int length,byte *box) {

	unsigned int index = 0;
	while (index < length) {
		in[index] = box[in[index]];
		index++;
	}
}

bool TeaAlgorithm::decrypt( byte *in, unsigned int length,byte *box) {

	unsigned int index = 0;
	//printf("\nlength is %u\n",length);
	while (index < length) {
//		/if(in[index] < 0 || in[index] > 256)return false;
		in[index] = box[in[index]];
		index++;
	}
	return true;
}

/*
void TeaAlgorithm::encrypt( ulong *in, unsigned int length,byte *box) {

	//out[0] = ~in[0];

	ulong *k = (ulong*) _key;
	register ulong y = ntoh(in[0]);
	register ulong z = ntoh(in[1]);
	register ulong a = ntoh(k[0]);
	register ulong b = ntoh(k[1]);
	register ulong c = ntoh(k[2]);
	register ulong d = ntoh(k[3]);
	register ulong delta = 0x9E3779B9;
	register int round = _round;
	register ulong sum = 0;

	while (round--) {
		sum += delta;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
	}
	out[0] = hton(out[0]);
	out[1] = hton(out[1]);

}

void TeaAlgorithm::decrypt(const ulong *in, ulong *out) {

	//out[0] = ~in[0];

	ulong *k = (ulong*) _key;
	register ulong y = ntoh(in[0]);
	register ulong z = ntoh(in[1]);
	register ulong a = ntoh(k[0]);
	register ulong b = ntoh(k[1]);
	register ulong c = ntoh(k[2]);
	register ulong d = ntoh(k[3]);
	register ulong delta = 0x9E3779B9;
	register int round = _round;
	register ulong sum = 0;

	if (round == 32)
		sum = 0xC6EF3720;
	else if (round == 16)
		sum = 0xE3779B90;
	//else
	sum = delta * round;

	while (round--) {
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		sum -= delta;
	}
	out[0] = ntoh(out[0]);
	out[1] = ntoh(out[1]);
}

void TeaAlgorithm::charToByte(const char *in, byte *out) {

	const char *temp = in;

	while (*temp != '\0') {
		*out = *temp;
		temp++;
		out++;
	}
}

void TeaAlgorithm::byteToChar(const byte *in, char *out) {
	const char *temp = in;

	while (*temp != '\0') {
		*out = *temp;
		temp++;
		out++;
	}
}
*/
