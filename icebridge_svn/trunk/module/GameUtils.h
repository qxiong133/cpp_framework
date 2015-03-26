/* 
 * File:   GameUtils.h
 * Author: Edwin Xie
 *
 * Created on 2013年10月17日, 下午2:43
 */

#ifndef GAMEUTILS_H
#define	GAMEUTILS_H

#include <string>

extern unsigned long long gameGetSystemTime();
extern unsigned long long gameGetElapsedTimeMillis();
extern const std::string gameToString(int value);
extern const std::string gameFloatToString(float value);
extern const int gameToInt(const std::string& intString);
extern const unsigned int gameToUnsignedInt(const std::string& intString);

#endif	/* GAMEUTILS_H */