/*
 * _system_world_map.h
 *
 *  Created on: Nov 5, 2013
 *      Author: zhongbing
 */

#ifndef SYSTEM_WORLD_MAP_H_
#define SYSTEM_WORLD_MAP_H_

#include <boost/container/vector.hpp>
#include <boost/container/map.hpp>
#include <redis_base.h>
#include <redis_word_map.h>
#include <msgpack.hpp>
#include <base_function.hpp>
#include <msgpack_struct.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <timer.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/container/map.hpp>
#include <vector>
#include <RandomUtil.h>

#define MAX_SERVICE_NUM 13

using namespace msgpack;
using namespace eddy;

class WorldMapBase
{
	typedef void (WorldMapBase::*Service)(object&,unsigned int);


private:

	//data menber
	boost::container::map<int,int> remaining_position;//first int is (x<<16 + y),second int is map_key_index

	boost::container::vector<int> map_key;//int is (x<<16) + y

	//modual object
	RedisWordMap* redis;

	Service service[MAX_SERVICE_NUM];

	boost::asio::io_service world_map_thread;

	boost::asio::io_service::work*  work_;

	long last_update[6400];
	vector<unsigned int> all_map_player_uuid;

	boost::container::map<unsigned int,long> player_last_update;

	Timer* timer;

public ://important

	WorkerSender * sender;

	mdwrk* session;	void check_remain_position(int checktype);

	void dispatch_func(int index,object& requst,unsigned int player_uuid);

	static WorldMapBase& GetInstance();

	boost::asio::io_service& get_io_service();

	void run();

private://private function

	WorldMapBase();

	WorldMapBase(const WorldMapBase&){}

	void register_all_service();

	void first_terrain_init();

	void check_remaining_position();

//	long update_time[100];

private :

	~WorldMapBase();

	bool is_update_area(int area,long& last_time);

	void set_final_update_time(int area);

	bool GetRangePosition(short& x,short& y,unsigned int uuid);

	bool GetSettlePosition(short x,short y,unsigned int uuid);

	void GetNearbyInfo(short x,short y,WorlMapNearByResponse* nearby);

	void AddNewPositionToMap(short x,short y);

	void UpdateWorldMap(short x,short y,unsigned char s);

	void BroadCastPlayerInfo();

private :// message service relate;

	void RandomNewMapRequestService(object& request,unsigned int player_uuid);//end 1 broadcast to other map player

	void WorldMapNearByRequestService(object& request,unsigned int player_uuid);//end2

	void RandomMoveCityRequestService(object& request,unsigned int player_uuid);//end 3

	void SettleMoveCityRequestService(object& request,unsigned int player_uuid);//end4

	void LeaveWordMapRequestService(object& request,unsigned int player_uuid);//5

	void WorldMapAttackCityRequestService(object& request,unsigned int player_uuid);//todo6

	void HeroMoveRequestService(object& request,unsigned int player_uuid);//todo7

	void LookUpCityRequestService(object& request,unsigned int player_uuid);//todo8

	void WorldMapEnterCityRequestService(object& request,unsigned int player_uuid);//todo9

	void WorldMapPlunderResourceRequestService(object& request,unsigned int player_uuid);//todo9

	void PVEAttackRequestService(object& request,unsigned int player_uuid);//todo9

	void KillMonsterRequestService(object& request,unsigned int player_uuid);//todo9

	void GetSomePlayerRequestService(object& request,unsigned int player_uuid);

	void LogoutRequestService(object& request,unsigned int player_uuid);//10

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
public:

	//update operation

	struct coord
	{
		int x;
		int y;
	};

	void check_remain_coord(unsigned char checktype,int big_area_num,vector<coord>  remain_coord[100]);//big_area_num  0-63

	void update_wood(int i,vector<coord>  remain_coord[100],long cur_time);

	void update_stone(int i,vector<coord>  resource_coord[100],long cur_time);

	void update_iron(int i,vector<coord>  resource_coord[100],long cur_time);

	void update_resource();

	void update_monstor(int i,vector<coord>  resource_coord[100],long cur_time);


};
#endif /* SYSTEM_WORLD_MAP_H_ */
