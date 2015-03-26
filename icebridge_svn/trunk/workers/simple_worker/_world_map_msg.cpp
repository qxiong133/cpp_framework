/*
 * _system_world_map_msg.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: zhongbing
 */
#include "_world_map.h"
#include <iostream>
#include <msgpack_convert.hpp>
#include <map>

using namespace std;
extern NearByResourceResponse  resource[6400];
extern NearByPlayerInfoResponse  player_info[6400];
extern NearByMapMonsterResponse  monster[6400];
extern unsigned char world_position[640][640];

map<unsigned int,map<int,bool> > player_update_stutas;

void WorldMapBase::RandomNewMapRequestService(object& request,unsigned int player_uuid)
{
	short x,y;
	if(GetRangePosition(x,y,player_uuid))
	{
		MyWorldMapCoordResponse res;
		res.x = x;
		res.y = y;
		sender->SendToPlayer(res,MyWorldMapCoordResponse_ID,player_uuid);
		int area = (y/8)*80 + x/8;
		last_update[area] = time(0);
		WorldMapPlayerInfoResponse pres;
		redis->get_player_att(pres,player_uuid);
		int key = (x << 16) + y;
		player_info[area].player_info[key] = pres;
		sender->SendToPlayerList(pres,WorldMapPlayerInfoResponse_ID,&all_map_player_uuid);

		PlayerSimpleInfo  p;
		p.x = x;
		p.y = y;
		p.city_level = pres.city_level;
		p.player_name = pres.player_name;
		redis->add_random_player(p);

	}
	else
	{
		SimpleResponse sres;
		sres.msgid = RandomNewMapRequest_ID;
		sres.sn = 1000;
		sres.result = -1000;
		sender->SendToPlayer(sres,SimpleResponse_ID,player_uuid);
	}
}


void AddInNearBy(int area_num,NearByAreaResponse*  res,unsigned int uuid)
{
	if(area_num > 0 && area_num < 6400)
	{
		if(player_update_stutas.find(uuid) != player_update_stutas.end())
		{
			if(player_update_stutas[uuid].find(area_num) == player_update_stutas[uuid].end())
			{
				player_update_stutas[uuid][area_num] = true;
				WorlMapNearByResponse wres;
				wres.area = area_num;
				wres.monster = monster[area_num].monster;
				wres.player_info = player_info[area_num].player_info;
				wres.resource = resource[area_num].resource;
				res->near_area.push_back(wres);
			}
		}
	}
}

void WorldMapBase::WorldMapNearByRequestService(object& request,unsigned int player_uuid)
{
	WorldMapNearByRequest req;
	MsgpackConvert(&req,request);
	NearByAreaResponse  res;
//check is first;is update or check
	if(player_last_update.find(player_uuid) == player_last_update.end())
	{
		map<int,bool> update_map;
		player_update_stutas[player_uuid] = update_map;
		player_last_update[player_uuid] = 0;
		all_map_player_uuid.push_back(player_uuid);
	}
	printf("id is %d\n",req.center_area);
	int area_num = req.center_area;
	AddInNearBy(area_num - 1,&res,player_uuid);
	AddInNearBy(area_num,&res,player_uuid);
	AddInNearBy(area_num + 1,&res,player_uuid);
	AddInNearBy(area_num - 81,&res,player_uuid);
	AddInNearBy(area_num - 80,&res,player_uuid);
	AddInNearBy(area_num - 79,&res,player_uuid);
	AddInNearBy(area_num + 79,&res,player_uuid);
	AddInNearBy(area_num + 80,&res,player_uuid);
	AddInNearBy(area_num + 81,&res,player_uuid);
	if(res.near_area.size()>0)
	{
		player_last_update[player_uuid] = time(0);
		sender->SendToPlayer(res,NearByAreaResponse_ID,player_uuid);
	}
	else
	{
		SimpleResponse sres;
		sres.msgid = WorldMapNearByRequest_ID;
		sres.result = -1;
		sres.sn = 10000;
		cout<<"Don`t Need Updat;"<<endl;
	}
}



void WorldMapBase::RandomMoveCityRequestService(object& request,unsigned int player_uuid)
{
	short x,y;
	if(GetRangePosition(x,y,player_uuid))
	{
		MyWorldMapCoordResponse res;
		res.x = x;
		res.y = y;
		sender->SendToPlayer(res,MyWorldMapCoordResponse_ID,player_uuid);
	}
	else
	{
		SimpleResponse sres;
		sres.msgid = RandomMoveCityRequest_ID;
		sres.sn = 1000;
		sres.result = -1000;
		sender->SendToPlayer(sres,SimpleResponse_ID,player_uuid);
	}
}

