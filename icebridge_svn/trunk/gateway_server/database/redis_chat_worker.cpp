/*
 * redis_chat_worker.cpp
 *
 *  Created on: Nov 18, 2013
 *      Author: zhongbing
 */
#include "redis_chat_worker.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include "json/json.h"
#include <boost/foreach.hpp>
#include <vector>
#include <map>

extern RedisBase* redis_main_thread;

vector<MarketGoodsResponse> gloabal_market;
map<string,int>  market_index;

RedisChatWorker::RedisChatWorker()
{

}
RedisChatWorker::~RedisChatWorker()
{

}

bool check_string(const char* str_bigin,int length)
{
	bool num_flag = true;// if 1 all char is num
	if(length < 1)
	{
		return false;
	}
	for(int i = 0;i<length;i++)
	{
		if(str_bigin[i] == ' ')return false;
		if(str_bigin[i] < '0' || str_bigin[i] > '9')
		{
			num_flag = true;
		}
	}
	return num_flag;
}


bool RedisChatWorker::exsit_user_name(string& name)
{
	if(!check_string(name.c_str(),name.length()))
	{
		return false;//0 is not exsit
	}
	unsigned int uuidtemp;
	return find_user_uuid(name,uuidtemp);
}

bool RedisChatWorker::set_user_name(unsigned int uuid,string& name)
{
	char redis_sql[128] = {};
	if(!check_string(name.c_str(),name.length()))
	{
		return false;
	}
	unsigned int uuidtemp;

	if(find_user_uuid(name,uuidtemp))
	{
		return false;
	}
	else
	{
		sprintf(redis_sql,"HSET nickname %s %d",name.c_str(),uuid);
		RedisBase::SimpleWriter writer(redis_main_thread,redis_sql);
		sprintf(redis_sql,"HSET user#%u nickname %s",uuid,name.c_str());
		RedisBase::SimpleWriter writer2(redis_main_thread,redis_sql);
		return true;
	}
}
bool RedisChatWorker::get_random_name(string & name,bool is_man)
{
	if(is_man)
	{
		ostringstream  redis_sql;
		redis_sql<<"SPOP boy_name";
		RedisBase::SimplerReader reader(redis_main_thread,redis_sql.str().c_str());
		redisReply* result = reader.get_result();
		if(result->type == REDIS_REPLY_STRING)
		{
			name =  result->str;
			return true;
		}
	}
	else
	{
		ostringstream  redis_sql;
		redis_sql<<"SPOP girl_name";
		RedisBase::SimplerReader reader(redis_main_thread,redis_sql.str().c_str());
		redisReply* result = reader.get_result();
		if(result->type == REDIS_REPLY_STRING)
		{
			name =  result->str;
			return true;
		}
	}
	return false;
}

void RedisChatWorker::add_random_name(string name,bool is_man)
{
	ostringstream  redis_sql;
	if(is_man)
	{
		redis_sql<<"SADD boy_name";
		RedisBase::SimpleWriter writer(redis_main_thread,redis_sql.str().c_str());
	}
	else
	{
		redis_sql<<"SADD girl_name";
		RedisBase::SimpleWriter writer(redis_main_thread,redis_sql.str().c_str());
	}
}

string RedisChatWorker::find_user_name(unsigned int uuid)
{
	ostringstream  redis_sql;
	redis_sql<<"HGET user#"<<uuid <<" nickname ";
	RedisBase::SimplerReader reader(redis_main_thread,redis_sql.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_STRING)
	{
		return result->str;
	}
	else
	{
		return "Not Exsit Name";
	}
}

bool RedisChatWorker::find_user_uuid(string& name,unsigned int&UUID)
{
	if(!check_string(name.c_str(),name.length()))
	{
		return false;
	}
	string redis_sql = "HGET nickname " + name;
	RedisBase::SimplerReader reader(redis_main_thread,redis_sql.c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_STRING)
	{
		UUID = atoi(result->str);
		return true;
	}
	else
	{
		return false;
	}
}

