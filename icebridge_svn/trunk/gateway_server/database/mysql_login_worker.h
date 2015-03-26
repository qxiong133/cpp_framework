/*
 * mysql_login_worker.h
 *
 *  Created on: Nov 20, 2013
 *      Author: zhongbing
 */

#ifndef MYSQL_LOGIN_WORKER_H_
#define MYSQL_LOGIN_WORKER_H_
#include <mysql_base.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

class MysqlLoginWorker
{
private:


	friend class LoginWorker;

	MysqlLoginWorker();

	MysqlLoginWorker(const MysqlLoginWorker& r){
	}

	//handle

	void user_register(std::string& account,std::string& passwd,unsigned int& uuid);//end

	bool verify_user(const string& account,const string& passwd,unsigned int &uuid,int& diamond);

	bool bind_user_email(const string& email,const string& passwd,unsigned int uuid);//end

	void update_diamond(const int diamond,unsigned int uuid);//end

	int get_diamond(unsigned int uuid);


public:
	virtual ~MysqlLoginWorker();

};




#endif /* MYSQL_LOGIN_WORKER_H_ */