void WorldMapBase::SettleMoveCityRequestService(object& request,unsigned int player_uuid)
{
	//xiaohao asset
	SettleMoveCityRequest req;
	MsgpackConvert(&req,request);
	printf("-------------x:%d,y:%d\n",req.x,req.y);
	if(GetSettlePosition(req.x,req.y,player_uuid))
	{
		MyWorldMapCoordResponse res;
		res.x = req.x;
		res.y = req.y;
		sender->SendToPlayer(res,MyWorldMapCoordResponse_ID,player_uuid);
	}
	else
	{
		SimpleResponse sres;
		sres.msgid = SettleMoveCityRequest_ID;
		sres.sn = 1000;
		sres.result = -1000;
		sender->SendToPlayer(sres,SimpleResponse_ID,player_uuid);
	}
	//broadcast to this area player
}
void WorldMapBase::WorldMapAttackCityRequestService(object& request,unsigned int player_uuid)
{

}
void WorldMapBase::HeroMoveRequestService(object& request,unsigned int player_uuid)
{

}
void WorldMapBase::LookUpCityRequestService(object& request,unsigned int player_uuid)
{

}

void WorldMapBase::WorldMapPlunderResourceRequestService(object& request,unsigned int player_uuid)
{


}

void WorldMapBase::WorldMapEnterCityRequestService(object& request,unsigned int player_uuid)
{

}
void WorldMapBase::PVEAttackRequestService(object& request,unsigned int player_uuid)
{

}

void WorldMapBase::KillMonsterRequestService(object& request,unsigned int player_uuid)
{
	KillMonsterRequest req;
	MsgpackConvert(&req,request);
	PVEWinResponse res;
	res.box_num = 1;
	res.level = 1;
	sender->SendToPlayer(res,PVEWinResponse_ID,player_uuid);
}
void WorldMapBase::GetSomePlayerRequestService(object& request,unsigned int player_uuid)
{
	GetSomePlayerRequest req;
	MsgpackConvert(&req,request);
	SomePlayerResponse res;
	redis->get_random_player(res.world_player,10);
	if(res.world_player.size() == 0)
	{
		SimpleResponse sres;
		sres.msgid = GetSomePlayerRequest_ID;
		sres.result = -1;
		sres.sn = 0;
		sender->SendToPlayer(sres,SimpleResponse_ID,player_uuid);
		return ;
	}
	sender->SendToPlayer(res,SomePlayerResponse_ID,player_uuid);
}
void WorldMapBase::LeaveWordMapRequestService(object& request,unsigned int player_uuid)
{
	bool is_exsit = false;
	if(player_last_update.find(player_uuid) != player_last_update.end())
	{
		player_last_update.erase(player_uuid);
	}
	for(unsigned int i = 0;i< all_map_player_uuid.size();i++)
	{
		if(all_map_player_uuid[i] == player_uuid)
		{
			all_map_player_uuid[i] =  all_map_player_uuid[all_map_player_uuid.size() - 1];
			is_exsit = true;
			break ;
		}
	}
	if(is_exsit)all_map_player_uuid.pop_back();
	map<unsigned int,map<int,bool> >::iterator it = player_update_stutas.find(player_uuid);
	if(it != player_update_stutas.end())
	{
		player_update_stutas.erase(player_uuid);
	}
}

void WorldMapBase::LogoutRequestService(object& request,unsigned int player_uuid)
{
	bool is_exsit = false;
	if(player_last_update.find(player_uuid) != player_last_update.end())
	{
		player_last_update.erase(player_uuid);
	}
	for(unsigned int i = 0;i< all_map_player_uuid.size();i++)
	{
		if(all_map_player_uuid[i] == player_uuid)
		{
			all_map_player_uuid[i] =  all_map_player_uuid[all_map_player_uuid.size() - 1];
			is_exsit = true;
			break ;
		}
	}
	if(is_exsit)all_map_player_uuid.pop_back();
	map<unsigned int,map<int,bool> >::iterator it = player_update_stutas.find(player_uuid);
	if(it != player_update_stutas.end())
	{
		player_update_stutas.erase(player_uuid);
	}
}