void RedisChatWorker::get_player(MyAttruibuteResponse& myatt,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HMGET user#" << UUID
	  << " abt#wood abt#stone abt#ore abt#concrete  abt#point  level  city_hall_uuid "
	  <<" coord#x  coord#y  race  backpack_num recurite_type chance_num  combat_union_uid "
	  <<"  groupId   group#position ";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		myatt.wood = result->element[0]->str?atoi(result->element[0]->str):0;
		myatt.stone = result->element[1]->str?atoi(result->element[1]->str):0;
		myatt.iron = result->element[2]->str?atoi(result->element[2]->str):0;
		myatt.soil = result->element[3]->str?atoi(result->element[3]->str):0;
		myatt.record = result->element[4]->str?atoi(result->element[4]->str):0;
		myatt.level = result->element[5]->str?atoi(result->element[5]->str):0;
		myatt.city_hall_uuid = result->element[6]->str?result->element[6]->str:"no_uuid";
		myatt.worldmap_x = result->element[7]->str?atoi(result->element[7]->str): -1;
		myatt.worldmap_y = result->element[8]->str?atoi(result->element[8]->str): -1;
		myatt.race = result->element[9]->str?atoi(result->element[9]->str): 0;
		myatt.backpack_num = result->element[10]->str?atoi(result->element[10]->str): 12;
		myatt.recuite_type = result->element[11]->str?atoi(result->element[11]->str): 0;
		myatt.chance_num = result->element[12]->str?atoi(result->element[12]->str): 0;
		myatt.combat_uuid = result->element[13]->str?result->element[13]->str:"NoUUID";
		myatt.groupId =  result->element[14]->str?atoi(result->element[14]->str):-1;
		myatt.group_position =  result->element[15]->str?atoi(result->element[15]->str):-1;
	}
}

