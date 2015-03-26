/*
 * mysql_base.h
 *
 *  Created on: Oct 28, 2013
 *      Author: root
 */
#ifndef MYSQL_BASE_H
#define MYSQL_BASE_H

#include <mysql/mysql.h>
#include <vector>
#include <list>
#include <map>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

class MysqlBase
{
private:
	MYSQL* init_connect();
	int get_connect();
	void release_connect(int connect_pool_index);
	MysqlBase(){}
	MysqlBase(const MysqlBase&){}
	MysqlBase(int connect_min_num );
	string HOST,USERNAME, PASSWORD, DATABASE;
	int DATAPORT;
public:
	class Writer
	{
	friend class MysqlBase;
	private:
		MYSQL_RES* res_ptr;
		int index;
		MysqlBase* base;
		Writer(){}
		Writer(const Writer&){}
	public:
		Writer(MysqlBase* base_a,const char* sql);
		virtual ~Writer();
	};

	class Reader
	{
	friend class MysqlBase;
	private:
		MYSQL_RES* res_ptr;
		int index;
		MysqlBase* base;
		Reader(){}
		Reader(const Writer&){}
	public:
		Reader(MysqlBase* base_a,const char* sql);
		bool is_sucess;
		char ** get_one_row();
		virtual ~Reader();
	};
private:
	int use_connect_num;
	int connect_min_num;
	int connect_max_num;
	list<int> index_list;
	vector< MYSQL* > mysql_connect_pool;
	static map<pid_t,MysqlBase*> object_map;//one thread and only one object

public :
	static MysqlBase* get_instance(int connect_min_num = 5);//create an  object if this pthread hasn`t an object;
	virtual ~MysqlBase();

};

#endif //MYSQL_BASE_H
