/*
 * mysql_game_config.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: zhongbing
 */
#include <stdio.h>
#include "mysql_game_configbuff.h"
#include <time.h>



#include <iostream>
#include <boost/make_shared.hpp>
//#include "Poco/Data/SessionFactory.h"
#include <Poco/Data/Session.h>
//#include "Poco/Data/RecordSet.h"
//#include "Poco/Data/Column.h"
#include <Poco/Data/MySQL/MySQL.h>
#include <Poco/Data/MySQL/Connector.h>
//#include "Poco/Data/SessionPool.h"
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
//#include "Poco/Data/StatementImpl.h"
#include <Poco/Data/Statement.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Exception.h>
#include "Poco/Data/SessionPool.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <Poco/Data/RecordSet.h>
using Poco::UUID;
using Poco::UUIDGenerator;
using namespace Poco::Data;
using namespace std;

//global_arms[arms_type][arms_level];
boost::container::vector< vector<ArmsResponse> > global_arms;
boost::container::vector<vector<SkillData> > global_skill;
//global_build[build_type][build_level];
boost::container::vector< vector<BuildResponse> > global_build;

//city_limit[cur_city_level][build_type].max_level,max_num
vector<CityLimit> city_limit[11];

//use combat_union_limit[combat_union_level][arms_type] get max_level limit
vector<int>  combat_union_limit[6];

ConfigBuffer config_buffer;
///boost::container::vector<TowerList> tower_list;

VersionInforResponse game_version;
MysqlBase* mysql_main_thread;

vector<HeroUpdateConfig> hero_update_config;
vector<HeroBaseConfig> hero_base_config;

typedef boost::shared_ptr<SessionPool>    SessionPoolPointer;

void InitGlobalData()
{
	ReadGameVersion();
	ReadArmsData();
	ReadBuildData();
	ReadCityLimit();
	ReadCombatLimit();
	ReadHeroUpdateConfig();
	ReadHeroBaseConfig();
	ReadSkillConfig();
	/*
	string sql = "SELECT version_option,version FROM _global_version WHERE id=1";
	clock_t start_time=clock();
	for(int i = 0;i<10000;i++)
	{
		MysqlBase::Reader reader(mysql_main_thread,sql.c_str());
		//sess << "SELECT version_option,version FROM _global_version WHERE id=1",now;
	}
	clock_t end_time = clock();
	double duration=((double)(end_time-start_time))/((double)CLOCKS_PER_SEC);
	printf("%f\n",duration);s
	5倍以上的差距
	*/
}

void ReadGameVersion()
{
	string sql = "select version_option,version from _global_version";
	MysqlBase::Reader reader(mysql_main_thread,sql.c_str());
	while(char** row = reader.get_one_row())
	{
		if(strcmp(row[0],"arms_version") == 0)
		{
			game_version.arms_version = row[1];
			//printf("Read GameVersion end%s\n",row[1]);
		}
		else if(strcmp(row[0],"build_version") == 0)
		{
			game_version.build_version = row[1];
			//printf("Read GameVersion end%s\n",row[1]);
		}
		else if(strcmp(row[0],"client_version") == 0)
		{
			game_version.client_version = row[1];
			//printf("Read GameVersion end%s\n",row[1]);
		}
		else if(strcmp(row[0],"update_URL") == 0)
		{
			game_version.update_URL = row[1];
			//printf("Read GameVersion end%s\n",row[1]);
		}
		else if(strcmp(row[0],"update_URL") == 0)
		{
			game_version.skill_version = row[1];
			//printf("Read GameVersion end%s\n",row[1]);
		}
	}
	//printf("Read GameVersion end\n");
}

