/*
 * mysql_login_worker.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: zhongbing
 */
#include "mysql_login_worker.h"
#include <stdlib.h>
#include <string.h>
extern MysqlBase* mysql_main_thread;

MysqlLoginWorker::MysqlLoginWorker()
{

}
MysqlLoginWorker::~MysqlLoginWorker()
{

}

bool check_string_space(const char* str_bigin,int length)
{
	bool num_flag = true;// if 1 all char is num

	for(int i = 0;i<length;i++)
	{
		if(str_bigin[i] == ' ' || str_bigin[i] == '\'' || str_bigin[i] == '=' || str_bigin[i] == ';')return false;
	}

	return num_flag;
}


void MysqlLoginWorker::user_register(std::string& account,std::string& passwd,unsigned int& uuid)
{
	char uuid_char[33] = {};//final char is '\0';fangbian convert string type
	char passwd_char[10] = {};
	string sql = "select replace(uuid(),'-','');";
	MysqlBase::Reader reader1(mysql_main_thread,sql.c_str());
	if(char** row = reader1.get_one_row())
	{
		memcpy(uuid_char,row[0],32);
		uuid_char[32] = '\0';
		account = uuid_char;
	}
	else
	{
		printf("generate uuid failed!\n");
		return ;
	}
	MysqlBase::Reader reader2(mysql_main_thread,sql.c_str());
	if(char** row = reader2.get_one_row())
	{
		memcpy(passwd_char,row[0] + time(0)%20,9);
		passwd_char[9] = '\0';
		passwd =passwd_char;
	}
	else
	{
		printf("generate passwd failed!\n");
		return ;
	}
	sql =  "insert user_info(username,password,uuid) values('" + account + "','" + passwd + "','" + account +"');";
	MysqlBase::Writer writer(mysql_main_thread,sql.c_str());
	sql =  "select id from user_info where username = '" + account + "';";
	MysqlBase::Reader reader3(mysql_main_thread,sql.c_str());
	string id;
	if(char** row = reader3.get_one_row())
	{
		if(row[0])
		{
			id = row[0];
			uuid = atoi(row[0]);
		}
	}
	else
	{
		printf("generate passwd failed!\n");
		return ;
	}
	sql =  "insert account_info(user_id,region,balance) values('" + id + "',0,20000);";
	MysqlBase::Writer writer1(mysql_main_thread,sql.c_str());
}

bool MysqlLoginWorker::verify_user(const string& account,const string& passwd,unsigned int &uuid,int& diamond)
{
	if(!check_string_space(account.c_str(),account.length())||
			!check_string_space(passwd.c_str(),passwd.length()))
	{
		return false;
	}
	string sql = "select id from user_info where username = '" + account + "' and password = '" + passwd + "';";
	MysqlBase::Reader reader(mysql_main_thread,sql.c_str());
	if(char** row = reader.get_one_row())
	{
		uuid = atoi(row[0]);
		string str = row[0];
		sql = "select balance from account_info where user_id = '" + str + "';";
		MysqlBase::Reader reader2(mysql_main_thread,sql.c_str());
		if(char** row = reader2.get_one_row())
		{
			diamond = atoi(row[0]);
		}
		return true;
	}
	return false;
}


bool MysqlLoginWorker::bind_user_email(const string& email,const string& passwd,unsigned int uuid)
{
	if(!check_string_space(email.c_str(),email.length())||
			!check_string_space(passwd.c_str(),passwd.length()))
	{
		return false;
	}
	string sql = "select * from user_info where username = '" + email + "';";
	MysqlBase::Reader reader(mysql_main_thread,sql.c_str());
	if(char** row = reader.get_one_row())
	{
		return false;
	}
	ostringstream  os;
	os << "update user_info set username ='" << email << "', password = '" << passwd << "' where id = '" << uuid << "';";
	//printf("%s\n",os.str().c_str());
	MysqlBase::Writer writer(mysql_main_thread,os.str().c_str());

	return true;

}


void MysqlLoginWorker::update_diamond(const int diamond,unsigned int uuid)
{
	ostringstream  os;
	os << "update account_info set balance ='" << diamond <<  "' where user_id = '" << uuid << "';";
	//printf("%s\n",os.str().c_str());
	MysqlBase::Writer writer(mysql_main_thread,os.str().c_str());

}

int MysqlLoginWorker::get_diamond(unsigned int uuid)
{
	ostringstream  os;
	os<<"select balance from account_info where user_id = '" << uuid << "';";
	MysqlBase::Reader reader(mysql_main_thread,os.str().c_str());
	if(char** row = reader.get_one_row())
	{
		return atoi(row[0]);
	}
	return 0;
}
