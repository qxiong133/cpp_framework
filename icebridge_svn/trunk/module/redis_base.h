/*
 * redis_base.h
 *
 *  Created on: Nov 14, 2013
 *      Author: zhongbing
 */

#ifndef REDIS_BASE_H_
#define REDIS_BASE_H_

#include <hiredis/hiredis.h>
#include <list>
#include <map>
#include <pthread.h>
#include <string>

using namespace std;

class RedisBase
{
private:

	string REDIS_IP;
	int REDIS_PORT;
	string REDIS_PASSWD;

	redisContext *m_pRedisContext;
	static map<pid_t,RedisBase*> object_map;
	RedisBase(const RedisBase&){}
	RedisBase();
public:
	class SimpleWriter
	{
	friend class RedisBase;
	private:
		RedisBase* base;
		SimpleWriter(){}
		SimpleWriter(const SimpleWriter&){}
	public:
		SimpleWriter(RedisBase* base_a,const char* redis_sql);
		virtual ~SimpleWriter();
	};

	class SimplerReader
	{
	friend class RedisBase;
	private:
		redisReply* result;
		RedisBase* base;
		SimplerReader(){}
		SimplerReader(const SimplerReader&){}
	public:
		SimplerReader(RedisBase* base_a,const char* redis_sql);
		redisReply* get_result();
		virtual ~SimplerReader();
	};

	class MultiOperate
	{
	friend class RedisBase;
	private:
		redisReply* result;
		RedisBase* base;
		MultiOperate(){}
		MultiOperate(const MultiOperate&){}
	public:
		MultiOperate(RedisBase* base_a,list<const char*>& redis_sql_list);
		redisReply* get_result();
		virtual ~MultiOperate();
	};

private:
	void init_connect();

public :
	static RedisBase* get_instance();//create an  object if this pthread hasn`t an object;
	virtual ~RedisBase();

};



#endif /* REDIS_BASE_H_ */
