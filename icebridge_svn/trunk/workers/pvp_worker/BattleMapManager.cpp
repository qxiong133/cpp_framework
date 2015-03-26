#include "BattleMapManager.h"
#include "BattleMap.h"
#include "Defines.h"
#include <msgpack.hpp>
#include <msgpack_struct.h>
#include "msgpack_struct.h"
#include "msgpack_id.h"
#include "base_function.hpp"
#include "RedisClient.h"
#include "GameLog.h"
#include <iostream>

extern WorkerSender* msgSender;
extern RedisClient* mRedis;
static BattleMaps battleMaps;
static PlayerPairs playerPairs;

BattleMaps const &getBattleMaps() {
    return battleMaps;
}

BattleMap* getBattleMap(unsigned int uid) {
    BattleMaps::iterator iter = battleMaps.find(uid);
    return (iter != battleMaps.end()) ? iter->second : NULL;
}

int isAttackerOrDefender(unsigned int uid) {
    BattleMap* tmp = getBattleMap(uid);
    //uid非守方
    if (!tmp) {
        PlayerPairs::iterator pairIter = playerPairs.find(uid);
        if (pairIter != playerPairs.end()) {
            return 2;
        }
    } else {
        return 1;
    }
    return 0;
}

unsigned int getPeerUid(unsigned int uid) {
    BattleMap* tmp = getBattleMap(uid);

    if (!tmp) {
        PlayerPairs::iterator pairIter = playerPairs.find(uid);

        if (pairIter != playerPairs.end()) {
            return pairIter->second;
        } else {
            //TODO
            return 0;
        }
    } else {
        return tmp->getAtkerID();
    }
}

void deinitializeBattleMap(unsigned int uid) {
    BattleMaps::iterator iter = battleMaps.find(uid);

    if (iter != battleMaps.end()) {
        delete iter->second;
        battleMaps.erase(iter);
    }
}

void deinitialize() {
    for (BattleMaps::iterator i = battleMaps.begin(), i_end = battleMaps.end(); i != i_end; ++i) {
        delete i->second;
    }

    battleMaps.clear();
}

bool insertBattleMap(unsigned int uid, BattleMap* item) {
    BattleMaps::iterator iter = battleMaps.find(uid);

    //防守方在被攻打中
    if (iter != battleMaps.end()) {
        GameLog() << "dfder is being attacked. dfdUid = " << uid << ". atkerUid = " << iter->second->getAtkerID();
        return false;
    }

    //可以攻打
    battleMaps[uid] = item;
    //攻防uid pair
    unsigned int atkerID = item->getAtkerID();
    playerPairs[atkerID] = uid;
    return true;
}

void updateBattleMaps() {
    // GameLog() << "battlemap num = " << battleMaps.size();
    BattleMaps::iterator iter = battleMaps.begin();

    while (iter != battleMaps.end()) {
        if (iter->second->getIsStart() == false) {
            iter++;
            continue;
        }
        iter->second->update();

        unsigned int atkerUid = iter->second->getAtkerID();
        unsigned int dfderUid = iter->second->getDfderID();

        //每隔2s要求双方报告状态
        if ((iter->second->getCurTick() % (2000 / SERVER_TICK_TIME)) == 0) {
            msgAskCurStateResponse msgACSRes;
            msgACSRes.isReport = true;
            msgSender->SendToPlayer(msgACSRes, msgAskCurStateResponse_ID, atkerUid);
            msgSender->SendToPlayer(msgACSRes, msgAskCurStateResponse_ID, dfderUid);
            GameLog() << "ask atker for state response. atkerUid = " << atkerUid << ". dfderUid = " << dfderUid;
        }
        //三分钟时间到, 结算
        if (iter->second->getCurTick() >= (1000 / SERVER_TICK_TIME) * SECONDS_PER_BATTLE) {
            unsigned short starNum = iter->second->getStarNum();
            /*计算双方积分*/

            int curAtkerPoint = iter->second->getAtkerPoint();
            int curDfderPoint = iter->second->getDfderPoint();
            int atkerGettedPoint;
            int dfderGettedPoint;
            //进攻方积分
            if (starNum > 0) {
                atkerGettedPoint = (30 - (curAtkerPoint - curDfderPoint) / 10) *(starNum / 3);
                if (starNum == 3) {
                    atkerGettedPoint += THREE_STAR_REWARD;
                }
                dfderGettedPoint = 0 - atkerGettedPoint;
            }//防守方积分
            else {
                //（20+(进攻方积分-防守方积分)/20）
                dfderGettedPoint = (20 + (curAtkerPoint - curDfderPoint) / 20);
                atkerGettedPoint = 0 - dfderGettedPoint;
            }

            msgReportBattleResultToAtkerResponse msgRBRTARes;
            msgRBRTARes.starNum = starNum;
            msgRBRTARes.gettedPoint = atkerGettedPoint;
            msgSender->SendToPlayer(msgRBRTARes, msgReportBattleResultToAtkerResponse_ID, atkerUid);

            msgReportBattleResultToDfderResponse msgRBRTDRes;
            msgRBRTDRes.starNum = starNum;
            msgRBRTDRes.gettedPoint = dfderGettedPoint;
            msgSender->SendToPlayer(msgRBRTDRes, msgReportBattleResultToDfderResponse_ID, dfderUid);
            GameLog() << "battler game over.  atkerUid = " << atkerUid << ". dfderUid = " << dfderUid <<
                    ". atker point = " << atkerGettedPoint << ". dfder point = " << dfderGettedPoint <<
                    "star level = " << starNum;
            //积分信息写Redis
            mRedis->plusPlayerPoint(atkerUid, atkerGettedPoint);
            mRedis->plusPlayerPoint(dfderUid, dfderGettedPoint);

            //销毁该battlemap
            delete iter->second;
            battleMaps.erase(iter++);
            GameLog() << "battle game over. battlemap num = " << battleMaps.size();
        } else {
            iter++;
        }
    }
}