void RedisChatWorker::get_enemy_att(EnemyAttruibuteResponse& enemy_att,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HMGET user#" << UUID
	  << " abt#wood abt#stone abt#ore abt#concrete  abt#point  level  city_hall_uuid "
	  <<" coord#x  coord#y  race ";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		enemy_att.wood = result->element[0]->str?atoi(result->element[0]->str):0;
		enemy_att.stone = result->element[1]->str?atoi(result->element[1]->str):0;
		enemy_att.iron = result->element[2]->str?atoi(result->element[2]->str):0;
		enemy_att.soil = result->element[3]->str?atoi(result->element[3]->str):0;
		enemy_att.record = result->element[4]->str?atoi(result->element[4]->str):0;
		enemy_att.level = result->element[5]->str?atoi(result->element[5]->str):0;
		enemy_att.city_hall_uuid = result->element[6]->str?result->element[6]->str:"no uuid";
		enemy_att.worldmap_x = result->element[7]->str?atoi(result->element[7]->str): -1;
		enemy_att.worldmap_y = result->element[8]->str?atoi(result->element[8]->str): -1;
		enemy_att.race = result->element[9]->str?atoi(result->element[9]->str): 0;
	}
	get_build_map(enemy_att.my_build,UUID);
}
void RedisChatWorker::set_backpack_num(int num,unsigned int& uuid)
{
	ostringstream  os;
	os<<"HSET user#" << uuid
	  << " backpack_num "<< num;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::set_city_hall(string& city_hall_uuid,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HSET user#" << UUID
	  << "  city_hall_uuid "<< city_hall_uuid;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}


void RedisChatWorker::set_combat_uuid(string& combat_union_uid,unsigned int UUID)
{
	ostringstream  os;
	os<<"HSET user#" << UUID
	  << "  combat_union_uid "<< combat_union_uid;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::set_material(int wood,int stone,int iron,int soil,unsigned int&UUID)
{
	char redis_sql[1024] = {};
	sprintf(redis_sql,"HMSET user#%u abt#wood %d abt#stone %d abt#ore %d abt#concrete %d",UUID,wood,stone,iron,soil);
	RedisBase::SimpleWriter writer(redis_main_thread,redis_sql);
}


void RedisChatWorker::set_player_base(int race,int record,int level,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HMSET user#" << UUID
	  << "  abt#point "<< record
	  << "  level " << level
	  << " race "<< race;
//	/printf("redis sql is %s\n",);
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::set_buidl_num(int buildingsNum,unsigned int& uuid)
{
	ostringstream  os;
	os<<"HMSET user#" << uuid
	  << "  buildingsNum "<< buildingsNum;
//	/printf("redis sql is %s\n",);
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}


void RedisChatWorker::set_my_build_num(int build_type,int build_num,unsigned int uuid)
{
	ostringstream  os;
	os<<"HSET buildnum#" << uuid
	  << "  "<< build_type
	  << "  " <<build_num;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::get_my_build_num(map<int,int>& build_num,unsigned int uuid)
{
	ostringstream  os;
	os<<"HGETALL buildnum#" << uuid;
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		int build_type = 0;
		int build_number = 0;
		for(int i = 0;i< result->elements;)
		{
			build_type = result->element[i]->str?atof(result->element[i]->str):0;
			++i;
			build_number = result->element[i]->str?atof(result->element[i]->str):0;
			++i;
			build_num[build_type] = build_number;
		}
	}
}
void RedisChatWorker::get_build_map(map<string,MyBuildDataResponse>& build_map,unsigned int&UUID)
{
	build_map.clear();
	char redis_sql[64] = {};
	sprintf(redis_sql,"keys build#%u#* ",UUID);
	RedisBase::SimplerReader reader(redis_main_thread,redis_sql);
	redisReply* result = reader.get_result();
	long start_time = time(0);
	if(result->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i< result->elements;i++)
		{
			ostringstream redis_sql2;
			redis_sql2<<"HMGET "<<result->element[i]->str<<
					" build_sn  x y big_type status ctime time build_type build_level cur_hp build_skin cur_resource";
			RedisBase::SimplerReader reader2(redis_main_thread,redis_sql2.str().c_str());
			redisReply* result2 = reader2.get_result();
			if(result2->type == REDIS_REPLY_ARRAY)
			{
				if(result2->element[0]->str)
				{
					MyBuildDataResponse res;
					res.build_sn = result2->element[0]->str;
					res.x = result2->element[1]->str?atof(result2->element[1]->str):0;
					res.y = result2->element[2]->str?atof(result2->element[2]->str):0;
					res.big_type = result2->element[3]->str?atoi(result2->element[3]->str):0;
					res.status = result2->element[4]->str?atoi(result2->element[4]->str):0;
					res.ctime = result2->element[5]->str?atol(result2->element[5]->str):0;
					res.time = result2->element[6]->str?atol(result2->element[6]->str):0;
					res.build_type = result2->element[7]->str?atoi(result2->element[7]->str):0;
					res.build_level = result2->element[8]->str?atoi(result2->element[8]->str):0;
					res.cur_hp = result2->element[9]->str?atoi(result2->element[9]->str):0;
					res.build_skin = result2->element[10]->str?atoi(result2->element[10]->str):0;
					res.cur_resource = result2->element[11]->str?atoi(result2->element[11]->str):0;
					if(res.big_type == 2 && res.status == 0)
					{
						res.time = start_time - res.ctime;
					}
					build_map[res.build_sn] = res;
				}
			}
			else
			{
				continue ;
			}
		}
	}

}

void RedisChatWorker::set_build(MyBuildDataResponse& build,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HMSET build#" << UUID << "#" <<build.build_sn
	  <<"  build_sn  "<<build.build_sn
	  <<" x "<<build.x
	  <<" y "<<	build.y//3
	  <<" big_type "<<build.big_type
	  << " status " << build.status
	  <<" ctime " << build.ctime//6
	  <<" time " << build.time
	  << " build_type " << build.build_type
	  <<" build_level "<< build.build_level//9
	  << " cur_hp " << build.cur_hp
	  <<"  build_skin "<<build.build_skin
	  << " cur_resource "<<build.cur_resource;//12
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}

void RedisChatWorker::set_build_timer(string build_sn,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HSET timer#" << UUID << " " << build_sn << "  0 ";
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}

void RedisChatWorker::del_build_timer(string build_sn,unsigned int&UUID)
{
	ostringstream  os;
	os<<"HDEL timer#" << UUID << " " << build_sn;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::get_build_timer_list(list<string>& building,unsigned int& UUID)
{
	building.clear();
	ostringstream  os;
	os<<"HKEYS timer#" << UUID << " ";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i < result->elements;i++)
		{
			if(result->element[i]->str)
			{
				building.push_back(result->element[i]->str);
			}
		}
	}
	//printf("mybuding size is %u\n",building.size());
}


void RedisChatWorker::set_equip(EquipResponse& res,unsigned int uuid)
{
	if(res.where_uuid == "" || res.where_uuid == "  ")
	{
		res.where_uuid = "NoUUID";
	}
	ostringstream  os;
	os<<"HMSET equip#" << uuid
	  <<"#"<<res.equip_sn
	  <<" witch_pannle "<<res.witch_pannle
	  <<" position "<<res.position
	  <<" equip_sn "<<res.equip_sn
	  <<" part "<<res.part
	  <<" quality "<<res.quality
	  <<" level "<<res.level
	  <<" attr1_type "<<res.attr1_type
	  <<" attr1_value "<<res.attr1_value
	  <<" attr2_type "<<res.attr2_type
	  <<" attr2_value "<<res.attr2_value
	  <<" extral1_type "<<res.extral1_type
	  <<" extral1_value "<<res.extral1_value
	  <<" extral2_type "<<res.extral2_type
	  <<" extral2_value "<<res.extral2_value
	  <<" diamond_cost "<<res.diamond_cost
	  <<" where_uuid "<<res.where_uuid;
	printf("\n%s\n",os.str().c_str());
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}

void RedisChatWorker::get_equip_list(EquipListResponse& equip_list,EquipResponse my_backpack[64],bool back_status[64],
		HeroResponse* my_hero[6],map<string,vector<EquipResponse> >& hero_equip,HeroListResponse& myherolist,
		HeroResponse* radom_hero[3],EmailSaleListResponse& email_sale_list,unsigned int uuid)
{
	get_all_hero(my_hero,hero_equip,myherolist,radom_hero,uuid);
	equip_list.equip_list.clear();
	char redis_sql[64] = {};
	sprintf(redis_sql,"keys equip#%u#* ",uuid);
	printf("redis_sql = %s\n",redis_sql);
	RedisBase::SimplerReader reader(redis_main_thread,redis_sql);
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i< result->elements;i++)
		{
			ostringstream redis_sql2;
			redis_sql2<<"HMGET "<<result->element[i]->str<<
					" witch_pannle position equip_sn part quality "
					" level attr1_type attr1_value attr2_type attr2_value "
					" extral1_type extral1_value extral2_type extral2_value diamond_cost "
					" where_uuid ";
			RedisBase::SimplerReader reader2(redis_main_thread,redis_sql2.str().c_str());
			redisReply* result2 = reader2.get_result();
			if(result2->type == REDIS_REPLY_ARRAY)
			{
				if(result2->element[0]->str)
				{
					EquipResponse res;
					res.witch_pannle = result2->element[0]->str?atoi(result2->element[0]->str):0;
					res.position = result2->element[1]->str?atoi(result2->element[1]->str):0;
					res.equip_sn = result2->element[2]->str?result2->element[2]->str:"FaliedEquip";
					res.part = result2->element[3]->str?atoi(result2->element[3]->str):0;
					res.quality = result2->element[4]->str?atoi(result2->element[4]->str):0;
					res.level = result2->element[5]->str?atol(result2->element[5]->str):0;
					res.attr1_type = result2->element[6]->str?atoi(result2->element[6]->str):0;
					res.attr1_value = result2->element[7]->str?atoi(result2->element[7]->str):0;
					res.attr2_type = result2->element[8]->str?atoi(result2->element[8]->str):0;
					res.attr2_value = result2->element[9]->str?atoi(result2->element[9]->str):0;
					res.extral1_type = result2->element[10]->str?atoi(result2->element[10]->str):0;
					res.extral1_value = result2->element[11]->str?atoi(result2->element[11]->str):0;
					res.extral2_type = result2->element[12]->str?atoi(result2->element[12]->str):0;
					res.extral2_value = result2->element[13]->str?atoi(result2->element[13]->str):0;
					res.diamond_cost = result2->element[14]->str?atoi(result2->element[14]->str):0;
					res.where_uuid = result2->element[15]->str?result2->element[15]->str:"NoEquipUUID";
					if(res.witch_pannle == 2)
					{
						email_sale_list.email_sale_list[res.where_uuid].equip = res;
						continue;
					}
					if(res.witch_pannle == 0 && res.position >= 0 && res.position < 64)
					{
						back_status[res.position] = true;
						my_backpack[res.position] = res;
					}
					else if(res.witch_pannle == 1 && res.position >= 0 && res.position < 6)
					{
						if(hero_equip.find(res.where_uuid) != hero_equip.end())
						{
							hero_equip[res.where_uuid][res.position] = res;
						}
					}
					equip_list.equip_list.push_back(res);
				}
			}
			else
			{
				continue ;
			}
		}
	}

	//to read email_sale

	ostringstream  os111;
	os111<<"keys email_sale#"<<uuid<<"#*";

	RedisBase::SimplerReader reader111(redis_main_thread,os111.str().c_str());
	redisReply* result111 = reader111.get_result();
	if(result111->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i< result111->elements;i++)
		{
			ostringstream redis_sql2222;

			redis_sql2222<<"HMGET "<<result111->element[i]->str<<" email_uuid diamond time other_name ";
			RedisBase::SimplerReader reader2222(redis_main_thread,redis_sql2222.str().c_str());
			redisReply* result2222= reader2222.get_result();
			if(result2222->type == REDIS_REPLY_ARRAY)
			{
				string email_uuid = result2222->element[0]->str?result2222->element[0]->str:"NoUUID";
				email_sale_list.email_sale_list[email_uuid].email_uuid = email_uuid;
				email_sale_list.email_sale_list[email_uuid].diamond = result2222->element[1]->str?atoi(result2222->element[1]->str):0;
				email_sale_list.email_sale_list[email_uuid].time = result2222->element[2]->str?atol(result2222->element[2]->str):0;
				email_sale_list.email_sale_list[email_uuid].other_name = result2222->element[3]->str?result2222->element[3]->str:"NoName";
			}
		}
	}
}
void RedisChatWorker::del_equip(string equip_sn,unsigned int uuid)
{
	ostringstream  os;
	os<<"DEL equip#" << uuid
	<<"#"<<equip_sn;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}

