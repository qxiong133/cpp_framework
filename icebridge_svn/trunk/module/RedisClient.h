/* 
 * File:   RedisClient.h
 * Author: Edwin Xie
 *
 * Created on 2013年10月9日, 下午5:31
 */

#ifndef REDISCLIENT_H
#define	REDISCLIENT_H

#include "hiredis/hiredis.h"
#include <string>
#include <vector>
#include "msgpack_struct.h"

class RedisClient {
	private:
		redisContext *redisCtx;
		void init(std::string host = "127.0.0.1", int port = 6379);
	public:
		bool strWriteExec(std::string cmd);
		bool strReadExec(std::string cmd, std::string &output);
		//bool multiWriteExec(std::string cmd);
		bool multiReadExec(std::string cmd, std::vector<std::string> &output);
		bool luaExec();
		RedisClient(std::string host, int port);
		RedisClient(std::string host);
		RedisClient();
		~RedisClient();
		//logic operation
		//玩家地图信息
		bool setPlayerLocation(int x, int y, int uid);
		bool getPlayerIDByLocation(int x, int y, int &output);
		
		//查询用户资源 vector的资源顺序,wood,stone,iron,concrete
		bool getPlayerResources(int uid, std::vector<std::string> &output);
		//管理用户资源, type =1表示加资源; type=2 表示减资源
		bool manPlayerResources(int uid, int type, int wood, int stone, int iron, int concrete);
		//插入士兵
		bool insertToBuildedArmyQueue(int uid, short soldier_type, short soldier_level, short population, int &soldier_id);
		
		//更新建筑信息, x,y为建筑坐标, 
		bool updateBuildingInfo(int uid, std::string uuid, float x, float y, short category, 
		short buildingClass, int hp);
		
		//pvp server 获取士兵信息
		
		bool getSoldiersByPlayerID(int uid, std::vector<soldier>& soldiers);
		//根据士兵ID获取攻击方士兵信息
		bool getSoldierInfoByIDs(int uid, std::vector<std::string> &input, std::vector<soldier>& soldiers);
		//void getUserBasicInfo(unsigned int uid);
		//获取守方士兵信息
		bool getDefenderSoldiers(int uid, std::vector<soldier>& soldiers);
		bool getUsernameByID(int uid, std::string& output);
		bool getBuildingNumByID(int uid, std::string& output);
		bool getPlayerPointsByID(int uid, std::string& output);
		
		//写用户积分
		bool plusPlayerPoint(int uid, int points);
		//根据用户名反查UID
		bool getPlayerIDByUname(std::string uname, unsigned int& uid);
		//查询建筑当前资源
		bool getBuildingCurRes(int uid, std::string buildingID, int& resNum);
		//查询建筑类型
		bool getBuildingType(int uid, std::string buildingID, int& type);
		
		//士兵死亡
		bool setSoldierDie(int uid, std::string soldierID);
		
		bool setDfdSoldier(int uid, std::string soldierID, std::string build_sn);
		bool cancelDfdSoldier(int uid, std::string soldierID);
};


#endif	/* REDISCLIENT_H */