void ReadArmsData()
{
	global_arms.clear();
	string sql1 = "select max(type) from _global_arms";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	if(char** row = reader1.get_one_row())
	{
		for(int i  = 0;i<= atoi(row[0]);i++)
		{//first data is erro data;
			vector<ArmsResponse> t_list;
			global_arms.push_back(t_list);
			ArmsResponse res;
			t_list.push_back(res);
		}
	}
	else
	{
		printf("Read Tower Data is Error\n");
		return;
	}
	printf("size is %d\n",global_arms.size());
	string sql2 ="select "
			"type,"
			"level,"
			"hp,"
			"defense,"
			"attack,"
			"move_speed,"
			"fire_rate,"
			"need_wood,"
			"need_stone,"
			"need_iron,"
			"need_soil,"
			"population,"
			"build_time,"
			"available,"
			"magic_skill,"
			"name  "
				"from _global_arms order by id asc;";
	MysqlBase::Reader reader2(mysql_main_thread,sql2.c_str());
	//caculate read tower pack size();

	int size = 0;
	ArmsListResponse armslist;
	armslist.flags = 0;
	config_buffer.arms_list_buffer.clear();//if repeat read database;


	while(char** row = reader2.get_one_row())
	{
		ArmsResponse a;
		a.type = atoi(row[0]);
		a.level = atoi(row[1]);
		a.hp = atoi(row[2]);
		a.defense = atoi(row[3]);
		a.attack = atoi(row[4]);
		a.move_speed = atof(row[5]);
		a.fire_rate = atof(row[6]);
		a.need_wood = atoi(row[7]);
		a.need_stone = atoi(row[8]);
		a.need_iron = atoi(row[9]);
		a.need_soil = atoi(row[10]);
		a.population = atoi(row[11]);
		a.build_time = atoi(row[12]);
		a.available = atoi(row[13]);
		a.magic_skill = row[14];
		a.name = row[15];
		//printf("build time is %s \n",a.magic_skill.c_str());
		if(a.type < global_arms.size() && a.type > 0)
		{
			if(a.level >= global_arms[a.type].size())
			{
				for(int i = global_arms[a.type].size();i<= a.level;i++)
				{
					ArmsResponse newb = {};
					global_arms[a.type].push_back(newb);
				}
			}
			//printf("a.level is %d,global_arms[a.type].size() is %u \n",a.level,global_arms[a.type].size());
			global_arms[a.type][a.level] = a;
			//pack all tower in towerlist
			msgpack::sbuffer m_sbuf;
			msgpack::pack(m_sbuf,a);
			size += m_sbuf.size();
			if(size > 6500)//65535-4-1 = 65530,530 is reserve;
			{
				msgpack::sbuffer m_buff_list;
				msgpack::pack(m_buff_list,armslist);

				MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
				config_buffer.arms_list_buffer.push_back(buffer);
				armslist.arms_list.clear();
				size = m_sbuf.size();
				armslist.flags = 1;
			}
			armslist.arms_list.push_back(a);
		}
		else
		{
			printf("tower_type data is error \n");
		}
	}//end while
	if(armslist.arms_list.size()!=0)
	{
		if(armslist.flags == 0)
		{
			armslist.flags = 3;
		}
		else
		{
			armslist.flags = 2;
		}
		msgpack::sbuffer m_buff_list;
		msgpack::pack(m_buff_list,armslist);

		MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
		config_buffer.arms_list_buffer.push_back(buffer);
	}

	/*int sizecout = 0;
	for(int i = 0;i< global_arms.size();i++)
	{
		for(int j = 0;j< global_arms[i].size();j++)
		{
			printf("i %d  j %d level %d type%d     size is %d %s \n",i,j,global_arms[i][j].level,
					global_arms[i][j].type,++sizecout,global_arms[i][j].name.c_str());
		}
	}
*/
	printf("Read ArmsData end\n");
}

