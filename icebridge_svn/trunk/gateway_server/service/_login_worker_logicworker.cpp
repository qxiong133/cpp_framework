#include "_login_worker.h"
#include <boost/container/map.hpp>
#include <boost/foreach.hpp>
#include <service/_global_macro.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>
#include <timer.h>
#include <boost/bind.hpp>

extern boost::container::vector< vector<ArmsResponse> > global_arms;
extern boost::container::vector< vector<BuildResponse> > global_build;
extern vector<CityLimit>  city_limit[11];
extern TCPIOThreadManager manager;
extern vector<MarketGoodsResponse> gloabal_market;
extern map<string,int>  market_index;

typedef boost::shared_ptr<TCPSessionHandler>  SessionHandlerPointer;
extern boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;
extern vector<HeroUpdateConfig> hero_update_config;
extern vector<HeroBaseConfig> hero_base_config;

void LoginWorker::build_timer_handle(string build_sn)
{
	printf("enter LoginWorker::build_timer_handle(string build_sn)\n");
	if(my_attribute.my_build.find(build_sn) != my_attribute.my_build.end())
	{
		MyBuildDataResponse & b = my_attribute.my_build[build_sn];
		b.status = 0;
		b.time = 0;
		b.build_level += 1;
		b.cur_hp = global_build[b.build_type][b.build_level].max_hp;
		if(b.big_type == 2)
		{
			b.status = 0;
		    b.ctime = time(0);
		    b.time = 0;
		    b.cur_resource = 0;
		}
		if(building_timer.find(build_sn) != building_timer.end())
		{
			delete building_timer[build_sn];
			building_timer[build_sn] = NULL;
			building_timer.erase(build_sn);
			SendMessage(b,MyBuildDataResponse_ID);
		}
		building.remove(build_sn);
		redis.set_build(b,player_UUID);
	}
}


void LoginWorker::en_timer(string build_sn)
{
	static boost::asio::io_service& io= manager.io_service();
	building_timer[build_sn] = new DeadlineTimer(io);
	building_timer[build_sn]->Wait(boost::bind(&LoginWorker::build_timer_handle,this,build_sn),my_attribute.my_build[build_sn].time*1000);
	printf("enter LoginWorker::en_timer(string build_sn)\n");
}
void LoginWorker::BuildStatusListRequestService(object *request)
{
	BuildStatusListRequest req;
	MsgpackConvert(&req,*request);

	redis.get_build_map(my_attribute.my_build,player_UUID);

	BOOST_FOREACH(BuildStatus & value,req.build_status_list)
	{
		if(my_attribute.my_build.find(value.build_sn) != my_attribute.my_build.end())
		{
			 my_attribute.my_build[value.build_sn].x = value.x;
			 my_attribute.my_build[value.build_sn].y = value.y;
			 redis.set_build( my_attribute.my_build[value.build_sn],player_UUID);
		}
	}
}

void LoginWorker::BuildCreateRequestService(object *request)
{
	BuildCreateRequest req;
	MsgpackConvert(&req,*request);
	if(req.build_type > 0 && req.build_type < global_build.size())
	{// is  gernerate build
		int count = 0;
		if(my_attribute.build_num.find(req.build_type) != my_attribute.build_num.end())
		{
			count = my_attribute.build_num[req.build_type];
		}
		BuildResponse& b = global_build[req.build_type][0];
		//read city hall

		if(my_attribute.soil >= b.need_soil &&  my_attribute.stone >= b.need_stone &&
				my_attribute.iron >= b.need_iron && my_attribute.wood >= b.need_wood &&
				my_attribute.diamond >= b.need_diamond &&
				my_attribute.my_build.find(my_attribute.city_hall_uuid) != my_attribute.my_build.end())
		{
			if(count < city_limit[my_attribute.my_build[my_attribute.city_hall_uuid].build_level][req.build_type].max_num)
			{
				my_attribute.soil -= b.need_soil;
				my_attribute.stone -= b.need_stone;
				my_attribute.iron -= b.need_iron;
				my_attribute.wood -= b.need_wood;
				my_attribute.diamond -= b.need_diamond;
				mysql.update_diamond(my_attribute.diamond,player_UUID);
				redis.set_material(my_attribute.wood,my_attribute.stone,my_attribute.iron,my_attribute.soil,player_UUID);
				//new build
			    Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
			    Poco::UUID uid1 = gen.createRandom();
				MyBuildDataResponse newbuild;
				newbuild.build_sn = uid1.toString();
				newbuild.big_type = b.big_type;
				newbuild.status = 1;
				newbuild.ctime = time(0);
				newbuild.time = b.build_time;
				newbuild.build_type = req.build_type;
				newbuild.build_level = 0;
				newbuild.cur_hp = b.max_hp;
				newbuild.build_skin = 0;
				newbuild.cur_resource = 0;
				newbuild.x = req.x;
				newbuild.y = req.y;
				my_attribute.my_build[newbuild.build_sn] = newbuild;
				redis.set_build_timer(newbuild.build_sn,player_UUID);
				building.push_back(newbuild.build_sn);
				if(newbuild.time <= 1100)
				{
					en_timer(newbuild.build_sn);
				}
				redis.set_build(newbuild,player_UUID);
				redis.set_buidl_num(my_attribute.my_build.size() + 1,player_UUID);
				++count;
				my_attribute.build_num[req.build_type] = count;
				redis.set_my_build_num(req.build_type,count,player_UUID);
				SendMessage(newbuild,MyBuildDataResponse_ID);
				AssetResponse ares;
				ares.wood = my_attribute.wood;
				ares.stone = my_attribute.stone;
				ares.iron = my_attribute.iron;
				ares.soil = my_attribute.soil;
				ares.diamond = my_attribute.diamond;
				SendMessage(ares,AssetResponse_ID);
			}
			else
			{
				//printf("+++++++++++++++++++++%d,%d\n",city_limit[my_attribute.my_build[my_attribute.city_hall_uuid].build_level + 1][req.build_type].max_num,
					//	city_limit[my_attribute.my_build[my_attribute.city_hall_uuid].build_level][req.build_type].max_level);
			}
			return ;
		}
		//SimpleResponse Send
	}
	SimpleResponse sres;
	sres.msgid = BuildCreateRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);

}

