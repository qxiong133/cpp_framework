#ifndef BATTLEMAPMANAGER
#define BATTLEMAPMANAGER

#include <map>
#include <string>
#include "BattleMap.h"
//class BattleMap;

//key为防守者uid
typedef std::map< unsigned int, BattleMap* > BattleMaps;

//<atker uid, deder uid>
typedef std::map<unsigned int, unsigned int> PlayerPairs;

//获取一个battle map
BattleMap* getBattleMap(unsigned int uid);

//根据uid, 获取对方(攻或防)uid
unsigned int getPeerUid(unsigned int uid);

//根据UID判断是攻方还是防守方, 1表示是守方, 2表示攻方, 0表示非攻非守
int isAttackerOrDefender(unsigned int uid);
//释放一个battle map
void deinitializeBattleMap(unsigned int uid);
//获取所有BattleMap
BattleMaps const &getBattleMaps();

//uid为守方uid, 如果玩家已参加战斗, 返回flase
bool insertBattleMap(unsigned int uid, BattleMap* item);

void updateBattleMaps();

void deinitialize();

#endif