void RedisChatWorker::get_market_list()
{
	//equip_list.equip_list.clear();
	ostringstream  os;
	os<<"keys market#* ";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	long start_time = time(0);
	if(result->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i< result->elements;i++)
		{
			ostringstream redis_sql2;
			redis_sql2<<"HMGET "<<result->element[i]->str<<
					" player remain_time diamond_cost goods_type position "
					" equip_sn part quality level attr1_type "
					" attr1_value attr2_type attr2_value extral1_type extral1_value "
					" extral2_type extral2_value ";
			RedisBase::SimplerReader reader2(redis_main_thread,redis_sql2.str().c_str());
			redisReply* result2 = reader2.get_result();
			if(result2->type == REDIS_REPLY_ARRAY)
			{
				if(result2->element[0]->str)
				{
					MarketGoodsResponse res;
					res.player = result2->element[0]->str?result2->element[0]->str:"NoName";//1
					long time = result2->element[1]->str?atol(result2->element[1]->str):0;//2
					res.remain_time = 86400 - (start_time - time);
					if(res.remain_time <= 0)
					{
						res.remain_time = 0;
					}
					res.diamond_cost = result2->element[2]->str?atoi(result2->element[2]->str):1;//3
					res.goods_type = result2->element[3]->str?atoi(result2->element[3]->str):0;//4
					res.position = result2->element[4]->str?atoi(result2->element[4]->str):0;//5
					if(!result2->element[5]->str)continue;
					res.equip_sn = result2->element[5]->str;//6
					res.part = result2->element[6]->str?atoi(result2->element[6]->str):1;//7
					res.quality = result2->element[7]->str?atoi(result2->element[7]->str):1;//8
					res.level = result2->element[8]->str?atoi(result2->element[8]->str):1;//9
					res.attr1_type = result2->element[9]->str?atoi(result2->element[9]->str):0;//10
					res.attr1_value = result2->element[10]->str?atoi(result2->element[10]->str):0;//11
					res.attr2_type = result2->element[11]->str?atoi(result2->element[11]->str):0;//12
					res.attr2_value = result2->element[12]->str?atoi(result2->element[12]->str):0;//13
					res.extral1_type = result2->element[13]->str?atoi(result2->element[13]->str):0;//14
					res.extral1_value = result2->element[14]->str?atoi(result2->element[14]->str):0;//15
					res.extral2_type = result2->element[15]->str?atoi(result2->element[15]->str):0;//16
					res.extral2_value = result2->element[16]->str?atoi(result2->element[16]->str):0;//17
					gloabal_market.push_back(res);
					market_index[res.equip_sn] = gloabal_market.size() - 1;
				}
			}
			else
			{
				continue ;
			}
		}
	}
}