void LoginWorker::BuildUpgradeRequestService(object *request)
{
	BuildUpgradeRequest req;
	MsgpackConvert(&req,*request);
	if(my_attribute.my_build.find(my_attribute.city_hall_uuid) != my_attribute.my_build.end() &&
			my_attribute.my_build.find(req.build_sn) != my_attribute.my_build.end()
	)
	{
		MyBuildDataResponse& b = my_attribute.my_build[req.build_sn];
		if(my_attribute.my_build[req.build_sn].build_level <
				city_limit[my_attribute.my_build[my_attribute.city_hall_uuid].build_level][b.build_type].max_level
				&& my_attribute.my_build[req.build_sn].status != 1)
		{
			BuildResponse& res = global_build[b.build_type][b.build_level];
			if(my_attribute.soil >= res.need_soil &&  my_attribute.stone >= res.need_stone &&
					my_attribute.iron >= res.need_iron && my_attribute.wood >= res.need_wood)
			{
				my_attribute.soil -= res.need_soil;
				my_attribute.stone -= res.need_stone;
				my_attribute.iron -= res.need_iron;
				my_attribute.wood -= res.need_wood;
				redis.set_material(my_attribute.wood,my_attribute.stone,my_attribute.iron,my_attribute.soil,player_UUID);
				b.status = 1;
				b.ctime = time(0);
				b.time =  res.build_time;
				my_attribute.my_build[b.build_sn] = b;
				redis.set_build_timer(b.build_sn,player_UUID);
				building.push_back(b.build_sn);
				if(b.time <= 1100)
				{
					en_timer(b.build_sn);
				}
				redis.set_build(b,player_UUID);
				SendMessage(b,MyBuildDataResponse_ID);
				AssetResponse ares;
				ares.wood = my_attribute.wood;
				ares.stone = my_attribute.stone;
				ares.iron = my_attribute.iron;
				ares.soil = my_attribute.soil;
				ares.diamond = my_attribute.diamond;
				SendMessage(ares,AssetResponse_ID);
				return ;
			}
		}
	}
	SimpleResponse sres;
	sres.msgid = BuildUpgradeRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);

}
void LoginWorker::GetMyAssetRequestService(object *request)
{
// find user uuid;
	GetMyAssetRequest req;
	MsgpackConvert(&req,*request);
	if(my_attribute.my_build.find(req.build_sn) != my_attribute.my_build.end())
	{
		MyBuildDataResponse & b = my_attribute.my_build[req.build_sn];
		if(b.big_type == 2 && b.status == 0)
		{
			long startime = time(0);
			int cur_time = startime - b.ctime;
			b.ctime = startime;
			redis.set_build(b,player_UUID);
			int cur_output = (float)global_build[b.build_type][b.build_level].output * (float)cur_time / 3600.0;
			if(cur_output > global_build[b.build_type][b.build_level].space)
			{
				cur_output = global_build[b.build_type][b.build_level].space;
			}
			GetMyAssetResponse res = {};
			res.msg_id = GetMyAssetRequest_ID;
			switch(b.build_type)
			{
			case 8://wood
			{
				res.add_wood = cur_output;
				my_attribute.wood += cur_output;
				break;
			}
			case 9://stone
			{
				my_attribute.stone += cur_output;
				res.add_stone = cur_output;
				break;
			}
			case 10://iron
			{
				my_attribute.iron += cur_output;
				res.add_iron = cur_output;
				break;
			}
			case 11://soil
			{
				my_attribute.soil += cur_output;
				res.add_soil = cur_output;
				break;
			}
			default:break;
			}
//			SendMessage(b,MyBuildDataResponse_ID);
			SendMessage(res,GetMyAssetResponse_ID);
			redis.set_material(my_attribute.wood,my_attribute.stone,my_attribute.iron,my_attribute.soil,player_UUID);
			AssetResponse ares;
			ares.wood = my_attribute.wood;
			ares.stone = my_attribute.stone;
			ares.iron = my_attribute.iron;
			ares.soil = my_attribute.soil;
			ares.diamond = my_attribute.diamond;
			SendMessage(ares,AssetResponse_ID);
		}
	}
	SimpleResponse sres;
	sres.msgid = GetMyAssetRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
}

int LoginWorker::find_hero_equip_sn(HeroResponse& hero)
{
	vector<EquipResponse>& e = hero_equip[hero.hero_uuid];
	for(int i = 0;i<e.size();i++)
	{
		if(e[i].witch_pannle != 1)return i;
	}
	return -1;//full not enough space
}

