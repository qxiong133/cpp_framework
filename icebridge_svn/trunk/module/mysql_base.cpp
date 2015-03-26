/*
 * mysql_base.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: root
 */
#include "mysql_base.h"
#include <unistd.h>
#include <GameConfig.h>


MysqlBase::MysqlBase(int connect_min_num):connect_min_num(connect_min_num),connect_max_num(20000)
{
	HOST = GameConfig::getInstance().getString("mysqlMaster.ip");
	USERNAME = GameConfig::getInstance().getString("mysqlMaster.user");
	PASSWORD = GameConfig::getInstance().getString("mysqlMaster.passwd");
	DATABASE = GameConfig::getInstance().getString("mysqlMaster.database");
	DATAPORT = GameConfig::getInstance().getInt("mysqlMaster.port");
	use_connect_num = 0;
	for(int i = 0;i<connect_min_num;i++)
	{
		mysql_connect_pool.push_back(init_connect());
		index_list.push_back(i);
	}
}
MysqlBase::~MysqlBase()
{
	for(list<int>::iterator it = index_list.begin();it!=index_list.begin();it++)
	{
		mysql_close(mysql_connect_pool[*it]);
	}
}
MYSQL* MysqlBase::init_connect()
{
	MYSQL* connect_ptr = mysql_init(NULL);
	connect_ptr = mysql_real_connect(connect_ptr, HOST.c_str(),USERNAME.c_str(), PASSWORD.c_str(), DATABASE.c_str(), DATAPORT, NULL, 0);
	if (!connect_ptr) {
		printf("Connection or mysql init failed,please check mysql connect\n");
		exit(0);
		return NULL;
	}
	connect_ptr->reconnect = 1;
	if (mysql_query(connect_ptr, "set names utf8;") != 0)
	{
		printf("Please Set mysql char utf8\n");
	}
	return connect_ptr;
}
int MysqlBase::get_connect()
{
	if( ++use_connect_num > mysql_connect_pool.size())//expand connect
	{
		if(use_connect_num > connect_max_num)
		{
			printf("Connect is so many too I can`t run\n");
			//exit(0);
			return  -3;
		}
		mysql_connect_pool.push_back(init_connect());
		return use_connect_num - 1;
	}
	else
	{
		int index;
		if(index_list.size() > 0)
		{
			index = index_list.back();
			index_list.pop_back();
		}
		else
		{
			index = mysql_connect_pool.size();
			mysql_connect_pool.push_back(init_connect());
		}
		return index;
	}
}
void MysqlBase::release_connect(int connect_pool_index)
{
	if(connect_pool_index >= connect_min_num)//delete expand connect
	{
		mysql_close(mysql_connect_pool[connect_pool_index]);
	}
	else
	{
		index_list.push_back(connect_pool_index);
	}
	--use_connect_num;
}
map<pid_t,MysqlBase*> MysqlBase::object_map;
MysqlBase* MysqlBase::get_instance(int connect_min_num)
{
	if(connect_min_num < 0)
	{
		printf("erro paramter");
		return  NULL;
	}
	pid_t pid = getpid();
	if(object_map.find(pid) == object_map.end())//not exsit
	{
		//MysqlBase * base = new MysqlBase(connect_min_num);
		//object_map[pid] = base;
		//bject_map.insert(map<pid_t,MysqlBase*>::value_type(pid,base));
		object_map[pid] = new MysqlBase(connect_min_num);
	}
	return object_map[pid];
}

MysqlBase::Writer::Writer(MysqlBase* base_a,const char* sql):res_ptr(NULL),base(base_a)
{
	index = base->get_connect();
	if (mysql_query(base->mysql_connect_pool[index], sql))
	{
		printf("mysql is error:\n %s\ndatabase say:%s\n",sql,mysql_error(base->mysql_connect_pool[index]));
		if( 1043 == mysql_errno(base->mysql_connect_pool[index]))//disconnect
		{
			if (mysql_query(base->mysql_connect_pool[index], sql))//because reconnect = 1,so is auto reconnect
			{
				printf("connect is invalid,please check network or database connect");
				//exit(0);
			}
		}
	}
}
MysqlBase::Writer::~Writer()
{
	base->release_connect(index);
}

MysqlBase::Reader::Reader(MysqlBase* base_a,const char* sql):res_ptr(NULL),base(base_a)
{
	index = base->get_connect();
	if (mysql_query(base->mysql_connect_pool[index], sql))
	{
		printf("mysql is error:\n %s\ndatabase say:%s\n",sql,mysql_error(base->mysql_connect_pool[index]));
		if(1043 == mysql_errno(base->mysql_connect_pool[index]))//disconnect
		{
			if (mysql_query(base->mysql_connect_pool[index], sql))//because reconnect = 1,so is auto reconnect
			{
				printf("connect is invalid,please check network or database connect");
				//exit(0);
			}
			else
			{
				res_ptr = mysql_store_result(base->mysql_connect_pool[index]);
			}
		}
	}
	else
	{
		res_ptr = mysql_store_result(base->mysql_connect_pool[index]);
	}
}
MysqlBase::Reader::~Reader()
{
	base->release_connect(index);
	if(res_ptr)
	{
		mysql_free_result(res_ptr);
	}
}


char ** MysqlBase::Reader::get_one_row()
{
	if(res_ptr)
	{
		return mysql_fetch_row(res_ptr);
	}
	else
	{
		printf("data is errro");
		return NULL;
	}
}