void RedisChatWorker::set_market(MarketGoodsResponse& res)
{
	ostringstream  os;
	os<<"HMSET market#"<< res.equip_sn
	  <<" player "<<res.player//1
	  <<" remain_time "<<res.remain_time//2
	  <<" diamond_cost "<<res.diamond_cost//3
	  <<" goods_type "<<res.goods_type//4
	  <<" position "<<res.position//5
	  <<" equip_sn "<<res.equip_sn//6
	  <<" part "<<res.part//7
	  <<" quality "<<res.quality//8
	  <<" level "<<res.level//9
	  <<" attr1_type "<<res.attr1_type//10
	  <<" attr1_value "<<res.attr1_value//11
	  <<" attr2_type "<<res.attr2_type//12
	  <<" attr2_value "<<res.attr2_value//13
	  <<" extral1_type "<<res.extral1_type//14
	  <<" extral1_value "<<res.extral1_value//15
	  <<" extral2_type "<<res.extral2_type//16
	  <<" extral2_value "<<res.extral2_value;//17
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
	res.remain_time = 86400;
	gloabal_market.push_back(res);//is important
	market_index[res.equip_sn] = gloabal_market.size() - 1;
}
void RedisChatWorker::del_market(string equip_sn)
{
	map<string,int>::iterator it = market_index.find(equip_sn);
	if(it != market_index.end())
	{
		if(market_index[equip_sn] >=0 && market_index[equip_sn] < gloabal_market.size())
		{
			if(gloabal_market.size() == 1)
			{
				gloabal_market.clear();
				market_index.clear();
			}
			else
			{
				gloabal_market[market_index[equip_sn]] = gloabal_market[gloabal_market.size() - 1];
				string new_equip_sn = gloabal_market[market_index[equip_sn]].equip_sn;
				market_index[new_equip_sn] = market_index[equip_sn];
				market_index.erase(it);
				gloabal_market.pop_back();
			}
			ostringstream  os;
			os<<"DEL market#" << equip_sn;
	 		RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
		}
	}

}
void RedisChatWorker::set_recurite(int recurite,int chance_num,unsigned int uuid)
{
	ostringstream  os;
	os<<"HMSET user#" << uuid
	  << "  recurite_type " << recurite
	  << "  chance_num "<< chance_num;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}