void ReadBuildData()
{
	//init global
	global_build.clear();
	string sql1 = "select max(type) from _global_build";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	if(char** row = reader1.get_one_row())
	{
		for(int i  = 0;i<= atoi(row[0]);i++)
		{//first data is erro data;
			vector<BuildResponse> b_list;
			global_build.push_back(b_list);
			BuildResponse res;
			b_list.push_back(res);
		}
	}
	else
	{
		printf("Read Build Data is Error\n");
		return;
	}
	printf("size is %d\n",global_build.size());

	//end init
	string sql2 ="select "
			"name,"
			"big_type,"
			"type,"
			"level,"
			"need_wood,"
			"need_stone,"
			"need_iron,"
			"need_soil,"
			"need_diamond,"
			"build_time,"
			"output,"
			"space,"
			"max_hp,"
			"attack,"
			"defense,"
			"fire_rate,"
			"attack_speed,"
			"attack_effect,"
			"build_describe "
				"from _global_build order by id asc;";
	MysqlBase::Reader reader2(mysql_main_thread,sql2.c_str());
	//caculate read tower pack size();

	int size = 0;
	BuildListResponse buildlist;
	buildlist.flags = 0;
	config_buffer.build_list_buffer.clear();//if repeat read database;

	static int count_i = 0;
	while(char** row = reader2.get_one_row())
	{
		BuildResponse b;
		b.name = row[0];
		b.big_type = atoi(row[1]);
		b.type = atoi(row[2]);
		b.level = atoi(row[3]);
		b.need_wood = atoi(row[4]);
		b.need_stone = atoi(row[5]);
		b.need_iron = atoi(row[6]);
		b.need_soil = atoi(row[7]);
		b.need_diamond = atoi(row[8]);
		b.build_time = atoi(row[9]);
		b.output = atoi(row[10]);
		b.space = atoi(row[11]);
		b.max_hp = atoi(row[12]);
		b.attack = atoi(row[13]);
		b.defense = atoi(row[14]);
		b.fire_rate = atof(row[15]);
		b.attack_speed = atof(row[16]);
		b.attack_effect = row[17]?row[17]:"0";
		b.build_describe = row[18]?row[18]:"No Describe";
		++count_i;
		//printf("build time is %d \n",b.space);
		if(b.type < global_build.size() && b.type > 0)
		{
			if(b.level >= global_build[b.type].size())
			{
				for(int i = global_build[b.type].size();i<= b.level;i++)
				{
					BuildResponse newb = {};
					global_build[b.type].push_back(newb);
				}
			}
			global_build[b.type][b.level] = b;
			//pack all tower in towerlist
			msgpack::sbuffer m_sbuf;
			msgpack::pack(m_sbuf,b);
			size += m_sbuf.size();
			if(size > 6500)//65535-4-1 = 65530,530 is reserve;
			{
				msgpack::sbuffer m_buff_list;
				msgpack::pack(m_buff_list,buildlist);

				MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
				config_buffer.build_list_buffer.push_back(buffer);
				buildlist.build_list.clear();
				buildlist.flags = 1;
				size = m_sbuf.size();
				printf("buidl has data========================1,\n");
			}
			buildlist.build_list.push_back(b);
			//printf("buidl%s========================%d,\n",b.name.c_str(),count_i++);
		}
		else
		{
			printf("tower_type data is error \n");
		}
	}//end while
	if(buildlist.build_list.size()!=0)
	{
		if(buildlist.flags == 0)
		{
			buildlist.flags = 3;
		}
		else
		{
			buildlist.flags = 2;
		}
		msgpack::sbuffer m_buff_list;
		msgpack::pack(m_buff_list,buildlist);

		MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
		config_buffer.build_list_buffer.push_back(buffer);
		printf("buidl has data========================2,\n");
	}
/*
	int sizecout = 0;
	for(int i = 0;i< global_build.size();i++)
	{
		for(int j = 0;j< global_build[i].size();j++)
		{
			printf("i %d  j %d level %d type%d     size is %d %s \n",i,j,global_build[i][j].level,
					global_build[i][j].type,++sizecout,global_build[i][j].name.c_str());
		}
	}

*/
	printf("Read build Data end%d\n",count_i);
}

