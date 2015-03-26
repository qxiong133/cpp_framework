#include "RedisClient.h"
#include "GameLog.h"
#include "GameUtils.h"
#include <iostream>
#include <string.h>
using namespace std;

#define REDIS_LBR                       "\r\n"
#define REDIS_STATUS_REPLY_OK           "OK"
#define REDIS_PREFIX_STATUS_REPLY_ERROR "-ERR "
#define REDIS_PREFIX_STATUS_REPLY_ERR_C '-'
#define REDIS_PREFIX_STATUS_REPLY_VALUE '+'
#define REDIS_PREFIX_SINGLE_BULK_REPLY  '$'
#define REDIS_PREFIX_MULTI_BULK_REPLY   '*'
#define REDIS_PREFIX_INT_REPLY          ':'
#define REDIS_WHITESPACE                "\f\n\r\t\v"

//lua 脚本sha
#define LUA_BATCH_READ_RESOURCE "5f3a457ed3f517bbe102259c6835bf579399e96c"
#define LUA_MAN_RESOURCE "92d0a81d4746aab22a1f090e0b07164bea81a140"
#define LUA_INSERT_SOLDIER "3793a46839a2e79784b3c3a79a85742bb8efa024"
#define LUA_MAN_BUILDING "a32d5391c0ec4fed0d05b6cbce91e3c88287959c"

void RedisClient::init(std::string host, int port) {
	redisCtx = redisConnect(host.c_str(), port);
	if (redisCtx != NULL && redisCtx->err) {
		GameLogError() << "redis conn failed. host= <" << host << ">. port = <" << port << ">";
	}
	GameLog() << "init redis connection success.";
}

RedisClient::RedisClient(std::string host, int port) {
	init(host, port);
}

RedisClient::RedisClient(std::string host) {
	init(host);
}

RedisClient::RedisClient() {
	init();
}

RedisClient::~RedisClient() {
	redisFree(redisCtx);
	redisCtx = NULL;
}

bool RedisClient::luaExec() {
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, "eval %s 1 %s", "return redis.call('set',KEYS[1],'bar')", "foo"));
	std::cout << reply->type << std::endl;
	std::cout << reply->str << std::endl;
	return false;
}

