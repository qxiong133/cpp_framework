#include <msgpack.hpp>
#include <msgpack_struct.h>
#include "msgpack_struct.h"
#include "msgpack_id.h"
#include "msgpack_convert.hpp"
#include <vector>
#include <string>
#include "PlayerManager.h"
#include "BattleMapManager.h"
#include "base_function.hpp"
#include "GameHandler.h"
#include "Defines.h"
#include <iostream>
#include "GameLog.h"

extern WorkerSender* msgSender;

void GameHandler::handleMsg() {
    int size = mMsgQueue->size();
    for (int i = 0; i < size; i++) {
        msgData msg = mMsgQueue->pop();
        unsigned int uid = *((unsigned int*) (msg.data.c_str() + 2));
        unsigned int peerUid;
        short msgID = *((short*) (msg.data.c_str() + 6));
        //GameLog() << "msg received. msgID = " << msgID << ". uid = " << uid;

        if (msgID == LoginRequest_ID) {
            GameLog() << "player login. Uid = " << uid;
            userLogin(uid);
            //unsigned int atkerUid = getPeerUid(uid);
            //如果处于被攻打中, 告诉攻方, 攻方发送当前战斗信息给守方
            BattleMap* curBattle = getBattleMap(uid);
            if (curBattle != NULL) {
                GameLog() << "player is being atked. uid = " << uid;
                curBattle->setIsStart(false);
                //step 1: 告诉守方, 正在被攻击
                msgBeingAtkResponse beingAtkRes;
                beingAtkRes.atkerUid = peerUid;
                mRedis->getUsernameByID(peerUid, beingAtkRes.atkerUname);
                msgSender->SendToPlayer(beingAtkRes, msgBeingAtkResponse_ID, uid);
                //step 2: 告诉攻方, 守方上线
                msgDfderLoginResponse loginRes;
                loginRes.dfderUid = uid;
                mRedis->getUsernameByID(uid, loginRes.dfderUname);
                msgSender->SendToPlayer(loginRes, msgDfderLoginResponse_ID, curBattle->getAtkerID());
            }
        } else if (msgID == LogoutRequest_ID) {
            GameLog() << "player logout. Uid = " << uid;
            userLogout(uid);
            //玩家下线处理相关逻辑
            int ret = isAttackerOrDefender(uid);
            if (ret == 1 || ret == 2) {
                peerUid = getPeerUid(uid);
                //守方下线
                if (ret == 1) {
                    msgDfderLogoutResponse dfderLogoutRes;
                    dfderLogoutRes.logout = true;
                    msgSender->SendToPlayer(dfderLogoutRes, msgDfderLogoutResponse_ID, peerUid);
                }
                //攻方下线
                if (ret == 2) {
                    msgAtkerLogoutResponse atkerLogoutRes;
                    atkerLogoutRes.logout = true;
                    msgSender->SendToPlayer(atkerLogoutRes, msgAtkerLogoutResponse_ID, peerUid);
                    //TODO 出core
                    //视为撤退, battle结束, 结算资源评分等等
                    BattleMap* curBattle = getBattleMap(peerUid);
                    if (curBattle == NULL) {
                        GameLog() << "LogoutRequest, curBattle is NULL. uid = " << uid;
                        break;
                    }
                    //==========================================================
                    int curAtkerPoint = curBattle->getAtkerPoint();
                    int curDfderPoint = curBattle->getDfderPoint();
                    int atkerGettedPoint;
                    int dfderGettedPoint;
                    unsigned short starNum = curBattle->getStarNum();
                    //进攻方积分
                    if (starNum > 0) {
                        atkerGettedPoint = (30 - (curAtkerPoint - curDfderPoint) / 10) *(starNum / 3);
                        if (starNum == 3) {
                            atkerGettedPoint += THREE_STAR_REWARD;
                        }
                        dfderGettedPoint = 0 - atkerGettedPoint;
                    } else {
                        //（20+(进攻方积分-防守方积分)/20）
                        dfderGettedPoint = (20 + (curAtkerPoint - curDfderPoint) / 20);
                        atkerGettedPoint = 0 - dfderGettedPoint;
                    }

                    msgReportBattleResultToAtkerResponse msgRBRTARes;
                    msgRBRTARes.starNum = starNum;
                    msgRBRTARes.gettedPoint = atkerGettedPoint;
                    msgSender->SendToPlayer(msgRBRTARes, msgReportBattleResultToAtkerResponse_ID, uid);

                    msgReportBattleResultToDfderResponse msgRBRTDRes;
                    msgRBRTDRes.starNum = starNum;
                    msgRBRTDRes.gettedPoint = dfderGettedPoint;
                    msgSender->SendToPlayer(msgRBRTDRes, msgReportBattleResultToDfderResponse_ID, peerUid);
                    //==========================================================

                    //销毁battleMap
                    deinitializeBattleMap(peerUid);
                }
            }
        } else {
            msgpack::zone z;
            msgpack::object request;
            msgpack::unpack_return ret = msgpack::unpack((const char*) (msg.data.c_str() + 8), msg.len - 8, NULL, &z, &request);
            //GameLog() << "login msgid = " << LoginRequest_ID;
            bool msgUnpackRet;
            if (ret == msgpack::UNPACK_SUCCESS) {
                switch (msgID) {
                        //发起攻击
                    case msgStartAttack_ID:
                    {
                        GameLog() << "start atk";
                        msgStartAttack msgSABody;
                        msgUnpackRet = MsgpackConvert(&msgSABody, request);
                        if (!msgUnpackRet) {
                            GameLog() << "start atk unpack failed.";
                            break;
                        }
                        //request.convert(&msgSABody);
                        std::string defenderUname = msgSABody.defenderUname;
                        mRedis->getPlayerIDByUname(defenderUname, peerUid);
                        GameLog() << "===============dfder uid = " << peerUid;
                        //==================for test=====================
                        //peerUid = 100; //msgSABody.defenderUid;
                        //msgSABody.sidList.push_back("0");
                        //msgSABody.sidList.push_back("1");
                        //msgSABody.sidList.push_back("2");
                        //======================================================
                        if (getBattleMap(peerUid) != NULL) {
                            GameLog() << "\n dfder is being attacked";
                            break;
                        }
                        BattleMap *item = new BattleMap(uid, peerUid);
                        GameLog() << "atkerUID = " << uid << " dfderUid = " << peerUid;
                        bool initBattleMapRet = insertBattleMap(peerUid, item);

                        GameLog() << "init battle success. initBattleMapRet = " << initBattleMapRet;

                        msgStartAtkToDfderResponse msgSADRes;
                        //获取攻击方士兵信息, 发送至守方 出core过
                        mRedis->getSoldierInfoByIDs(uid, msgSABody.sidList, msgSADRes.atkSoldiers);
                        msgSADRes.heroID = msgSABody.heroID;
                        msgSADRes.x = msgSABody.x;
                        msgSADRes.y = msgSABody.y;
                        msgSender->SendToPlayer(msgSADRes, msgStartAtkToDfderResponse_ID, peerUid);
                        //获取防守方士兵信息, 发送至攻方 TODO 出core过
                        msgStartAtkToAtkerResponse msgSAARes;
                        mRedis->getDefenderSoldiers(peerUid, msgSAARes.dfderSoldiers);
                        //TODO  防守方英雄?
                        msgSAARes.heroID = 2;
                        msgSAARes.dfderIsOnline = isOnline(peerUid);
                        msgSender->SendToPlayer(msgSAARes, msgStartAtkToAtkerResponse_ID, uid);

                    }
                        break;
                    case msgHeroCastSkillRequest_ID:
                    {
                        peerUid = getPeerUid(uid);

                        msgHeroCastSkillRequest msgHCSBody;
                        msgUnpackRet = MsgpackConvert(&msgHCSBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }

                        //发送确认
                        msgRequestAckResponse msgRARes;
                        msgRARes.msgRequestID = msgHeroCastSkillRequest_ID;
                        msgSender->SendToPlayer(msgRARes, msgRequestAckResponse_ID, uid);

                        msgHeroCastSkillResponse msgHCSRes;
                        msgHCSRes.heroID = msgHCSBody.heroID;
                        msgHCSRes.isAtk = msgHCSBody.isAtk;
                        msgHCSRes.targets = msgHCSBody.targets;
                        msgHCSRes.skillID = msgHCSBody.skillID;
                        msgSender->SendToPlayer(msgHCSRes, msgHeroCastSkillResponse_ID, peerUid);
                    }
                        break;
                    case msgObjectDieRequest_ID:
                    {
                        peerUid = getPeerUid(uid);
                        if (peerUid == 0) {
                            GameLog() << "there is no battle about uid = " << uid;
                            break;
                        }
                        msgObjectDieRequest msgODBody;
                        msgUnpackRet = MsgpackConvert(&msgODBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }
                        //request.convert(&msgODBody);

                        msgObjectDieResponse msgODRes;
                        msgODRes.objectID = msgODBody.objectID;
                        msgODRes.isBuilding = msgODBody.isBuilding;
                        msgSender->SendToPlayer(msgODRes, msgObjectDieResponse_ID, peerUid);

                        //发送确认
                        msgRequestAckResponse msgRARes;
                        msgRARes.msgRequestID = msgObjectDieRequest_ID;
                        msgSender->SendToPlayer(msgRARes, msgRequestAckResponse_ID, uid);
                        //士兵死亡写入redis
                        mRedis->setSoldierDie(peerUid, msgODBody.objectID);

                        //计算积分, 以及掠夺资源
                        int isAtker = isAttackerOrDefender(uid);
                        //攻方, 计算资源掠夺情况
                        if (isAtker == 2) {
                            if (msgODBody.isBuilding) {
                                BattleMap *battle = getBattleMap(peerUid);
                                if (msgODBody.isTownhall) {
                                    battle->setTownhallDied();
                                }
                                battle->plusDiedBuildingNum();
                                //获取建筑类型
                                int buildingType;
                                bool typeRet = mRedis->getBuildingType(peerUid, msgODBody.objectID, buildingType);
                                if (typeRet) {
                                    //如果是四类资源
                                    if (buildingType == 1 || buildingType == 2 || buildingType == 3 || buildingType == 4) {
                                        //资源数量
                                        int resLoot;
                                        bool resRet = mRedis->getBuildingCurRes(peerUid, msgODBody.objectID, resLoot);
                                        if (resRet) {
                                            if (buildingType == 1) {
                                                battle->setWoodLoot(resLoot);
                                            }
                                            if (buildingType == 2) {
                                                battle->setStoneLoot(resLoot);
                                            }
                                            if (buildingType == 3) {
                                                battle->setOreLoot(resLoot);
                                            }
                                            if (buildingType == 4) {
                                                battle->setConcreteLoot(resLoot);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                        break;
                    case msgHeroMoveRequest_ID:
                    {
                        peerUid = getPeerUid(uid);
                        if (peerUid == 0) {
                            GameLog() << "there is no battle about uid = " << uid;
                            break;
                        }
                        msgHeroMoveRequest msgHMBody;
                        msgUnpackRet = MsgpackConvert(&msgHMBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }
                        //request.convert(&msgHMBody);
                        //发送确认
                        msgRequestAckResponse msgRARes;
                        msgRARes.msgRequestID = msgHeroMoveRequest_ID;
                        msgSender->SendToPlayer(msgRARes, msgRequestAckResponse_ID, uid);
                        //////
                        msgHeroMoveResponse msgHMRes;
                        msgHMRes.heroID = msgHMBody.heroID;
                        msgHMRes.dstX = msgHMBody.dstX;
                        msgHMRes.dstY = msgHMBody.dstY;
                        msgSender->SendToPlayer(msgHMRes, msgHeroMoveResponse_ID, peerUid);

                    }
                        break;
                    case msgCurBattleInfoRequest_ID:
                    {
                        peerUid = getPeerUid(uid);
                        GameLog() << "msgCurBattleInfoRequest reveived. peerUid = " << peerUid;
                        if (peerUid == 0) {
                            GameLog() << "there is no battle about uid = " << uid;
                            break;
                        }
                        msgCurBattleInfoRequest msgCBIBody;
                        msgUnpackRet = MsgpackConvert(&msgCBIBody, request);

                        if (!msgUnpackRet) {
                            GameLog() << "msgCurBattleInfoRequest unpack failed.";
                            break;
                        }
                        //request.convert(&msgCBIBody);

                        msgCurBattleInfoResponse msgCBIRes;
                        msgCBIRes.attackers = msgCBIBody.attackers;
                        msgCBIRes.defenders = msgCBIBody.defenders;
                        msgCBIRes.buildings = msgCBIBody.buildings;
                        msgSender->SendToPlayer(msgCBIRes, msgCurBattleInfoResponse_ID, peerUid);
                    }
                        break;
                    case msgDfderIsreadyRequest_ID:
                    {
                        peerUid = getPeerUid(uid);
                        if (peerUid == 0) {
                            GameLog() << "there is no battle about uid = " << uid;
                            break;
                        }
                        msgDfderIsreadyRequest msgDIBody;
                        msgUnpackRet = MsgpackConvert(&msgDIBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }

                        BattleMap* curBattle = getBattleMap(uid);
                        if (curBattle == NULL) {
                            GameLog() << "get battlemap failed. there is no battle about uid = " << uid;
                            break;
                        }
                        curBattle->setIsStart(true);

                        msgDfderIsreadyResponse msgDIRes;
                        msgDIRes.isReady = msgDIBody.isReady;
                        msgSender->SendToPlayer(msgDIRes, msgDfderIsreadyResponse_ID, peerUid);
                    }
                        break;
                    case msgReportCurStateRequest_ID:
                    {

                        peerUid = getPeerUid(uid);
                        if (peerUid == 0) {
                            GameLog() << "there is no battle about uid = " << uid;
                            break;
                        }
                        GameLog() << "msgReportCurStateRequest received. sender uid = " << uid << ". peerUid = " << peerUid;
                        msgReportCurStateRequest msgRCSBody;
                        msgUnpackRet = MsgpackConvert(&msgRCSBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }
                        //request.convert(&msgRCSBody);

                        msgReportCurStateResponse msgRCSRes;
                        msgRCSRes.peerSoldiers = msgRCSBody.selfSoldiers;
                        msgSender->SendToPlayer(msgRCSRes, msgReportCurStateResponse_ID, peerUid);
                    }
                        break;
                    case msgSetRallypointRequest_ID:
                    {
                        peerUid = getPeerUid(uid);
                        msgSetRallypointRequest msgSRPBody;
                        msgUnpackRet = MsgpackConvert(&msgSRPBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }
                        //request.convert(&msgSRPBody);

                        //发送确认
                        msgRequestAckResponse msgRARes;
                        msgRARes.msgRequestID = msgSetRallypointRequest_ID;
                        msgSender->SendToPlayer(msgRARes, msgRequestAckResponse_ID, uid);

                        msgSetRallypointResponse msgSRPRes;
                        msgSRPRes.x = msgSRPBody.x;
                        msgSRPRes.y = msgSRPBody.y;
                        msgSRPRes.targetID = msgSRPBody.targetID;
                        msgSRPRes.tgtIsBuilding = msgSRPBody.tgtIsBuilding;
                        msgSender->SendToPlayer(msgSRPRes, msgSetRallypointResponse_ID, peerUid);

                    }
                        break;
                    case msgAtkerStopBattleRequest_ID:
                    {
                        GameLog() << "atker stop battle. uid = " << uid;
                        peerUid = getPeerUid(uid);
                        msgAtkerStopBattleRequest msgASBBody;
                        msgUnpackRet = MsgpackConvert(&msgASBBody, request);
                        if (!msgUnpackRet) {
                            break;
                        }
                        //告诉防守方攻方主动停止攻打
                        msgAtkerStopBattleToDfderResponse msgASBTDRes;
                        msgASBTDRes.isStop = msgASBBody.isStop;
                        msgSender->SendToPlayer(msgASBTDRes, msgAtkerStopBattleToDfderResponse_ID, peerUid);

                        //ack发送给攻方
                        msgRequestAckResponse msgRARes;
                        msgRARes.msgRequestID = msgAtkerStopBattleRequest_ID;
                        msgSender->SendToPlayer(msgRARes, msgRequestAckResponse_ID, uid);

                        //发送战果给双方

                        //TODO 出core
                        //视为撤退, battle结束, 结算资源评分等等
                        BattleMap* curBattle = getBattleMap(peerUid);
                        if (curBattle == NULL) {
                            GameLog() << "stop battle, curBattle is NULL. uid = " << uid;
                            break;
                        }
                        //==========================================================
                        int curAtkerPoint = curBattle->getAtkerPoint();
                        int curDfderPoint = curBattle->getDfderPoint();
                        int atkerGettedPoint;
                        int dfderGettedPoint;
                        unsigned short starNum = curBattle->getStarNum();
                        //进攻方积分
                        if (starNum > 0) {
                            atkerGettedPoint = (30 - (curAtkerPoint - curDfderPoint) / 10) *(starNum / 3);
                            if (starNum == 3) {
                                atkerGettedPoint += THREE_STAR_REWARD;
                            }
                            dfderGettedPoint = 0 - atkerGettedPoint;
                        } else {
                            //（20+(进攻方积分-防守方积分)/20）
                            dfderGettedPoint = (20 + (curAtkerPoint - curDfderPoint) / 20);
                            atkerGettedPoint = 0 - dfderGettedPoint;
                        }

                        msgReportBattleResultToAtkerResponse msgRBRTARes;
                        msgRBRTARes.starNum = starNum;
                        msgRBRTARes.gettedPoint = atkerGettedPoint;
                        msgRBRTARes.woodLoot = curBattle->getWoodLoot();
                        msgRBRTARes.stoneLoot = curBattle->getStoneLoot();
                        msgRBRTARes.oreLoot = curBattle->getOreLoot();
                        msgRBRTARes.concreteLoot = curBattle->getConcreteLoot();
                        msgSender->SendToPlayer(msgRBRTARes, msgReportBattleResultToAtkerResponse_ID, uid);

                        msgReportBattleResultToDfderResponse msgRBRTDRes;
                        msgRBRTDRes.starNum = starNum;
                        msgRBRTDRes.gettedPoint = dfderGettedPoint;
                        msgRBRTDRes.woodLoot = curBattle->getWoodLoot();
                        msgRBRTDRes.stoneLoot = curBattle->getStoneLoot();
                        msgRBRTDRes.oreLoot = curBattle->getOreLoot();
                        msgRBRTDRes.concreteLoot = curBattle->getConcreteLoot();
                        msgSender->SendToPlayer(msgRBRTDRes, msgReportBattleResultToDfderResponse_ID, peerUid);
                        //==========================================================

                        //销毁battleMap
                        deinitializeBattleMap(peerUid);

                    }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
