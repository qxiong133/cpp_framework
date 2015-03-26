/*
 * redis_word_map.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: zhongbing
 */

#include "redis_word_map.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

using namespace std;

RedisWordMap::RedisWordMap()
{
	redis =  RedisBase::get_instance();
}
RedisWordMap::~RedisWordMap()
{

}

bool split_x_y(char * str,short & x,short& y)
{
	char temp[20] = {};
	char *startp = str;
	char *endp = str;
	if((endp = strstr(startp,"#")) > 0)
	{
		memset(temp,0,sizeof(temp));
		memcpy(temp,startp,endp-startp);
		try
		{
			x= atoi(temp);
			y = atoi(endp + 1);
			if(x >= 0 && x < 640 && y >= 0 && y < 640)
			{
				return true;
			}
		}
		catch(...)
		{
			return false;
		}
	}
	return false;
}

void RedisWordMap::word_map_player_init(unsigned char world_position[640][640],NearByPlayerInfoResponse  player_info[6400])
{
	RedisBase::SimplerReader reader(redis,"HGETALL worldmap");

	redisReply* result = reader.get_result();

	if(result)
	{
		bool last_data_is_erro = false;
		for(int i = 0;i<result->elements;i+=2)
		{
			if(last_data_is_erro)continue;
			short position_x;
			short position_y;
			if(split_x_y(result->element[i]->str,position_x,position_y))
			{
				printf("=============%s x is %d,y is %d\n",result->element[i]->str,position_x,position_y);
				world_position[position_y][position_x] = 0xff;
				unsigned int uuid = (unsigned int)(atoi(result->element[i + 1]->str));
				WorldMapPlayerInfoResponse player_worldmap;
				get_player_att(player_worldmap,uuid);
				int area = (position_y/8)*80 + position_x/8;
				player_info[area].area = area;
			//	int coordx = position_x;
			//	int coordy = position_y;
			//	int key = (coordx << 16) + coordy;
			//	printf("key is %d=============%s x is %d,y is %d\n",key,result->element[i]->str,position_x,position_y);
				player_info[area].player_info[(position_x << 16) + position_y] = player_worldmap;

				PlayerSimpleInfo info;
				info.x = position_x;
				info.y = position_y;
				info.player_name = player_worldmap.player_name;
				info.city_level = player_worldmap.city_level;
				simple_player_data.AddMember(info);

			}
			else
			{
				last_data_is_erro = true;
			}
		}
	}

}

bool RedisWordMap::get_player_worldinfo(short& x,short& y ,unsigned int UUID)
{
	char temp[1024] = {};
	sprintf(temp,"HMGET user#%u coord#x coodrd#y",UUID);
	RedisBase::SimplerReader reader(redis,temp);
	redisReply* result = reader.get_result();
	if(result)
	{
		if(result->element[0]->str && result->element[1]->str)
		{
			x = atoi(result->element[0]->str);
			y = atoi(result->element[1]->str);
			return true;
		}
	}
	return false;
}

void RedisWordMap::add_new_player(short x,short y,unsigned int UUID)
{
	//del uuid
	char temp[1024] = {};
	sprintf(temp,"HMGET user#%u coord#x coodrd#y",UUID);
	RedisBase::SimplerReader reader(redis,temp);
	redisReply* result = reader.get_result();
	if(result)
	{
		if(result->element[0]->str && result->element[1]->str)
		{
			memset(temp,0,sizeof(temp));
			sprintf(temp,"HDEL worldmap %d#%d", atoi(result->element[0]->str), atoi(result->element[1]->str));
			RedisBase::SimpleWriter writer(redis,temp);
		}
	}
	memset(temp,0,sizeof(temp));
	sprintf(temp,"HSET worldmap %d#%d  %u",x,y,UUID);
	RedisBase::SimpleWriter writer(redis,temp);
	memset(temp,0,sizeof(temp));
	sprintf(temp,"HMSET user#%u coord#x %d coord#y %d",UUID,x,y);
	RedisBase::SimpleWriter write1r(redis,temp);
}
void RedisWordMap::save_player_worldinfo(short x,short y,unsigned int UUID)
{
	char temp[64] = {};
	sprintf(temp,"HSET user#lastposition %u %d#%d",x,y,UUID);
	RedisBase::SimpleWriter writer(redis,temp);
}

void RedisWordMap::get_palyer_last(short& x,short& y,unsigned int UUID)
{
	char temp[64] = {};
	sprintf(temp,"HGET user#lastposition %u",x,y,UUID);
	RedisBase::SimplerReader reader(redis,temp);
	redisReply* result = reader.get_result();
	if(result)
	{
		if(!split_x_y(result->str,x,y))
		{
			return ;
		}
	}
	else
	{
		char temp1[1024] = {};
		sprintf(temp1,"HMGET user#%u coord#x coord#y",UUID);
		RedisBase::SimplerReader reader2(redis,temp1);
		redisReply* result2 = reader2.get_result();
		if(result2)
		{
			x = result2->element[0]->str ? atoi(result2->element[0]->str) : 320;
			y = result2->element[1]->str ? atoi(result2->element[1]->str) : 320;
		}
	}

}

void RedisWordMap::get_player_att(WorldMapPlayerInfoResponse & p,unsigned int UUID)
{
	ostringstream  os;
	os<<"HMGET user#" << UUID
	  << " race nickname city_hall_uuid ";
	RedisBase::SimplerReader reader(redis,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		p.player_race = result->element[0]->str?atoi(result->element[0]->str):0;
		p.player_name = result->element[1]->str?result->element[1]->str:"Not_Exsit_Name";
		string city_hall_uuid = result->element[2]->str?result->element[2]->str:"Not_Exsit_city_hall_uuid";
		ostringstream  os1;
		os1<<"HGET build#" << UUID<<"#"<<city_hall_uuid<< "  build_level  ";
		RedisBase::SimplerReader reader1(redis,os1.str().c_str());
		redisReply* result2 = reader1.get_result();
		if(result2->type == REDIS_REPLY_STRING)
		{
			p.city_level = result2->str?atoi(result2->str):1;
			p.city_face = 0;
		}
	}
}

bool RedisWordMap::get_random_player(vector<PlayerSimpleInfo>& PlayerSimpleInfo,int n)
{
	return simple_player_data.GetSomeData(PlayerSimpleInfo,n);
}

void RedisWordMap::add_random_player(PlayerSimpleInfo & PlayerSimpleInfo)
{
	simple_player_data.AddMember(PlayerSimpleInfo);
}
