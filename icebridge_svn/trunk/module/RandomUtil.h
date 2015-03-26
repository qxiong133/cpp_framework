/*
 * RandomUtil.h
 *
 *  Created on: Sep 11, 2013
 *      Author: root
 */

#ifndef RANDOMUTIL_H_
#define RANDOMUTIL_H_

class RandomUtil {
public:
	static void GenerateLotsOfRandom();

	static unsigned int GetARandom();
    static bool IsThisPercentHappen(int percent);
	RandomUtil();
    static void getBoxofRandoms(unsigned char * &box);
    static unsigned char getRandomInBox(unsigned char *box);
	virtual ~RandomUtil();

private:
	//static RandomUtil pSharedInstance;
};

#endif /* RANDOMUTIL_H_ */