void LoginWorker::HeroRecaculate(HeroResponse& hero)
{
	int equip_leader = 0;
	int equip_strategy = 0;
	int equip_economy = 0;
	int equip_strength = 0;
	int equip_magic_power = 0;
	int equip_hp = 0;
	int equip_mp = 0;
	if(hero_equip.find(hero.hero_uuid) != hero_equip.end())
	{
		vector<EquipResponse>& h = hero_equip[hero.hero_uuid];
		for(int i = 0;i < h.size();i++)
		{
			if(h[i].witch_pannle != 1) continue;
			switch(h[i].attr1_type)
			{
			case 1:equip_leader += h[i].attr1_value;break;
			case 2:equip_strategy += h[i].attr1_value;break;
			case 3:equip_economy += h[i].attr1_value;break;
			case 4:equip_strength += h[i].attr1_value;break;
			case 5:equip_magic_power += h[i].attr1_value;break;
			case 6:equip_hp += h[i].attr1_value;break;
			case 7:equip_mp += h[i].attr1_value;break;
			default:break;
			}
			switch(h[i].attr2_type)
			{
			case 1:equip_leader += h[i].attr2_value;break;
			case 2:equip_strategy += h[i].attr2_value;break;
			case 3:equip_economy += h[i].attr2_value;break;
			case 4:equip_strength += h[i].attr2_value;break;
			case 5:equip_magic_power += h[i].attr2_value;break;
			case 6:equip_hp += h[i].attr2_value;break;
			case 7:equip_mp += h[i].attr2_value;break;
			default:break;
			}
			switch(h[i].extral1_type)
			{
			case 1:equip_leader += h[i].extral1_value;break;
			case 2:equip_strategy += h[i].extral1_value;break;
			case 3:equip_economy += h[i].extral1_value;break;
			case 4:equip_strength += h[i].extral1_value;break;
			case 5:equip_magic_power += h[i].extral1_value;break;
			case 6:equip_hp += h[i].extral1_value;break;
			case 7:equip_mp += h[i].extral1_value;break;
			default:break;
			}
			switch(h[i].extral2_type)
			{
			case 1:equip_leader += h[i].extral2_value;break;
			case 2:equip_strategy += h[i].extral2_value;break;
			case 3:equip_economy += h[i].extral2_value;break;
			case 4:equip_strength += h[i].extral2_value;break;
			case 5:equip_magic_power += h[i].extral2_value;break;
			case 6:equip_hp += h[i].extral2_value;break;
			case 7:equip_mp += h[i].extral2_value;break;
			default:break;
			}
		}
	}
	hero.leader = hero.base_att.base_leader +  equip_leader;
	hero.strategy = hero.base_att.base_strategy +  equip_strategy;
	hero.economy = hero.base_att.base_economy +  equip_economy;
	hero.strength = hero.base_att.base_strength +  equip_strength;
	hero.magic_power = hero.base_att.base_magic_power +  equip_magic_power;
	hero.max_hp = hero.strength  * 5 +  equip_hp;
	hero.max_mp = hero.magic_power  +  equip_mp;
	hero.attack = hero.base_att.base_attack + hero.strength;
	hero.defence =  hero.base_att.base_defence + hero.strength * 0.02;
}

