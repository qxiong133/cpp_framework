/*
 * RandomUtil.cpp
 *
 *  Created on: Sep 11, 2013
 *      Author: root
 */

#include "RandomUtil.h"
#include <boost/random.hpp>
#include    <boost/static_assert.hpp>
#include <ctime>

#define RandomNum  1000000

unsigned int global_randomarray[RandomNum];

RandomUtil::RandomUtil() {
	// TODO Auto-generated constructor stub

}

RandomUtil::~RandomUtil() {
	// TODO Auto-generated destructor stub

}

void RandomUtil::GenerateLotsOfRandom(){
	  static boost::mt19937 rng(time(0));
	  unsigned int max_int = 1<< 30;
	  boost::uniform_int<> randomSeed(0, max_int);
	  unsigned int iquestion = 0;

      for(int i = 0; i < RandomNum; i++){

    	  	 iquestion = randomSeed(rng);
    	  	 global_randomarray[i] = iquestion;

      }
}

unsigned int RandomUtil::GetARandom(){
	static int i = 0;

	if( i >= RandomNum ){
		i = 0;
	}

	return global_randomarray[i++];
}

void RandomUtil::getBoxofRandoms(unsigned char * &box){
	static int bindex[25]={1,1562,61,49180,395,504,200580,247,9,7553,65465,24,36595,21554,225,23,54,256,5,61,265145,6,215,156112,265419};
    static unsigned int i = 0,j = 0;
    i += 256/4;

    if( i >= RandomNum - 256 ){
         i = bindex[j];
         j++;
         if(j >= 25){
        	 j = 0;
         }
    }
    box = (unsigned char *)(&global_randomarray[i]);
}

unsigned char RandomUtil::getRandomInBox(unsigned char *box){
	static int i = 0;
	if(i >= 256){
		i = 0;
	}

	return box[i++];
}

bool RandomUtil::IsThisPercentHappen(int percent){

	unsigned int random = RandomUtil::GetARandom();
    int num = random % 1000+1;
    if(num <= percent){
    	return true;
    }

	return false;
}


