#include <iostream>
#include "MysqlManager.h"
#include "ConstParam.h"
#include <boost/make_shared.hpp>
//#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/Session.h"
//#include "Poco/Data/RecordSet.h"
//#include "Poco/Data/Column.h"
#include "Poco/Data/MySQL/MySQL.h"
#include "Poco/Data/MySQL/Connector.h"
//#include "Poco/Data/SessionPool.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
//#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Poco/Exception.h"
#include <stdio.h>

#include "GameConfig.h"

using Poco::UUID;
using Poco::UUIDGenerator;
using namespace Poco::Data;
using namespace std;

MysqlManager::MysqlManager() {
    Poco::Data::MySQL::Connector::registerConnector();
    //_pool = boost::make_shared<SessionPool>("MySQL", "user=root;password=root;db=dev_game;host=192.168.1.115;port=8888", 1, 50);
    char tmp[250];
    sprintf(tmp,"user=%s;password=%s;db=%s;host=%s;port=%s",
GameConfig::getInstance().getString("mysqlMaster.user").c_str(),
GameConfig::getInstance().getString("mysqlMaster.passwd").c_str(),
GameConfig::getInstance().getString("mysqlMaster.database").c_str(),
GameConfig::getInstance().getString("mysqlMaster.ip").c_str(),
GameConfig::getInstance().getString("mysqlMaster.port").c_str()
);
    _pool = boost::make_shared<SessionPool>("MySQL", tmp, 1, 50);
}

MysqlManager::~MysqlManager() {
}

int MysqlManager::regUser(const std::string email, const std::string psw) {
	UUIDGenerator& generator = UUIDGenerator::defaultGenerator();
	string uuid = generator.create().toString();
	Session sess = _pool->get();
	Poco::UInt32 ret;
	Poco::UInt32 uid = 0;
	try {
		Statement insert(sess);
		insert << "INSERT INTO user_info(username, password, uuid) VALUES (?, ?, ?)", use(email), use(psw), use(uuid);
		ret = insert.execute();
	} catch (DataException& e) {
		return DB_INTERACTICE_USER_EXIST;
	}
	if (0 != ret) {
		return DB_INTERACTICE_FAILED;
	}
	sess << "SELECT LAST_INSERT_ID()", into(uid), now;
	if (0 == uid) {
		return DB_INTERACTICE_FAILED;
	}
	return uid;
}

int MysqlManager::changePsw(const unsigned int uid, const std::string oldPsw, const std::string newPsw) {
	Session sess = _pool->get();
	string oriPsw;
	sess << "SELECT password FROM user_info WHERE id=?", into(oriPsw), use(uid), now;
	cout << oriPsw << endl;
	if (oriPsw != oldPsw) {
		return DB_INTERACTICE_ORI_PSW_ERROR;
	}
	Statement change(sess);
	change << "UPDATE user_info SET password=? WHERE id=? ", use(newPsw), use(uid);
	Poco::UInt32 ret = change.execute();
	if (0 != ret) {
		return DB_INTERACTICE_FAILED;
	}
	return DB_INTERACTICE_SUCCESS;
}

int MysqlManager::verifyUserPsw(const std::string email, const std::string psw) {
	Session sess = _pool->get();
	Poco::UInt32 uid = 0;
	string password;
	try {
		sess << "SELECT id,password FROM user_info WHERE username=?", use(email), into(uid), into(password), now;
	} catch (DataException& e) {
		return DB_INTERACTICE_FAILED;
	}
	if (0 == uid) {
		return DB_INTERACTICE_USER_NOT_EXIST;
	}
	if (password == psw) {
		return uid;
	}
	return DB_INTERACTICE_FAILED;
}

bool MysqlManager::subDiamond(const unsigned int uid, int diamond_to_sub){
	Session sess = _pool->get();
    Poco::UInt32 diamond_num= 0;

	sess << "SELECT balance FROM account_info WHERE user_id=?", into(diamond_num), use(uid), now;
	cout << diamond_num << endl;
	if (diamond_num < diamond_to_sub) {
        cout << "the total diamond num "<< diamond_num << "is less than " << diamond_to_sub << endl;
		return false;
	}
	Statement change(sess);
	change << "UPDATE account_info SET balance=? WHERE user_id=? ", use(diamond_num - diamond_to_sub), use(uid);
	Poco::UInt32 ret = change.execute();
	if (0 != ret) {
        cout << "mysql update diamond fail!" << endl;
		return false;
	}
	return true;
}