void RedisChatWorker::get_all_hero(HeroResponse* my_hero[6],map<string,vector<EquipResponse> >& hero_equip,
		HeroListResponse& myherolist,HeroResponse* radom_hero[3],unsigned int uuid)
{
	//equip_list.equip_list.clear();
	ostringstream  os;
	os<<"keys hero#"<<uuid<<"#*";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result = reader.get_result();
	if(result->type == REDIS_REPLY_ARRAY)
	{
		for(int i = 0;i< result->elements;i++)
		{
			ostringstream redis_sql2;
			redis_sql2<<"HMGET "<<result->element[i]->str<<
					" hero_sn name hero_uuid quality level "
					" leader strategy economy strength  magic_power  "
					" leader_grow strategy_grow economy_grow strength_grow magic_power_grow "
					" cur_hp max_hp cur_mp max_mp attack "
					" defence pre_skill study_skill cur_goods_space max_goods_space "
					" exp   type  where  base_leader base_strategy "
					" base_economy   base_strength base_magic_power   base_hp base_mp"
					" base_attack   base_defence ";

			RedisBase::SimplerReader reader2(redis_main_thread,redis_sql2.str().c_str());
			redisReply* result2 = reader2.get_result();
			if(result2->type == REDIS_REPLY_ARRAY)
			{
				if(result2->element[0]->str)
				{
					HeroResponse* res = new HeroResponse();
					res->hero_sn = result2->element[0]->str?atoi(result2->element[0]->str):0;//1
					if(res->hero_sn > 6) res->hero_sn = 1;
					res->name = result2->element[1]->str?result2->element[1]->str:"NoName";//2
					res->hero_uuid = result2->element[2]->str?result2->element[2]->str:"NoUUID";//3
					res->quality = result2->element[3]->str?atoi(result2->element[3]->str):1;//4
					res->level = result2->element[4]->str?atoi(result2->element[4]->str):1;//5

					res->leader = result2->element[5]->str?atof(result2->element[5]->str):5;//6
					res->strategy = result2->element[6]->str?atof(result2->element[6]->str):5;//7
					res->economy = result2->element[7]->str?atof(result2->element[7]->str):5;//8
					res->strength = result2->element[8]->str?atof(result2->element[8]->str):0;//9
					res->magic_power = result2->element[9]->str?atof(result2->element[9]->str):0;//10
					res->leader_grow = result2->element[10]->str?atof(result2->element[10]->str):0;//11
					res->strategy_grow = result2->element[11]->str?atof(result2->element[11]->str):0;//12
					res->economy_grow = result2->element[12]->str?atof(result2->element[12]->str):0;//13
					res->strength_grow = result2->element[13]->str?atof(result2->element[13]->str):0;//14
					res->magic_power_grow = result2->element[14]->str?atof(result2->element[14]->str):0;//15

					res->cur_hp = result2->element[15]->str?atoi(result2->element[15]->str):1;//16
					res->max_hp = result2->element[16]->str?atoi(result2->element[16]->str):1;//17
					res->cur_mp = result2->element[17]->str?atoi(result2->element[17]->str):1;//18
					res->max_mp = result2->element[18]->str?atol(result2->element[18]->str):1;//19
					res->attack = result2->element[19]->str?atoi(result2->element[19]->str):1;//20
					res->defence = result2->element[20]->str?atoi(result2->element[20]->str):0;//21
					res->pre_skill = result2->element[21]->str?atoi(result2->element[21]->str):0;//22
					res->study_skill = result2->element[22]->str?atoi(result2->element[22]->str):0;//23
					res->cur_goods_space = result2->element[23]->str?atoi(result2->element[23]->str):1;//24
					res->max_goods_space = result2->element[24]->str?atoi(result2->element[24]->str):1;//25
					res->exp = result2->element[25]->str?atoi(result2->element[25]->str):0;//26
					res->type = result2->element[26]->str?atoi(result2->element[26]->str):0;//27
					res->where = result2->element[27]->str?atoi(result2->element[27]->str):0;//28
					res->base_att.base_leader = result2->element[28]->str?atoi(result2->element[28]->str):0;//28
					res->base_att.base_strategy = result2->element[29]->str?atoi(result2->element[29]->str):0;//28
					res->base_att.base_economy = result2->element[30]->str?atoi(result2->element[30]->str):0;//28
					res->base_att.base_strength = result2->element[31]->str?atoi(result2->element[31]->str):0;//28
					res->base_att.base_magic_power = result2->element[32]->str?atoi(result2->element[32]->str):0;//28
					res->base_att.base_hp = result2->element[33]->str?atoi(result2->element[33]->str):0;//28
					res->base_att.base_mp = result2->element[34]->str?atoi(result2->element[34]->str):0;//28
					res->base_att.base_attack = result2->element[35]->str?atoi(result2->element[35]->str):0;//28
					res->base_att.base_defence = result2->element[36]->str?atoi(result2->element[36]->str):0;//28

					myherolist.hero_list.push_back(*res);
					if(res->where <= 0)
					{
						my_hero[res->hero_sn - 1] = res;
						vector<EquipResponse> equip_list;
						hero_equip[res->hero_uuid] =  equip_list;
						EquipResponse temp;
						temp.witch_pannle = -1;
						for(int i = 0;i< 6;i++)
						{
							hero_equip[res->hero_uuid].push_back(temp);
						}
					}
					else if( res->where == 1 && res->hero_sn > 0 &&  res->hero_sn < 4)//random store
					{
						radom_hero[res->hero_sn -1] = res;
					}
					else
					{
						delete res;
					}
				}
			}
			else
			{
				continue ;
			}
		}
	}
}