void ReadCityLimit()
{
	string sql1 = "select max(build_type) from city_limit";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	if(char** row = reader1.get_one_row())
	{
		for(int j = 0;j< 11;j++)
		{
			for(int i  = 0;i<= atoi(row[0]);i++)
			{//first data is erro data;
				CityLimit l_list;
				city_limit[j].push_back(l_list);
			}
		}
	}
	else
	{
		printf("Read Build Data is Error\n");
		return;
	}

	string sql3 = "select * from city_limit";
	MysqlBase::Reader reader3(mysql_main_thread,sql3.c_str());

	CityLimitListResponse res;

	while(char** row = reader3.get_one_row())
	{
		int build_type = atoi(row[2]);
		if( build_type >= city_limit[1].size())
		{
			printf("Read city limit Data is Error\n");
			return;
		}
		for(int index = 1;index<11;index++)
		{
			city_limit[index][build_type].max_level = atoi(row[index+3]);
			city_limit[index][build_type].max_num = atoi(row[index+13]);
			city_limit[index][build_type].city_level = index;
			city_limit[index][build_type].build_type = build_type;
			res.city_limit.push_back(city_limit[index][build_type]);
			//printf("index = %d,build_type = %d==========%s,%s\n",index,build_type,row[index+3],row[index+13]);
		}
	}
	msgpack::sbuffer m_buff;
	msgpack::pack(m_buff,res);
	config_buffer.city_config.Write(m_buff.data(),m_buff.size());
	printf("all tiaoshu is %u\n",res.city_limit.size());

	//test uppack

}

void ReadSkillConfig()
{
	global_skill.clear();
	string sql1 = "select max(type) from __skill_config";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	if(char** row = reader1.get_one_row())
	{
		for(int i  = 0;i<= atoi(row[0]);i++)
		{//first data is erro data;
			vector<SkillData> t_list;
			global_skill.push_back(t_list);
			SkillData res;
			t_list.push_back(res);
		}
	}
	else
	{
		printf("Read Skill Config is Error\n");
		return;
	}
	printf("size is %d\n",global_skill.size());
	string sql2 ="select "
			"name,"
			"level,"
			"type,"
			"attack_type,"
			"attack_value,"
			"attack_range,"
			"keep_time,"
			"cold_time,"
			"need_mp,"
			"magic_union_level,"
			"need_soil "
				"from __skill_config order by ID asc;";
	MysqlBase::Reader reader2(mysql_main_thread,sql2.c_str());
	//caculate read tower pack size();

	int size = 0;
	SkillConfigResponse config;
	config.flags = 0;
	config_buffer.skill_config_buffer.clear();//if repeat read database;


	while(char** row = reader2.get_one_row())
	{
		SkillData a;
		a.name = row[0];
		a.level = atoi(row[1]);
		a.type = atoi(row[2]);
		a.attack_type = atoi(row[3]);
		a.value = atoi(row[4]);
		a.range = atof(row[5]);
		a.keep_time = atof(row[6]);
		a.cold_time = atof(row[7]);
		a.need_mp = atoi(row[8]);
		a.magic_union_level = atoi(row[9]);
		a.need_soil = atoi(row[10]);

		//printf("build time is %s \n",a.magic_skill.c_str());
		if(a.type < global_skill.size() && a.type > 0)
		{
			if(a.level >= global_skill[a.type].size())
			{
				for(int i = global_skill[a.type].size();i<= a.level;i++)
				{
					SkillData newb = {};
					global_skill[a.type].push_back(newb);
				}
			}
			//printf("a.level is %d,global_arms[a.type].size() is %u \n",a.level,global_arms[a.type].size());
			global_skill[a.type][a.level] = a;
			//pack all tower in towerlist
			msgpack::sbuffer m_sbuf;
			msgpack::pack(m_sbuf,a);
			size += m_sbuf.size();
			if(size > 6500)//65535-4-1 = 65530,530 is reserve;
			{
				msgpack::sbuffer m_buff_list;
				msgpack::pack(m_buff_list,config);

				MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
				config_buffer.skill_config_buffer.push_back(buffer);
				config.skill_config.clear();
				size = m_sbuf.size();
				config.flags = 1;
			}
			config.skill_config.push_back(a);
		}
		else
		{
			printf("tower_type data is error \n");
		}
	}//end while
	if(config.skill_config.size()!=0)
	{
		if(config.flags == 0)
		{
			config.flags = 3;
		}
		else
		{
			config.flags = 2;
		}
		msgpack::sbuffer m_buff_list;
		msgpack::pack(m_buff_list,config);

		MyBuffer buffer(m_buff_list.data(),m_buff_list.size());
		config_buffer.skill_config_buffer.push_back(buffer);
	}

	/*int sizecout = 0;
	for(int i = 0;i< global_arms.size();i++)
	{
		for(int j = 0;j< global_arms[i].size();j++)
		{
			printf("i %d  j %d level %d type%d     size is %d %s \n",i,j,global_arms[i][j].level,
					global_arms[i][j].type,++sizecout,global_arms[i][j].name.c_str());
		}
	}
*/
	printf("Read ArmsData end\n");
}
void ReadCombatLimit()
{
	string sql1 = "select max(arms_type) from combat_union_limit";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	if(char** row = reader1.get_one_row())
	{
		for(int j = 0;j< 6;j++)
		{
			for(int i  = 0;i<= atoi(row[0]);i++)
			{//first data is erro data;
				int temp;
				combat_union_limit[j].push_back(temp);
			}
		}
	}
	else
	{
		printf("***************************Read combat_union_limit is Error\n");
		return;
	}

	string sql3 = "select * from combat_union_limit";
	MysqlBase::Reader reader3(mysql_main_thread,sql3.c_str());

	while(char** row = reader3.get_one_row())
	{
		int arms_type = atoi(row[2]);
		if( arms_type >= combat_union_limit[1].size())
		{
			printf("Read combat_union_limit Data is Error\n");
			return;
		}
		for(int index = 1;index < 6;index++)
		{
			combat_union_limit[index][arms_type] = atoi(row[index+2]);
		}
	}
	printf("***************************Read combat_union_limit is end\n");

}