bool RedisClient::strWriteExec(std::string cmd) {
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	bool ret = false;
	if (reply->type == REDIS_REPLY_STATUS) {
		if (!strcmp(reply->str, REDIS_STATUS_REPLY_OK)) {
			ret = true;
		}
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::strReadExec(std::string cmd, std::string &output) {
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	bool ret = false;
	if (reply->type == REDIS_REPLY_STRING) {
		output.assign(reply->str);
		ret = true;
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::multiReadExec(std::string cmd, std::vector<std::string> &output) {
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	bool ret = false;
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (int i = 0; i < reply->elements; i++) {
			output.push_back(reply->element[i]->str);
		}
		ret = true;
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::getPlayerIDByLocation(int x, int y, int &output){
	std::string cmd = "GET "+gameToString(x)+"#"+gameToString(y);
	std::string tmpOutput;
	bool ret = strReadExec(cmd,tmpOutput);
	if(!ret){
		return ret;
	}
	output = gameToInt(tmpOutput);
	return ret;	
}

bool RedisClient::setPlayerLocation(int x, int y, int uid){
	std::string cmd = "SET "+gameToString(x)+"#"+gameToString(y) +" "+ gameToString(uid);
	bool ret = strWriteExec(cmd);
	return ret; 
}

bool RedisClient::getPlayerResources(int uid, std::vector<std::string> &output){
	std::string strUid = gameToString(uid);
	std::string luaScriptSha = LUA_BATCH_READ_RESOURCE;
	std::string cmd = "EVALSHA " + luaScriptSha+" 0 " + strUid;
	bool ret = multiReadExec(cmd, output);
	return ret;
}

bool RedisClient::manPlayerResources(int uid, int type, int wood, int stone, int iron, int concrete){
	std::string strUid = gameToString(uid);
	std::string luaScriptSha = LUA_MAN_RESOURCE;
	std::string cmd = "EVALSHA " + luaScriptSha + " 0 " + strUid + " " + 
	gameToString(type) + " " + gameToString(wood) + " " + gameToString(stone) + " " + 
	gameToString(iron) + " " + gameToString(concrete);
	bool ret;
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
		ret = true;
	}else{
		ret = false;
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::insertToBuildedArmyQueue(int uid, short soldier_type, short soldier_level, short population, int &soldier_id){
	std::string strUid = gameToString(uid);
	std::string luaScriptSha = LUA_INSERT_SOLDIER;
	std::string cmd = "EVALSHA " + luaScriptSha + " 0 " + strUid + " " +
						gameToString((int)soldier_type) + " " + gameToString((int)soldier_level) + " " 
						+ gameToString((int)population);
	bool ret;
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	if (reply->type == REDIS_REPLY_INTEGER && reply->integer >= 0) {
		ret = true;
		soldier_id = reply->integer;
	}else{
		ret = false;
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::updateBuildingInfo(int uid, std::string uuid, float x, float y, short category, short buildingClass, int hp){
	std::string strUid = gameToString(uid);
	std::string luaScriptSha = LUA_MAN_BUILDING;
	std::string location = gameFloatToString(x) + "#" + gameFloatToString(y);
	std::string cmd = "EVALSHA " + luaScriptSha + " 0 " + strUid + " " + uuid + " " +
						location + " " + gameToString((int)category) + " " + gameToString((int)buildingClass)
						+ " " + gameToString((int)hp);
	bool ret;
	redisReply *reply = static_cast<redisReply*> (redisCommand(redisCtx, cmd.c_str()));
	if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
		ret = true;
	}else{
		ret = false;
	}
	freeReplyObject(reply);
	return ret;
}

bool RedisClient::getSoldierInfoByIDs(int uid, std::vector<std::string> &input, std::vector<soldier>& soldiers){
	std::vector<std::string>  allSoldiers;
	std::vector<std::string>  atkSoldiers;
	std::string strUid = gameToString(uid);
	std::string listKey = "army#" + strUid;	
	std::string cmd = "LRANGE " + listKey + " 0 -1";
	
	bool ret = multiReadExec(cmd, allSoldiers);
	if(!ret || allSoldiers.empty()){
		return false;
	}
	std::vector<std::string>::iterator iter;
	
	int index ;
	for(iter = input.begin(); iter != input.end(); iter++){
		index = gameToInt(*iter);
		std::string tmpStr = allSoldiers[index]; //形如army#1
		short category = gameToInt(tmpStr.substr(5,1));
		std::string field = tmpStr + "#max";
		std::string strLevel;
		ret = strReadExec("HGET user#" + strUid + " " +field, strLevel);
		/*if(!ret){
			return false;
		}*/
		short level = 1; //(short)gameToInt(strLevel);
		
		soldier tmpSoldier;
		tmpSoldier.category = category;
		tmpSoldier.level = level;
		tmpSoldier.sid = *iter;
		soldiers.push_back(tmpSoldier);
	}
	
	return true;
}

bool RedisClient::getDefenderSoldiers(int uid, std::vector<soldier>& soldiers){
	std::string strUid = gameToString(uid);
	std::string armyListKey = "army#" + strUid;
	std::string listKey = armyListKey +"#dfd"; 
	//std::string listArmyCampKey = listKey + "#camp";
	
	std::vector<std::string> soldierIDs;
	std::vector<std::string> dfdSoldierCamps;
	std::string cmd = "LRANGE " + listKey + " 0 -1";
	bool ret = multiReadExec(cmd, soldierIDs);
	if(!ret){
		return false;
	}
	//cmd = "LRANGE " + listArmyCampKey + " 0 -1";
	/*ret = multiReadExec(cmd, dfdSoldierCamps);
	if(!ret){
		return false;
	}*/
	
	int size = soldierIDs.size();
	//std::vector<std::string>::iterator iter;
	
	int index ;
	//int incr = 0;
	for(int i = 0; i < size; i++){
	//for(iter = soldierIDs.begin(); iter != soldierIDs.end(); iter++){
		//incr++;
		index = gameToInt(soldierIDs[i]);
		
		std::string tmpStr; //形如army#1
		std::string getCMD = "LINDEX " + armyListKey + " " + soldierIDs[i];
		ret = strReadExec(getCMD, tmpStr);
		if(!ret){
			return false;
		}
		
		if(tmpStr == "#"){
			continue;
		}
		
		
		
		short category = gameToInt(tmpStr.substr(5,1));
		std::string field = tmpStr + "#max";
		std::string strLevel;
		ret = strReadExec("HGET user#" + strUid + " " +field, strLevel);
		/*if(!ret){
			return false;
		}*/
		short level = 1;//(short)gameToInt(strLevel);
		
		soldier tmpSoldier;
		tmpSoldier.category = category;
		tmpSoldier.level = level;
		tmpSoldier.sid = soldierIDs[i];
		cmd = "HGET " + listKey + "#camp " + soldierIDs[i];
		std::string building_sn;
		ret = strReadExec(cmd, building_sn);
		/*if(!ret){
			return false;
		}*/
		tmpSoldier.camp_uuid = building_sn;
		soldiers.push_back(tmpSoldier);
	}
	
	return true;
}

bool RedisClient::getUsernameByID(int uid, std::string& output){
	std::string strUid = gameToString(uid);
	std::string cmd = "HGET user#" + strUid + " uname";
	bool ret = strReadExec(cmd, output);
	return ret;
}

bool RedisClient::getBuildingNumByID(int uid, std::string& output){
	std::string strUid = gameToString(uid);
	std::string cmd = "HGET user#" + strUid + " buildingsNum";
	bool ret = strReadExec(cmd, output);
	return ret;
}

bool RedisClient::getPlayerPointsByID(int uid, std::string& output){
	std::string strUid = gameToString(uid);
	std::string cmd = "HGET user#" + strUid + " abt#point";
	bool ret = strReadExec(cmd, output);
	return ret;
}

bool RedisClient::plusPlayerPoint(int uid, int points){
	std::string strUid = gameToString(uid);
	std::string incrNum = gameToString(points);
	std::string cmd = "HINCRBY user#" + strUid + " abt#point " + incrNum;
	bool ret = strWriteExec(cmd);
	return ret;
}

bool RedisClient::getSoldiersByPlayerID(int uid, std::vector<soldier>& soldiers){
	std::string strUid = gameToString(uid);
	std::string listKey = "army#" + strUid;
	std::string getArmyCmd = "lrange " + listKey + " 0 -1";
	std::vector<string> soldierIDs;
	bool ret = multiReadExec(getArmyCmd, soldierIDs);
	if(!ret){
		return false;
	}
	std::vector<std::string>::iterator iter;
	int index = 0;
	std::string cmd;
	for(iter = soldierIDs.begin(); iter != soldierIDs.end(); iter++){
		if(*iter == "#"){
			index++;
			continue;
		}
		//index = gameToInt(*iter);
		std::string categortStr = (*iter).substr(5,1);
		short category = (short)gameToInt(categortStr);
		
		
		std::string tmpStr; 
		std::string getCMD = "HGET user#" + strUid + " " + *iter + "#max";
		ret = strReadExec(getCMD, tmpStr);
		/*if(!ret){
			return false;
		}*/
		short level = 1;//(short)gameToInt(tmpStr);
		
		soldier tmpSlder;
		tmpSlder.category = category;
		tmpSlder.level = level;
		tmpSlder.sid = gameToString(index);
		//获取营地
		cmd = "HGET army#" + strUid + "#dfd#camp " + tmpSlder.sid;
		ret = strReadExec(cmd, tmpStr);
		//TODO
		if(!ret){
			//return false;
			tmpStr = "";
		}
		tmpSlder.camp_uuid = tmpStr;
		soldiers.push_back(tmpSlder);
		index++;
	}
	return true;
}

bool RedisClient::getPlayerIDByUname(std::string uname, unsigned int& uid){
	std::string cmd = "HGET nickname " + uname;
	std::string strUid;
	bool ret = strReadExec(cmd, strUid);
	if(!ret){
		return false;
	}
	uid = gameToUnsignedInt(strUid);
	return true;
} 

bool RedisClient::getBuildingCurRes(int uid, std::string buildingID, int& resNum){
	std::string strUid = gameToString(uid);
	std::string cmd = "HGET build#" + strUid + "#" + buildingID + " cur_resource";
	std::string strResNum;
	bool ret = strReadExec(cmd, strResNum);
	if(!ret){
		return false;
	}
	resNum = gameToInt(strResNum);
	return true;
}

bool RedisClient::getBuildingType(int uid, std::string buildingID, int& type){
	std::string strUid = gameToString(uid);
	std::string cmd = "HGET build#" + strUid + "#" + buildingID + " build_type";
	std::string strBuildingType;
	bool ret = strReadExec(cmd, strBuildingType);
	if(!ret){
		return false;
	}
	type = gameToInt(strBuildingType);
	return true;
}

bool RedisClient::setSoldierDie(int uid, std::string soldierID){
	std::string strUid = gameToString(uid);
	//std::string decrementSoldiersNumCMD = "HINCRBY user#" + strUid + " "
	//读出类型
	std::string cmd = "LINDEX army#" + strUid + " " + soldierID;
	std::string sType;
	bool ret = strReadExec(cmd, sType);
	if(!ret){
		return false;
	}
	//设置成"#"
	cmd = "LSET army#" + strUid + " " + soldierID + " #";
	ret = strWriteExec(cmd);
	if(!ret){
		return false;
	}
	
	cmd = "HINCRBY user#" + strUid + " " + sType + " -1";
	ret = strWriteExec(cmd);
	if(!ret){
		return false;
	}
	return true;
}

bool RedisClient::setDfdSoldier(int uid, std::string soldierID, std::string build_sn){
	std::string strUid = gameToString(uid);
	//先删除该ID
	std::string listKey = "army#" + strUid + "#dfd";
	std::string cmd = "LREM " + listKey + " 0 " + soldierID;
	bool ret = strWriteExec(cmd);
	
	cmd = "RPUSH " + listKey + " " + soldierID;
	ret = strWriteExec(cmd);
	cmd = "HSET army#" + strUid + "#dfd#camp " + soldierID + " " + build_sn;
	ret = strWriteExec(cmd);
	
	return true;
}

bool RedisClient::cancelDfdSoldier(int uid, std::string soldierID){
	std::string strUid = gameToString(uid);
	//先删除该ID
	std::string listKey = "army#" + strUid + "#dfd";
	std::string cmd = "LREM " + listKey + " 0 " + soldierID;
	bool ret = strWriteExec(cmd);
	//删除营地对应信息
	cmd = "HDEL " + listKey + "#camp " + soldierID;
	ret = strWriteExec(cmd);
	
	return true;
	
}