void RedisChatWorker::set_hero(HeroResponse & res,unsigned int uuid)
{
	ostringstream  os;
	os<<"HMSET hero#" << uuid
	  << "#"<<res.hero_uuid
	  << "  hero_sn " << res.hero_sn//1
	  << "  name " << res.name//2
	  << "  hero_uuid " << res.hero_uuid//3
	  << "  quality " << res.quality//4
	  << "  level " << res.level//5
	  << "  leader " << res.leader//6
	  << "  strategy " << res.strategy//7
	  << "  economy " << res.economy//8
	  << "  strength " << res.strength//9
	  << "  magic_power " << res.magic_power//10
	  << "  leader_grow " << res.leader_grow//11
	  << "  strategy_grow " << res.strategy_grow//12
	  << "  economy_grow " << res.economy_grow//13
	  << "  strength_grow " << res.strength_grow//14
	  << "  magic_power_grow " << res.magic_power_grow//15
	  << "  cur_hp " << res.cur_hp//16
	  << "  max_hp " << res.max_hp//17
	  << "  cur_mp " << res.cur_mp//18
	  << "  max_mp " << res.max_mp//19
	  << "  attack " << res.attack//20
	  << "  defence " << res.defence//21
	  << "  pre_skill " << res.pre_skill//22
	  << "  study_skill " << res.study_skill//23
	  << "  cur_goods_space " << res.cur_goods_space//24
	  << "  max_goods_space " << res.max_goods_space//25
	  << "  exp " << res.exp
	  << "  type  "<< res.type
	  << "  where "<< res.where
	  << "  base_leader  "<< res.base_att.base_leader
	  << "  base_strategy "<< res.base_att.base_strategy
	  << "  base_economy  "<< res.base_att.base_economy
	  << "  base_strength "<< res.base_att.base_strength
	  << "  base_magic_power  "<< res.base_att.base_magic_power
	  << "  base_hp "<< res.base_att.base_hp
	  << "  base_mp  "<< res.base_att.base_mp
	  << "  base_attack  "<< res.base_att.base_attack
	  << "  base_defence  "<< res.base_att.base_defence;

	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}


