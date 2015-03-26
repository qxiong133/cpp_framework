#include "BattleMap.h"
#include "RedisClient.h"
#include "GameUtils.h"
#include <string>

extern RedisClient* mRedis;

BattleMap::BattleMap(unsigned int aID, unsigned int dID) : attackerID(aID), defenderID(dID),
mTicks(0), mDiedBuildingNum(0), mTownhallIsDied(false), mStarNum(0), mIsStart(true),
mWoodLoot(0), mStoneLoot(0), mOreLoot(0), mConcreteLoot(0) {
    std::string buildingNum;
    mRedis->getBuildingNumByID(dID, buildingNum);
    mBuildingNum = (unsigned int) gameToInt(buildingNum);
    mBuildingNum = 50;
    std::string atkerPoint;
    mRedis->getPlayerPointsByID(aID, atkerPoint);
    mAtkerPoint = gameToInt(atkerPoint);

    std::string dfderPoint;
    mRedis->getPlayerPointsByID(aID, dfderPoint);
    mDfderPoint = gameToInt(dfderPoint);
}

BattleMap::~BattleMap() {
}

void BattleMap::update() {
    //tick+1
    plusCurTick();
}