/*
 * redis_chat_worker.h
 *
 *  Created on: Nov 18, 2013
 *      Author: zhongbing
 */

#ifndef REDIS_CHAT_WORKER_H_
#define REDIS_CHAT_WORKER_H_
#include <stdio.h>
#include "redis_base.h"
#include <string>
#include <msgpack_struct.h>
#include <iostream>
#include <sstream>

using namespace std;

class RedisChatWorker
{
private:

	friend class ChatWorker;

	friend class LoginWorker;

	bool get_random_name(string & name,bool is_man);

	void add_random_name(string name,bool is_man);

	bool exsit_user_name(string& name);

	bool set_user_name(unsigned int uuid,string& name);

	string find_user_name(unsigned int uuid);

	bool find_user_uuid(string& name,unsigned int&UUID);

	void get_build_map(map<string,MyBuildDataResponse>& build_map ,unsigned int&UUID);

	void set_build(MyBuildDataResponse& build,unsigned int&UUID);

	void get_player(MyAttruibuteResponse& myatt,unsigned int&UUID);

	void set_city_hall(string& city_hall_uuid,unsigned int&UUID);

	void set_combat_uuid(string& combat_union_uid,unsigned int UUID);

	void set_material(int wood,int stone,int iron,int soil,unsigned int&UUID);

	void set_player_base(int race,int record,int level,unsigned int&UUID);

	void set_build_timer(string build_sn,unsigned int&UUID);

	void del_build_timer(string build_sn,unsigned int&UUID);

	void get_build_timer_list(list<string>& building,unsigned int& UUID);

	void set_buidl_num(int buildingsNum,unsigned int& uuid);

	void set_my_build_num(int build_type,int build_num,unsigned int uuid);

	void get_my_build_num(map<int,int>& build_num,unsigned int uuid);

	void get_enemy_att(EnemyAttruibuteResponse& enemy_att,unsigned int&UUID);

	void set_backpack_num(int num,unsigned int& uuid);

	void set_equip(EquipResponse& res,unsigned int uuid);

	void get_equip_list(EquipListResponse& equip_list,EquipResponse my_backpack[64],bool back_status[64],
			HeroResponse* my_hero[6],map<string,vector<EquipResponse> >& hero_equip,
			HeroListResponse& myherolist,HeroResponse* radom_hero[3],EmailSaleListResponse& email_sale_list,unsigned int uuid);

	void del_equip(string equip_sn,unsigned int uuid);

	void set_market(MarketGoodsResponse& res);

	void get_market_list();

	void del_market(string equip_sn);

	void set_recurite(int recurite,int chance_num,unsigned int uuid);

	void get_recurite(int& recurite,int & chance_num,unsigned int uuid);

	void get_all_hero(HeroResponse* my_hero[6],
			map<string,vector<EquipResponse> >& hero_equip,HeroListResponse& myherolist,
			HeroResponse* radom_hero[3],unsigned int uuid);

	void set_hero(HeroResponse & res,unsigned int uuid);

	void del_hero(string hero_uuid,unsigned int uuid);

	void move_hero(HeroResponse* my_hero[6],int start_index,unsigned int uuid);

	void init_arms(unsigned int uuid);

	void set_email_sale(EmailSaleResponse& res,unsigned int uuid);

	void del_email_sale(EmailSaleResponse& res,unsigned int uuid);

	void set_arm_max_level(int arm_type,int arm_level,unsigned int uuid);

	void get_arm_max_level(int arm_type,int& arm_level,unsigned int uuid);

	void get_all_arm_max_level(MyArmpsLevelResponse& res,int max_arm_type,unsigned int uuid);

	RedisChatWorker();

	RedisChatWorker(const RedisChatWorker& r){

	}

public:
	virtual ~RedisChatWorker();

};




#endif /* REDIS_CHAT_WORKER_H_ */