void RedisChatWorker::del_hero(string hero_uuid,unsigned int uuid)
{
	ostringstream  os;
	os<<"DEL hero#" << uuid
	  << "#"<<hero_uuid;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::move_hero(HeroResponse* my_hero[6],int start_index,unsigned int uuid)
{
	for(int i = start_index;i< 6;i++)
	{
		if(my_hero[i] == NULL)
		{
			return ;
		}
		ostringstream  os;
		os<<"HSET hero#" << uuid
		  << "#"<<my_hero[i]->hero_uuid
		  <<" hero_sn "<<my_hero[i]->hero_sn;
		RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
	}
}
void RedisChatWorker::init_arms(unsigned int uuid)
{
	/*ostringstream  os;
	os<<"HMSET user#" << uuid
	  << " army#1  "<< 0
	  << " army#1#max "<< 1
	  << " army#2  "<< 0
	  << " army#2#max "<< 1
	  << " army#3  "<< 0
	  << " army#3#max "<< 1;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());*/
}

void RedisChatWorker::set_email_sale(EmailSaleResponse& res,unsigned int uuid)
{
	if(res.other_name == "" || res.other_name == " ")
	{
		res.other_name = "NoName";
	}
	ostringstream  os;
	os<<"HMSET email_sale#"<<uuid
	  <<"#"<<res.email_uuid
	  <<" diamond "<<res.diamond
	  << " email_uuid  "<< res.email_uuid
	  << " time "<< res.time
	  << " other_name "<< res.other_name;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
	if(res.diamond <= 0)
	{
		set_equip(res.equip,uuid);
	}
}

void RedisChatWorker::del_email_sale(EmailSaleResponse& res,unsigned int uuid)
{
	//del_equip(res.equip.equip_sn,uuid);
	ostringstream  os;
	os<<"DEL email_sale#" << uuid
	<<"#"<<res.email_uuid;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}
void RedisChatWorker::set_arm_max_level(int arm_type,int arm_level,unsigned int uuid)
{
	ostringstream  os;
	os<<"HSET user#" << uuid
	  << " army#"<<arm_type<<"#max  "<< arm_level;
	RedisBase::SimpleWriter writer(redis_main_thread,os.str().c_str());
}


void RedisChatWorker::get_arm_max_level(int arm_type,int& arm_level,unsigned int uuid)
{
	ostringstream  os;
	os<<"HGET user#" << uuid
	  << " army#"<<arm_type<<"#max";
	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result  = reader.get_result();
	if(result->type != REDIS_REPLY_ERROR)
	{
		arm_level = result->str?atoi(result->str):0;
	}
}

void RedisChatWorker::get_all_arm_max_level(MyArmpsLevelResponse& res,int max_arm_type,unsigned int uuid)
{
	res.my_arm_level.clear();


	ostringstream  os;
	os<<"HMGET user#" << uuid;
	for(int i = 1;i<= max_arm_type;i++)
	{
		 os<< " army#"<<i<<"#max";
	}

	RedisBase::SimplerReader reader(redis_main_thread,os.str().c_str());
	redisReply* result  = reader.get_result();
	if(result->type ==  REDIS_REPLY_ARRAY)
	{
		int level = 0;
		for(int i = 0;i< max_arm_type;i++)
		{
			level = result->element[i]->str?atoi(result->element[i]->str):0;
			if(level != 0)
			{
				res.my_arm_level[i + 1] = level;
			}
		}
	}
}
