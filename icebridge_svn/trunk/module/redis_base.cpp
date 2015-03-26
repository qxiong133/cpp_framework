/*
 * redis_base.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: zhongbing
 */
#include "redis_base.h"
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/foreach.hpp>
#include <GameConfig.h>


RedisBase::RedisBase()
{
	REDIS_IP = GameConfig::getInstance().getString("redisMaster.ip");
	//REDIS_PASSWD = GameConfig::getInstance().getString("redisMaster.passwd");
	REDIS_PORT = GameConfig::getInstance().getInt("redisMaster.port");
	printf("redis connect to ip is %s",GameConfig::getInstance().getString("redisMaster.ip").c_str());	
	printf("redis port is %d", REDIS_PORT);	
	init_connect();
}

void RedisBase::init_connect()
{
    timeval timeout = { 1, 500000 };
	//m_pRedisContext is connect biaoji
	m_pRedisContext = redisConnectWithTimeout(REDIS_IP.c_str(), REDIS_PORT, timeout);
	if (m_pRedisContext->err) {
		printf("log, redis connect error\n");
		exit(0);
	}
	/*redisReply *reply = static_cast<redisReply*>(redisCommand(m_pRedisContext,
			"AUTH %s", REDIS_PASSWD.c_str()));
	if (reply->type == REDIS_REPLY_ERROR) {
		std::cout << "log, redis passwd error, passwd is:" <<REDIS_PASSWD<< m_pRedisContext->errstr << std::endl;
		exit(0);
	}
	freeReplyObject(reply);
	reply = static_cast<redisReply*>(redisCommand(m_pRedisContext,"HGETALL worldmap"));
	if (!reply) {
		std::cout << "log, redis passwd error, " << m_pRedisContext->errstr << std::endl;
		exit(0);
	}
	//std::cout << "AUTH " << passwd <<"   " << reply->str << std::endl;
	freeReplyObject(reply);*/

}
RedisBase::~RedisBase()
{
	redisFree(m_pRedisContext);
}

map<pid_t,RedisBase*> RedisBase::object_map;
RedisBase* RedisBase::get_instance()
{
	pid_t pid = getpid();
	if(object_map.find(pid) == object_map.end())
	{
		object_map[pid] = new RedisBase();
	}
	return object_map[pid];
}

RedisBase::SimpleWriter::SimpleWriter(RedisBase* base_a,const char* redis_sql):base(base_a)
{
	redisReply* reply = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,redis_sql));
	if(reply->type == REDIS_REPLY_ERROR)
	{
		std::cout << "redis error, " << base->m_pRedisContext->errstr << std::endl;
		if(base->m_pRedisContext->err == REDIS_DISCONNECTING)
		{//reconect
			freeReplyObject(reply);
			redisFree(base->m_pRedisContext);
			base->init_connect();
			redisReply* reply2 = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,redis_sql));
			if(reply->type == REDIS_REPLY_ERROR && base->m_pRedisContext->err == REDIS_DISCONNECTING)
			{
				std::cout << "network is erro,please check it,redis erro" << base->m_pRedisContext->errstr << std::endl;
				//exit(0);
			}
			freeReplyObject(reply2);
		}
	}
}

RedisBase::SimpleWriter::~SimpleWriter()
{

}

RedisBase::SimplerReader::SimplerReader(RedisBase* base_a,const char* redis_sql):base(base_a)
{
	result = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,redis_sql));
	if(result->type == REDIS_REPLY_ERROR)
	{
		std::cout << "redis error, " << base->m_pRedisContext->errstr << std::endl;
		if(base->m_pRedisContext->err == REDIS_DISCONNECTING)
		{//reconect
			freeReplyObject(result);
			redisFree(base->m_pRedisContext);
			base->init_connect();
			redisReply* reply2 = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,redis_sql));
			if(result->type == REDIS_REPLY_ERROR && base->m_pRedisContext->err == REDIS_DISCONNECTING)
			{
				std::cout << "network is erro,please check it,redis erro" << base->m_pRedisContext->errstr << std::endl;
				//exit(0);
			}
		}
	}
}
redisReply* RedisBase::SimplerReader::get_result()
{
	if(result->type == REDIS_REPLY_ERROR)
	{
		return NULL;
	}
	return result;
}
RedisBase::SimplerReader::~SimplerReader()
{
	freeReplyObject(result);
}

RedisBase::MultiOperate::MultiOperate(RedisBase* base_a,list<const char*>& redis_sql_list):base(base_a)
{
	freeReplyObject(redisCommand(base->m_pRedisContext,"MULTI"));
	BOOST_FOREACH(const char* & value,redis_sql_list)
	{
		freeReplyObject(redisCommand(base->m_pRedisContext,value));
	}

	result = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,"EXEC"));
	if(result->type == REDIS_REPLY_ERROR)
	{
		std::cout << "redis error, " << base->m_pRedisContext->errstr << std::endl;
		if(base->m_pRedisContext->err == REDIS_DISCONNECTING)
		{//reconect
			freeReplyObject(result);
			redisFree(base->m_pRedisContext);
			base->init_connect();
			freeReplyObject(redisCommand(base->m_pRedisContext,"MULTI"));
			BOOST_FOREACH(const char* & value,redis_sql_list)
			{
				freeReplyObject(redisCommand(base->m_pRedisContext,value));
			}
			result = static_cast<redisReply*>(redisCommand(base->m_pRedisContext,"EXEC"));
			if(result->type == REDIS_REPLY_ERROR && base->m_pRedisContext->err == REDIS_DISCONNECTING)
			{
				std::cout << "network is erro,please check it,redis erro" << base->m_pRedisContext->errstr << std::endl;
				exit(0);
			}
		}
	}
}
redisReply* RedisBase::MultiOperate::get_result()
{
	if(result->type == REDIS_REPLY_ERROR)
	{
		return NULL;
	}
	return result;
}
RedisBase::MultiOperate::~MultiOperate()
{
	freeReplyObject(result);
}

