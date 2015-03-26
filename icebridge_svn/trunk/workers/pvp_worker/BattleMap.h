/* 
 * File:   BattleMap.h
 * Author: Edwin Xie
 *
 * Created on 2013年12月3日, 下午4:17
 */

#ifndef BATTLEMAP_H
#define	BATTLEMAP_H

class BattleMap {
public:
    BattleMap(unsigned int aID, unsigned int dID);
    ~BattleMap();

    unsigned short getCurTick() const {
        return mTicks;
    }

    unsigned int getAtkerID() const {
        return attackerID;
    }

    unsigned int getDfderID() const {
        return defenderID;
    }
    //tick加一

    void plusCurTick() {
        mTicks++;
    }

    void setTownhallDied() {
        mTownhallIsDied = true;
    }

    void plusDiedBuildingNum() {
        if (mDiedBuildingNum < mBuildingNum) {
            mDiedBuildingNum++;
        }
        double ratio = mDiedBuildingNum / mBuildingNum;
        if (ratio >= 0.5 && ratio < 1.0) {
            if (mTownhallIsDied) {
                mStarNum = 2;
            } else {
                mStarNum = 1;
            }
        } else if (ratio >= 1.0) {
            mStarNum = 3;
        }
    }

    unsigned short getStarNum() const {
        return mStarNum;
    }
    void update();

    int getAtkerPoint() const {
        return mAtkerPoint;
    }

    int getDfderPoint() const {
        return mDfderPoint;
    }

    bool getIsStart() const {
        return mIsStart;
    }

    void setIsStart(bool isStart) {
        mIsStart = isStart;
    }

    int getWoodLoot() const {
        return mWoodLoot;
    }

    void setWoodLoot(int woodLoot) {
        mWoodLoot += woodLoot;
    }

    int getStoneLoot() const {
        return mStoneLoot;
    }

    void setStoneLoot(int stoneLoot) {
        mStoneLoot += stoneLoot;
    }

    int getOreLoot()const {
        return mOreLoot;
    }

    void setOreLoot(int oreLoot) {
        mOreLoot += oreLoot;
    }

    int getConcreteLoot() const {
        return mConcreteLoot;
    }

    void setConcreteLoot(int concreteLoot) {
        mConcreteLoot += concreteLoot;
    }

private:
    unsigned int mTicks; //节拍计数, 三分钟计时, 3min = 180s = 1800*100ms
    unsigned int attackerID;
    unsigned int defenderID;
    unsigned int mBuildingNum; //建筑总数
    unsigned int mDiedBuildingNum;
    bool mTownhallIsDied; //市政厅是否已摧毁
    unsigned short mStarNum; //星级
    int mAtkerPoint;
    int mDfderPoint;
    bool mIsStart;

    int mWoodLoot;
    int mStoneLoot;
    int mOreLoot;
    int mConcreteLoot;
};

#endif	/* BATTLEMAP_H */

