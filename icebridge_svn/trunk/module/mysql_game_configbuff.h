/*
 * mysql_game_config.h
 *
 *  Created on: Nov 15, 2013
 *      Author: zhongbing
 */

#ifndef MYSQL_GAME_CONFIG_H_
#define MYSQL_GAME_CONFIG_H_
#include <mysql_base.h>
#include <msgpack_struct.h>
#include <boost/container/vector.hpp>
class MyBuffer
{
private:
	//MyBuffer(){}
	char* p;
	int len;
public :
	char* data()
	{
		return p;
	}
	int size()
	{
		return len;
	}
	MyBuffer()
	{
		p = NULL;
		len = 0;
	}
	MyBuffer(const char* position,int leng)
	{
		len =leng;
		p = new char[leng];
		memcpy(p,position,leng);
	}
	MyBuffer(const MyBuffer& b)
	{
		this->len = b.len;
		this->p = new char[this->len];
		memcpy(this->p,b.p,this->len);
		printf("exsit copy\n");
	}
	void Write(const char* position,int leng)
	{
		len =leng;
		p = new char[leng];
		memcpy(p,position,leng);
	}
	~MyBuffer()
	{
		if(p)
		{
			delete[] p;
		}
	}
};

typedef struct __config_buffer
{
	boost::container::vector<MyBuffer> arms_list_buffer;
	boost::container::vector<MyBuffer> build_list_buffer;
	boost::container::vector<MyBuffer> skill_config_buffer;

	MyBuffer city_config;

}ConfigBuffer;

struct HeroBaseConfig
{
	int quality;
	int all_min;
	int all_max;
	int s_min;
	int s_min_grow;
	int s_max;
	int s_max_grow;
	int s_min_limit_max;
	int s_min_limit_grow;
	int s_max_limit_min;
	int s_max_limit_grow;
	int goods_space;
	int max_space;
	int pre_skill;
	int study_skill;
	int special_name;
	int base_hp;
	int base_mp;
	int base_attack;
	int base_defence;
};

struct HeroUpdateConfig
{
	int type;
	int diamond;
	int hero_num;
	int cool_time;
	int start1;
	int start2;
	int start3;
	int start4;
	int start5;
};

void InitGlobalData();

void ReadGameVersion();
void ReadArmsData();
void ReadBuildData();
void ReadCityLimit();
void ReadSkillConfig();
void ReadCombatLimit();

void ReadHeroUpdateConfig();
void ReadHeroBaseConfig();

#endif /* MYSQL_GAME_CONFIG_H_ */