void ReadHeroUpdateConfig()
{
	string sql1 = "select type,diamond,hero_num,cool_time,start1,start2,start3,start4,start5 from   _global_recruit order by id asc";
	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	while(char** row = reader1.get_one_row())
	{
		HeroUpdateConfig h;
		h.type= atoi(row[0]);
		h.diamond= atoi(row[1]);
		h.hero_num= atoi(row[2]);
		h.cool_time= atoi(row[3]);
		h.start1= atoi(row[4]);
		h.start2= atoi(row[5]);
		h.start3= atoi(row[6]);
		h.start4= atoi(row[7]);
		h.start5= atoi(row[8]);
		hero_update_config.push_back(h);
	}
}
void ReadHeroBaseConfig()
{
	string sql1 = "select quality,all_min,all_max,s_min,s_min_grow,s_max,s_max_grow,s_min_limit_max,s_min_limit_grow,"
			"s_max_limit_min,s_max_limit_grow,goods_space,max_space,pre_skill,study_skill,special_name,"
			"base_hp,base_mp,base_attack,base_defence  "
			" from _global_hero order by id asc";

	MysqlBase::Reader reader1(mysql_main_thread,sql1.c_str());
	while(char** row = reader1.get_one_row())
	{
		HeroBaseConfig h;
		h.quality = atoi(row[0]);
		h.all_min= atoi(row[1]);
		h.all_max= atoi(row[2]);
		h.s_min= atoi(row[3]);
		h.s_min_grow= atoi(row[4]);
		h.s_max= atoi(row[5]);
		h.s_max_grow= atoi(row[6]);
		h.s_min_limit_max= atoi(row[7]);
		h.s_min_limit_grow= atoi(row[8]);
		h.s_max_limit_min= atoi(row[9]);
		h.s_max_limit_grow= atoi(row[10]);
		h.goods_space= atoi(row[11]);
		h.max_space= atoi(row[12]);
		h.pre_skill= atoi(row[13]);
		h.study_skill= atoi(row[14]);
		h.special_name= atoi(row[15]);
		h. base_hp =  atoi(row[16]);
		h.base_mp =  atoi(row[17]);
		h.base_attack = atoi(row[18]);
		h.base_defence = atoi(row[19]);
		hero_base_config.push_back(h);
	}
}