void LoginWorker::DressEquipRequestService(object *request)
{
	DressEquipRequest req;
	MsgpackConvert(&req,*request);
	SimpleResponse sres;
	sres.msgid = DressEquipRequest_ID;
	sres.sn = 0;
	if(req.backpack_position < 0 || req.backpack_position >= my_attribute.backpack_num ||
			req.which_hero < 1 || req.which_hero > 5)
	{
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	if(!backpack_status[req.backpack_position] || my_hero[req.which_hero - 1] == NULL)
	{
		sres.result = -2;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	if(req.hero_position >= my_hero[req.which_hero - 1]->cur_goods_space)
	{
		sres.result = -3;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	HeroResponse& h =  *my_hero[req.which_hero - 1];
	if(req.hero_position == -1)
	{
		req.hero_position = find_hero_equip_sn(h);
		if(req.hero_position == -1)
		{
			sres.result = -4;//errdata
			SendMessage(sres,SimpleResponse_ID);
			return ;
		}
	}
	EquipResponse & e = my_backpack[req.backpack_position];

	if(hero_equip[h.hero_uuid][req.hero_position].witch_pannle == 1)
	{
		EquipResponse & e1 = hero_equip[h.hero_uuid][req.hero_position];//backpack to hero
		EquipResponse temp = hero_equip[h.hero_uuid][req.hero_position];
		e1 = my_backpack[req.backpack_position];
		e1.witch_pannle = 1;
		e1.where_uuid = h.hero_uuid;
		e1.position = req.hero_position;
		e = temp;//hero to backpack
		e.witch_pannle = 0;
		e.position = req.backpack_position;
		redis.set_equip(e,player_UUID);
		redis.set_equip(e1,player_UUID);
	}
	else
	{
		e.where_uuid = h.hero_uuid;
		e.witch_pannle = 1;
		e.position = req.hero_position;
		redis.set_equip(e,player_UUID);
		backpack_status[req.backpack_position] = false;
		hero_equip[h.hero_uuid][req.hero_position] = e;
	}
	HeroRecaculate(h);
	redis.set_hero(h,player_UUID);
	SendMessage(h,HeroResponse_ID);
}
void LoginWorker::UnloadEquipRequestService(object *request)
{
	UnloadEquipRequest req;
	MsgpackConvert(&req,*request);

	SimpleResponse sres;
	sres.msgid = UnloadEquipRequest_ID;
	sres.sn = 0;

	if(req.which_hero < 1 || req.which_hero > 5 || req.position < 0 || req.position > 5)
	{
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	if( my_hero[req.which_hero - 1] == NULL)
	{
		sres.result = -2;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	HeroResponse& h =  *my_hero[req.which_hero - 1];
	if(hero_equip.find(h.hero_uuid) != hero_equip.end())
	{
		if(hero_equip[h.hero_uuid][req.position].witch_pannle != 1)
		{
			sres.result = -3;//errdata
			SendMessage(sres,SimpleResponse_ID);

			return ;
		}
	}
	int backpack_index = GetBackpack();
	if(backpack_index == -1)
	{
		sres.result = -4;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	backpack_status[backpack_index] = true;
	my_backpack[backpack_index] = hero_equip[h.hero_uuid][req.position];
	my_backpack[backpack_index].witch_pannle = 0;
	my_backpack[backpack_index].position = backpack_index;
	redis.set_equip(my_backpack[backpack_index],player_UUID);
	hero_equip[h.hero_uuid][req.position].witch_pannle = -1;
	HeroRecaculate(h);
	redis.set_hero(h,player_UUID);
	SendMessage(h,HeroResponse_ID);
	sres.result = 0;
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::SaleGoodsRequestService(object *request)
{

	SaleGoodsRequest req;
	MsgpackConvert(&req,*request);
	if(req.position >= 0 || my_attribute.backpack_num < 64)
	{
		if(backpack_status[req.position])
		{
			if(req.diamond_cost <= 0)req.diamond_cost = 1;
			EquipResponse & e = my_backpack[req.position];
			MarketGoodsResponse res;
			res.player = my_attribute.name;
			res.remain_time = time(0);
			res.diamond_cost = req.diamond_cost;
			res.goods_type = 0;
			res.position = 1;// is var
			res.equip_sn = e.equip_sn;
			res.part = e.part;
			res.quality = e.quality;
			res.level = e.level;
			res.attr1_type = e.attr1_type;
			res.attr1_value = e.attr1_value;
			res.attr2_type = e.attr2_type;
			res.attr2_value = e.attr2_value;
			res.extral1_type = e.extral1_type;
			res.extral1_value = e.extral1_value;
			res.extral2_type = e.extral2_type;
			res.extral2_value = e.extral2_value;
			//write redis
			redis.del_equip(res.equip_sn,player_UUID);
			backpack_status[req.position] = false;
			redis.set_market(res);
			SimpleResponse sres;
			sres.msgid = SaleGoodsRequest_ID;
			sres.sn = 0;
			sres.result = 0;//success
			SendMessage(sres,SimpleResponse_ID);
			return ;
		}
	}
	SimpleResponse sres;
	sres.msgid = SaleGoodsRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::BuyMarketGoodsRequestService(object *request)
{
	BuyMarketGoodsRequest req;
	MsgpackConvert(&req,*request);
	SimpleResponse sres;
	sres.msgid = BuyMarketGoodsRequest_ID;
	sres.sn = 0;
	map<string,int>::iterator it = market_index.find(req.goods_sn);
	if(it != market_index.end())
	{
		if(it->second >= 0 && it->second < gloabal_market.size())
		{
			int backpack_index = GetBackpack();
			if( backpack_index != -1)
			{
				MarketGoodsResponse&  e = gloabal_market[it->second];
				if(my_attribute.diamond >= e.diamond_cost)
				{
					my_attribute.diamond -= e.diamond_cost;
					mysql.update_diamond(my_attribute.diamond,player_UUID);
					unsigned int uuid;
					redis.find_user_uuid(e.player,uuid);
					int other_diamond = mysql.get_diamond(uuid);
					//other_diamond += e.diamond_cost;
					//mysql.update_diamond(other_diamond,uuid);
					//if()player online send it
					EquipResponse res;
					res.witch_pannle = 0;
					res.position = backpack_index;
					res.equip_sn = e.equip_sn;
					res.part = e.part;
					res.quality = e.quality;
					res.level = e.level;
					res.attr1_type = e.attr1_type;
					res.attr1_value = e.attr1_value;
					res.attr2_type = e.attr2_type;
					res.attr2_value = e.attr2_value;
					res.extral1_type = e.extral1_type;
					res.extral1_value = e.extral2_value;
					res.extral2_type = e.extral2_type;
					res.extral2_value = e.extral2_value;
					res.diamond_cost = 10;
					backpack_status[res.position] = true;
					my_backpack[res.position] = res;
					redis.del_market(e.equip_sn);
					redis.set_equip(res,player_UUID);
					SendMessage(res,EquipResponse_ID);
					sres.result = 0;
					SendMessage(sres,SimpleResponse_ID);
					AssetResponse aress;
					aress.diamond = my_attribute.diamond;
					aress.iron = my_attribute.iron;
					aress.soil = my_attribute.soil;
					aress.stone = my_attribute.stone;
					aress.wood = my_attribute.wood;
					SendMessage(aress,AssetResponse_ID);
					EmailSaleResponse eres;
					eres.diamond = e.diamond_cost;
				    Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
				    Poco::UUID uid1 = gen.createRandom();
					eres.email_uuid = uid1.toString();
					eres.other_name = my_attribute.name;
					eres.time = time(0);
					redis.set_email_sale(eres,uuid);
					boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >::iterator it_u = UuidHandlerPoitiner.find(uuid);
					if(it_u != UuidHandlerPoitiner.end())
					{
						it_u->second->email_sale_list.email_sale_list[eres.email_uuid] = eres;
						it_u->second->SendMessage(eres,EmailSaleResponse_ID);
						/*
						it_u->second->my_attribute.diamond = other_diamond;
						AssetResponse aress;
						aress.diamond = other_diamond;
						aress.iron = it_u->second->my_attribute.iron;
						aress.soil = it_u->second->my_attribute.soil;
						aress.stone = it_u->second->my_attribute.stone;
						aress.wood = it_u->second->my_attribute.wood;
						it_u->second->SendMessage(aress,AssetResponse_ID);*/
					}
					//add this equip in backpack
				}
				else
				{
					sres.result = MONEY_NOT_ENOUGH;
					SendMessage(sres,SimpleResponse_ID);
				}
			}
			else
			{
				sres.result = SPACE_NOT_ENOUGH;
				SendMessage(sres,SimpleResponse_ID);
			}
			return ;
		}
	}
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::LookMarketGoodsRequestService(object *request)
{
	LookMarketGoodsRequest req;
	MsgpackConvert(&req,*request);
	int all_page = gloabal_market.size()/5 + 1;
	if(req.page_num < 1 || req.page_num > all_page)
	{
		SimpleResponse sres;
		sres.msgid = BuyMarketGoodsRequest_ID;
		sres.sn = 0;
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
	}
	else
	{
		MarketListResponse res;
		for(int i = (req.page_num - 1)*5;i< req.page_num * 5;i++)
		{
			if(i < gloabal_market.size())
			{
				res.maket_list.push_back(gloabal_market[i]);
			}
		}
		res.max_page_num = all_page;
		SendMessage(res,MarketListResponse_ID);
	}
}
int LoginWorker::GetBackpack()
{
	if(my_attribute.backpack_num < 12 || my_attribute.backpack_num > 64 )return -1;
	for(int i = 0;i< my_attribute.backpack_num;i++)
	{
		if(!backpack_status[i])
		{
			return i;
		}
	}
	return -1;
}
void LoginWorker::OpenBoxRequestService(object* request)
{
	OpenBoxRequest req;
	MsgpackConvert(&req,*request);
	int backindex = GetBackpack();
	if( backindex == -1)
	{
		SimpleResponse sres;
		sres.msgid = OpenBoxRequest_ID;
		sres.sn = 0;
		sres.result = SPACE_NOT_ENOUGH;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	EquipResponse res;
	res.witch_pannle = 0;
	res.position = backindex;
    Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uid1 = gen.createRandom();
	res.equip_sn = uid1.toString();
	res.part = RandomUtil::GetARandom() % 4 + 1;
	res.quality = RandomUtil::GetARandom() % 3 + 1;
	res.level = 10;
	res.attr1_type = RandomUtil::GetARandom() % 7 + 1;
	res.attr1_value = 10;
	res.attr2_type = RandomUtil::GetARandom() % 7 + 1;
	res.attr2_value = 10;
	res.extral1_type = RandomUtil::GetARandom() % 7 + 1;
	res.extral1_value = 10;
	res.extral2_type = RandomUtil::GetARandom() % 7 + 1;
	res.extral2_value = 10;
	res.diamond_cost = 10;
	res.where_uuid = "NoEquipUUID";
	backpack_status[res.position] = true;
	my_backpack[res.position] = res;
	redis.set_equip(res,player_UUID);
	SendMessage(res,EquipResponse_ID);
}

void LoginWorker::FastFinishRequestService(object *request)
{
	FastFinishRequest req;
	MsgpackConvert(&req,*request);
	if(my_attribute.my_build.find(req.build_sn) != my_attribute.my_build.end())
	{
		long remain_time = my_attribute.my_build[req.build_sn].time - (time(0) - my_attribute.my_build[req.build_sn].ctime);
		if( remain_time <= 0)
		{
			remain_time = 1;
		}
		int diamond = ( remain_time / 300)  + 1;
		if(my_attribute.diamond >= diamond)
		{
			if(building_timer.find(req.build_sn)!= building_timer.end())
			{
					my_attribute.diamond -= diamond;
					build_timer_handle(req.build_sn);
					mysql.update_diamond(my_attribute.diamond,player_UUID);
					AssetResponse aress;
					aress.diamond = my_attribute.diamond;
					aress.iron = my_attribute.iron;
					aress.soil = my_attribute.soil;
					aress.stone = my_attribute.stone;
					aress.wood = my_attribute.wood;
					SendMessage(aress,AssetResponse_ID);
					return ;
			}
			else
			{
				list<string>::iterator it = building.begin();
				for(;it != building.end();it++)
				{
					if(*it == req.build_sn)
					{
						MyBuildDataResponse & b = my_attribute.my_build[req.build_sn];
						b.status = 0;
						b.time = 0;
						b.build_level += 1;
						b.cur_hp = global_build[b.build_type][b.build_level].max_hp;
						if(b.big_type == 2)
						{
							b.status = 0;
						    b.ctime = time(0);
						    b.time = 0;
						    b.cur_resource = 0;
						}
						redis.set_build(b,player_UUID);
						SendMessage(b,MyBuildDataResponse_ID);
						building.erase(it);
						my_attribute.diamond -= diamond;
						mysql.update_diamond(my_attribute.diamond,player_UUID);
						AssetResponse aress;
						aress.diamond = my_attribute.diamond;
						aress.iron = my_attribute.iron;
						aress.soil = my_attribute.soil;
						aress.stone = my_attribute.stone;
						aress.wood = my_attribute.wood;
						SendMessage(aress,AssetResponse_ID);
						return ;
					}
				}
			}
	}
	}
	SimpleResponse sres;
	sres.msgid = FastFinishRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
	//if(building)
}

int LoginWorker::get_combatunion_level()
{
	if(my_attribute.my_build.find(my_attribute.combat_uuid) != my_attribute.my_build.end())
	{
		return my_attribute.my_build[my_attribute.combat_uuid].build_level;
	}
	return 0;
}
void LoginWorker::UpgradeArmTypeRequestService(object *request)
{
	UpgradeArmTypeRequest req;
	MsgpackConvert(&req,*request);
	int test_size = global_arms.size() - 1;
	if(req.arm_type < 1 || req.arm_type > global_arms.size() - 1)
	{
		SimpleResponse sres;
		sres.msgid = UpgradeArmTypeRequest_ID;
		sres.sn = 0;
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	int max_level = 0;
	redis.get_arm_max_level(req.arm_type,max_level,player_UUID);
	test_size =  global_arms[req.arm_type].size() - 1;
	if(max_level >= global_arms[req.arm_type].size() - 1)
	{
		SimpleResponse sres;
		sres.msgid = UpgradeArmTypeRequest_ID;
		sres.sn = 0;
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	if(max_level != 0)
	{
		max_level += 1;
		redis.set_arm_max_level(req.arm_type,max_level,player_UUID);
		SimpleResponse sres;
		sres.msgid = UpgradeArmTypeRequest_ID;
		sres.sn = 0;
		sres.result =0;//errdata
		SendMessage(sres,SimpleResponse_ID);
		return ;
	}
	int combat_level = get_combatunion_level();
	if((req.arm_type)%3 == 1)
	{
		if(combat_level > (req.arm_type/3 ))
		{
			redis.set_arm_max_level(req.arm_type,1,player_UUID);
			SimpleResponse sres;
			sres.msgid = UpgradeArmTypeRequest_ID;
			sres.sn = 0;
			sres.result =0;//errdata
			SendMessage(sres,SimpleResponse_ID);
			return ;
		}
	}
	else
	{
		int pre_max_level = 0;
		int pre_type = ((req.arm_type - 1)/3)*3 + 1;
		redis.get_arm_max_level(pre_type,pre_max_level,player_UUID);
		if(pre_max_level == global_arms[pre_type].size() - 1)
		{
			redis.set_arm_max_level(req.arm_type,1,player_UUID);
			SimpleResponse sres;
			sres.msgid = UpgradeArmTypeRequest_ID;
			sres.sn = 0;
			sres.result =0;//errdata
			SendMessage(sres,SimpleResponse_ID);
			return ;
		}
	}

	SimpleResponse sres;
	sres.msgid = UpgradeArmTypeRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::send_result(int result,int msg_id)
{
	SimpleResponse sres;
	sres.msgid = msg_id;
	sres.sn = 0;
	sres.result = result;//errdata
	SendMessage(sres,SimpleResponse_ID);
}
int LoginWorker::get_hero_index(string hero_uuid)
{
	for(int i = 0;i<6;i++)
	{
		if(my_hero[i])
		{
			if(my_hero[i]->hero_uuid ==  hero_uuid)
			{
				return i;
			}
		}
	}
	return -1;
}
void LoginWorker::AppointHeroRequestService(object *request)
{
	AppointHeroRequest req;
	MsgpackConvert(&req,*request);
	int target_index = -1;
	for(int i = 0;i<6;i++)
	{
		if(my_hero[i])
		{
			if(my_hero[i]->where ==  req.job_title)
			{
				my_hero[i]->where = 0;
				redis.set_hero(*my_hero[i],player_UUID);
				break;
			}
		}
	}
	for(int i = 0;i<6;i++)
	{
		if(my_hero[i])
		{
			if(my_hero[i]->hero_uuid ==  req.hero_uuid)
			{
				my_hero[i]->where = req.job_title;
				redis.set_hero(*my_hero[i],player_UUID);
				send_result(0,CancelHeroRequest_ID);
				return ;
			}
		}
	}
}

void LoginWorker::CancelHeroRequestService(object *request)
{
	CancelHeroRequest req;
	MsgpackConvert(&req,*request);
	for(int i = 0;i<6;i++)
	{
		if(my_hero[i])
		{
			if(my_hero[i]->hero_uuid ==  req.hero_uuid)
			{
				my_hero[i]->where = 0;
				redis.set_hero(*my_hero[i],player_UUID);
				send_result(0,CancelHeroRequest_ID);
				return ;
			}
		}
	}
	send_result(-1,CancelHeroRequest_ID);
}
int random_quality(int type)
{
	printf("siz e is %u \n",hero_update_config.size());
	if(type <= 0 || type > hero_update_config.size())
	{
		type = 1;
	}
	type -= 1;
	if(hero_update_config[type].start5 != 0)
	{
		if(RandomUtil::GetARandom() % 1000 < hero_update_config[type].start5)
		{
			return 5;
		}
	}
	if(hero_update_config[type].start4 != 0)
	{
		if(RandomUtil::GetARandom() % 1000 < hero_update_config[type].start4)
		{
			return 4;
		}
	}
	if(hero_update_config[type].start3 != 0)
	{
		if(RandomUtil::GetARandom() % 1000 < hero_update_config[type].start3)
		{
			return 3;
		}
	}
	if(hero_update_config[type].start2 != 0)
	{
		if(RandomUtil::GetARandom() % 1000 < hero_update_config[type].start2)
		{
			return 2;
		}
	}
	if(hero_update_config[type].start1 != 0)
	{
		if(RandomUtil::GetARandom() % 1000 < hero_update_config[type].start1)
		{
			return 1;
		}
	}
	return 1;
}
void get_hero_att(int quality,HeroResponse& hero)
{
	if(--quality <= 0 || quality >= hero_base_config.size())quality = 0;
	struct att
	{
		float att;
		float att_grow;
	};
	att hero_att[5] = {};
	//max
	hero_att[0].att =hero_base_config[quality].s_max_limit_min +
			RandomUtil::GetARandom()%(hero_base_config[quality].s_max -  hero_base_config[quality].s_max_limit_min);
	hero_att[0].att_grow = (float)(hero_base_config[quality].s_max_limit_grow +
			RandomUtil::GetARandom()%(hero_base_config[quality].s_max_grow -  hero_base_config[quality].s_max_limit_grow))/100.0;
	//min
	hero_att[1].att =hero_base_config[quality].s_min +
			RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_max -  hero_base_config[quality].s_min);
	hero_att[1].att_grow = (float)(hero_base_config[quality].s_min_grow +
			RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_grow -  hero_base_config[quality].s_min_grow))/100.0;
	//putong
	hero_att[2].att =hero_base_config[quality].s_min +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_max -  hero_base_config[quality].s_min);
	hero_att[2].att_grow = (float)(hero_base_config[quality].s_min_grow +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_grow -  hero_base_config[quality].s_min_grow))/100.0;
	hero_att[3].att =hero_base_config[quality].s_min +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_max -  hero_base_config[quality].s_min);
	hero_att[3].att_grow = (float)(hero_base_config[quality].s_min_grow +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_grow -  hero_base_config[quality].s_min_grow))/100.0;
	hero_att[4].att =hero_base_config[quality].s_min +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_max -  hero_base_config[quality].s_min);
	hero_att[4].att_grow = (float)(hero_base_config[quality].s_min_grow +
				RandomUtil::GetARandom()%(hero_base_config[quality].s_min_limit_grow -  hero_base_config[quality].s_min_grow))/100.0;

	bool index_array[5] = {true,true,true,true,true};

	for(int i = 0;i < 5;i++)
	{
		int index = RandomUtil::GetARandom()%(5 - i);
		if(index_array[index] == false)
		{
			for(int j = 0;j<5;j++)
			{
				if(index_array[(++index)%5])
				{
					break;
				}
			}
		}
		index_array[index] = false;
		if(i == 0)
		{
			hero.type = index + 1;
		}
		switch(index)
		{
		case 0:
			hero.leader = hero_att[i].att;
			hero.base_att.base_leader = hero_att[i].att;
			hero.leader_grow = hero_att[i].att_grow;
			break;
		case 1:
			hero.strategy = hero_att[i].att;
			hero.base_att.base_strategy = hero_att[i].att;
			hero.strategy_grow = hero_att[i].att_grow;
			break;
		case 2:
			hero.economy = hero_att[i].att;
			hero.base_att.base_economy = hero_att[i].att;
			hero.economy_grow = hero_att[i].att_grow;
			break;
		case 3:
			hero.strength = hero_att[i].att;
			hero.base_att.base_strength = hero_att[i].att;
			hero.strength_grow = hero_att[i].att_grow;
			break;
		case 4:
			hero.magic_power = hero_att[i].att;
			hero.base_att.base_magic_power = hero_att[i].att;
			hero.magic_power_grow = hero_att[i].att_grow;
			break;
		default://case 0 and default
			hero.leader = hero_att[i].att;
			hero.base_att.base_leader = hero_att[i].att;
			hero.leader_grow = hero_att[i].att_grow;
			break;
		}
	}

}
void HeroBaseAttCaculate(HeroResponse* res)
{
	if(res->quality < 1 ||  res->quality > hero_base_config.size())res->quality = 1;
	int index = res->quality - 1;
	res->max_hp = hero_base_config[index].base_hp + (int)res->strength * 5;
	res->max_mp = hero_base_config[index].base_mp + (int)res->magic_power;
	res->attack = hero_base_config[index].base_attack + (int)res->strength;
	res->defence = hero_base_config[index].base_defence + (int)(res->strength * 0.02);
	res->base_att.base_hp =  hero_base_config[index].base_hp;
	res->base_att.base_mp =  hero_base_config[index].base_hp;
	res->base_att.base_attack =  hero_base_config[index].base_attack;
	res->base_att.base_defence =  hero_base_config[index].base_defence;
}

void LoginWorker::refresh_random_hero(int randomtype)
{
	if(randomtype < 1 ||randomtype > hero_update_config.size())
	{
		randomtype = 1;
	}
	HeroListResponse list_res;
	for(int i = 0;i< 3;i++)
	{
		if(rand_hero[i])
		{
			redis.del_hero(rand_hero[i]->hero_uuid,player_UUID);
			delete rand_hero[i];
		}
		int quality = random_quality(my_attribute.recuite_type);
		HeroResponse* res = new HeroResponse();
		Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
	    Poco::UUID uid1 = gen.createRandom();
	    res->hero_sn = i + 1;//1
	    res->name = "NoName";//2
	    res->hero_uuid =  uid1.toString();//3
	    res->quality = quality; //4
	    res->level = 1;//5
	    get_hero_att(quality,*res);
	    HeroBaseAttCaculate(res);
	    res->cur_hp =res->max_hp;//16
	    res->cur_mp = res->max_mp;//18
	    res->pre_skill = 0;//22
	    res->study_skill = 0;//23
	    res->cur_goods_space = hero_base_config[ quality -1 ].goods_space;//24
	    res->max_goods_space = hero_base_config[ quality -1 ].max_space;//25
	    res->exp = 0;
	    res->where = 1;
	    rand_hero[i] = res;
	    list_res.hero_list.push_back(*res);
	    redis.set_hero(*res,player_UUID);
	}
	SendMessage(list_res,HeroListResponse_ID);
}
void LoginWorker::RecruitRequestService(object *request)
{
	RecruitRequest req;
	MsgpackConvert(&req,*request);
	if(req.random_type > 0 && req.random_type < hero_update_config.size())
	{
		if(hero_update_config[req.random_type - 1].diamond <=  my_attribute.diamond)
		{
			my_attribute.diamond -= hero_update_config[req.random_type - 1].diamond;
			mysql.update_diamond(my_attribute.diamond,player_UUID);

			AssetResponse aress;
			aress.diamond = my_attribute.diamond;
			aress.iron = my_attribute.iron;
			aress.soil = my_attribute.soil;
			aress.stone = my_attribute.stone;
			aress.wood = my_attribute.wood;
			SendMessage(aress,AssetResponse_ID);

			refresh_random_hero(req.random_type);


			my_attribute.recuite_type = req.random_type;
			my_attribute.chance_num = 3;
			redis.set_recurite(req.random_type,3,player_UUID);
			return ;
		}
	}
	SimpleResponse sres;
	sres.msgid = RecruitRequest_ID;
	sres.sn = 0;
	sres.result = -1;//errdata
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::GetRandHeroRequestService(object *request)
{
	DropHeroRequest req;
	MsgpackConvert(&req,*request);
	SimpleResponse sres;
	sres.msgid = GetRandHeroRequest_ID;
	sres.sn = 0;
	if(req.hero_sn > 0 && req.hero_sn < 4)
	{
		int position = -1;
		for(int i = 0;i< 5;i++)
		{
			if(my_hero[i] == NULL)
			{
				position = i;
				break;
			}

		}
		if(position == -1 || rand_hero[req.hero_sn - 1] == NULL)
		{
			sres.result = -1;//errdata
			SendMessage(sres,SimpleResponse_ID);
		}
		else
		{
			my_hero[position] = rand_hero[req.hero_sn - 1];
			rand_hero[req.hero_sn - 1] = NULL;
			my_hero[position]->where = 0;
			my_hero[position]->hero_sn = position + 1;
			vector<EquipResponse>  temp;
			for(int i = 0;i< 6;i++)
			{
				EquipResponse e = {};
				e.witch_pannle = -1;
				temp.push_back(e);
			}
			hero_equip[my_hero[position]->hero_uuid] = temp;
			redis.set_hero(*my_hero[position],player_UUID);
			sres.result = 0;//errdata
			SendMessage(sres,SimpleResponse_ID);
		}
		return ;
	}
	sres.result = -2;//errdata
	SendMessage(sres,SimpleResponse_ID);
}


int LoginWorker::find_hero_position()
{
	for(int i = 0;i < 6;i++)
	{
		if(my_hero[i] == NULL)
		{
			return i;
		}
	}
	return -1;
}

void LoginWorker::DropHeroRequestService(object *request)
{
	DropHeroRequest req;
	MsgpackConvert(&req,*request);
	if(req.hero_sn <= 0 && req.hero_sn > 6)
	{
		SimpleResponse sres;
		sres.msgid = DropHeroRequest_ID;
		sres.sn = 0;
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
	}
	else
	{
		if(my_hero[req.hero_sn - 1])
		{
			if(hero_equip[my_hero[req.hero_sn - 1]->hero_uuid].size() != 0)
			{
				SimpleResponse sres;
				sres.msgid = DropHeroRequest_ID;
				sres.sn = 0;
				sres.result = -2;//errdata
				SendMessage(sres,SimpleResponse_ID);
				return ;
			}
			hero_equip.erase(my_hero[req.hero_sn - 1]->hero_uuid);
			//delete redis;
			redis.del_hero(my_hero[req.hero_sn - 1]->hero_uuid,player_UUID);
			delete my_hero[req.hero_sn - 1];
			my_hero[req.hero_sn - 1] = NULL;
			if(req.hero_sn < 6 || my_hero[req.hero_sn - 1] != NULL )
			{
				for(int i = req.hero_sn;i<6;i++)
				{
					if(my_hero[i])
					{
						my_hero[i]->hero_sn = i;
						my_hero[i - 1] = my_hero[i];
						my_hero[i] = NULL;
					}
				}
				redis.move_hero(my_hero,req.hero_sn - 1,player_UUID);
			}
			SimpleResponse sres;
			sres.msgid = DropHeroRequest_ID;
			sres.sn = 0;
			sres.result = 0;//errdata
			SendMessage(sres,SimpleResponse_ID);
		}
		SimpleResponse sres;
		sres.msgid = DropHeroRequest_ID;
		sres.sn = 0;
		sres.result = -2;//errdata
		SendMessage(sres,SimpleResponse_ID);
	}
}


void LoginWorker::GetEmailGoodsRequestService(object *request)
{
	GetEmailGoodsRequest req;
	MsgpackConvert(&req,*request);
	SimpleResponse sres;
	sres.msgid = GetEmailGoodsRequest_ID;
	sres.sn = 0;
	map<string,EmailSaleResponse>::iterator it = email_sale_list.email_sale_list.find(req.email_uuid);
	if(it != email_sale_list.email_sale_list.end())
	{
			if(email_sale_list.email_sale_list[req.email_uuid].diamond <= 0)
			{
				int position = GetBackpack();
				if(position != -1)
				{
					sres.result = 0;//errdata
					backpack_status[position] = true;
					my_backpack[position] = email_sale_list.email_sale_list[req.email_uuid].equip;
					my_backpack[position].witch_pannle = 0;
					my_backpack[position].position = position;
					redis.set_equip(my_backpack[position],player_UUID);
					redis.del_email_sale(email_sale_list.email_sale_list[req.email_uuid],player_UUID);
				}
			}
			else
			{
				my_attribute.diamond += email_sale_list.email_sale_list[req.email_uuid].diamond;
				mysql.update_diamond(my_attribute.diamond,player_UUID);
			}
			redis.del_email_sale(email_sale_list.email_sale_list[req.email_uuid],player_UUID);
			email_sale_list.email_sale_list.erase(it);
			sres.result = 0;
	}
	else
	{
		sres.result = -1;//errdata
	}
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::EmailNextPageRequstService(object *request)
{



}

void LoginWorker::PushNotify(MarketGoodsResponse& value)
{
	//find user uuid;
	unsigned int uuid;
	redis.find_user_uuid(value.player,uuid);
	EquipResponse res;
	res.witch_pannle = 2;
	Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uid1 = gen.createRandom();
	res.position = 0;
	res.equip_sn = value.equip_sn;
	res.where_uuid = uid1.toString();
	res.part = value.part;
	res.quality = value.quality;
	res.level = value.level;
	res.attr1_type = value.attr1_type;
	res.attr1_value = value.attr1_value;
	res.attr2_type = value.attr2_type;
	res.attr2_value = value.attr2_value;
	res.extral1_type = value.extral1_type;
	res.extral1_value = value.extral2_value;
	res.extral2_type = value.extral2_type;
	res.extral2_value = value.extral2_value;
	res.diamond_cost = 10;

	EmailSaleResponse eres;
	eres.diamond = 0;
	eres.email_uuid = res.where_uuid;
	eres.other_name = "MySelf";
	eres.equip = res;
	eres.time = time(0);
	redis.del_market(value.equip_sn);
	redis.set_email_sale(eres,uuid);
	boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >::iterator it_u = UuidHandlerPoitiner.find(uuid);
	if(it_u != UuidHandlerPoitiner.end())
	{
		it_u->second->email_sale_list.email_sale_list[res.where_uuid] = eres;
		it_u->second->SendMessage(eres,EmailSaleResponse_ID);
	}
}



void LoginWorker::SetArmsToHeroRequestService(object *request)
{
	//	remove this msg,because write it will be xiepengcheng

	SetArmsToHeroRequest req;
	MsgpackConvert(&req,*request);

	//  check
	int hero_index = get_hero_index(req.hero_uuid);
	if(hero_index == -1)
	{
		send_result(0,SetArmsToHeroRequest_ID);
	}

	//  swite redis



	//  simplresponse


}

void LoginWorker::QuikAttackRequestService(object *request)
{
	QuikAttackRequest req;
	MsgpackConvert(&req,*request);
	//  check time and caculate diamond

	//	check diamond is enough

	//

}

void LoginWorker::AttackEnemyRequestService(object *request)
{
	//	check flag is OK! candition is Enough

	//	OK  and response target info

	//	else response error

	AttackEnemyRequest req;
	MsgpackConvert(&req,*request);
	unsigned int enemy_uuid;

	if(redis.find_user_uuid(req.enemy_name,enemy_uuid))
	{
		EnemyAttruibuteResponse res;
		redis.get_enemy_att(res,enemy_uuid);
		SendMessage(res,EnemyAttruibuteResponse_ID);
	}
	else
	{
		SimpleResponse sres;
		sres.msgid = AttackEnemyRequest_ID;
		sres.sn = 0;
		sres.result = -1;//errdata
		SendMessage(sres,SimpleResponse_ID);
	}


}
