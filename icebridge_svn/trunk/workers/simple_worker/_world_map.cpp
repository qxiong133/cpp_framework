#include "_world_map.h"
#include <stdlib.h>
#include <redis_word_map.h>
#include <time.h>

void process(const char * data,int size){
    static WorldMapBase& smallmap = WorldMapBase::GetInstance();

    cout << "process function " ;

    int player_uuid = *((int*)(data+2));
    short message_id = *((short*)(data + 6));
    msgpack::zone z;
    msgpack::object request;
    if(message_id == LogoutRequest_ID)
    {
    	smallmap.dispatch_func(MAX_SERVICE_NUM - 1,request,player_uuid);
    	if(data)
    	{
    		free((void*)data);
    	}
    	return ;
    }

    msgpack::unpack_return ret = msgpack::unpack((const char*)(data + 8), size-8, NULL, &z, &request);
	if(ret == msgpack::UNPACK_SUCCESS)
	{
		smallmap.dispatch_func(message_id - WorldMapNearByRequest_ID,request,player_uuid);
	}
	else
	{
		smallmap.session->send("unpack error or out of array\0 limit",sizeof("unpack error or out of array\0 limit"));
	}
	if(data)
	{
		free((void*)data);
	}

}

void handler_in_worker(const char * data, size_t size)
{
   static WorldMapBase& smallmap = WorldMapBase::GetInstance();
   char *  p_data = (char *)malloc(size);
   memcpy(p_data,data,size);
   smallmap.get_io_service().post(boost::bind(process,p_data, size));
}

WorldMapBase& WorldMapBase::GetInstance()
{
	static WorldMapBase wordbase;
	return wordbase;
}
unsigned char world_position[640][640];

NearByResourceResponse  resource[6400];
NearByPlayerInfoResponse  player_info[6400];
NearByMapMonsterResponse  monster[6400];

WorldMapBase::WorldMapBase()
{
	long cur_time = time(0);

	memset(last_update,cur_time,sizeof(last_update));

	work_ = new boost::asio::io_service::work(world_map_thread);

	register_all_service();

	session = mdwrk::getInstance("worker01", "tcp://localhost:5555", "worldmap", 0,handler_in_worker);

	sender = new WorkerSender(session);

	//memset(update_time,0,sizeof(long)*100);

	first_terrain_init();

	redis = new RedisWordMap();

	redis->word_map_player_init(world_position,player_info);

	check_remaining_position();

	timer = new Timer(world_map_thread);

	//test();player_update
}

WorldMapBase::~WorldMapBase()
{
	delete timer;

	delete work_;

	delete session;

	delete redis;

	delete sender;
}

void WorldMapBase::register_all_service()
{
	/*
	WorldMapNearByRequest_ID,//33
	RandomNewMapRequest_ID,//34
	RandomMoveCityRequest_ID,//35
	SettleMoveCityRequest_ID,//36
	HeroMoveRequest_ID,//37
	WorldMapEnterCityRequest_ID,//38
	WorldMapPlunderResourceRequest_ID,//39
	LookUpCityRequest_ID,//40
	PVEAttackRequest_ID,//41
	KillMonsterRequest_ID,//42
	GetSomePlayerRequest_ID,//43
	LeaveWordMapRequest_ID,//44
	*/
	service[0] = &WorldMapBase::WorldMapNearByRequestService;//20
	service[1] = &WorldMapBase::RandomNewMapRequestService;//21
	service[2] = &WorldMapBase::RandomMoveCityRequestService;//22
	service[3] = &WorldMapBase::SettleMoveCityRequestService;//23
	service[4] = &WorldMapBase::HeroMoveRequestService;//24
	service[5] = &WorldMapBase::WorldMapEnterCityRequestService;//25
	service[6] = &WorldMapBase::WorldMapPlunderResourceRequestService;//26
	service[7] = &WorldMapBase::LookUpCityRequestService;//27
	service[8] = &WorldMapBase::PVEAttackRequestService;//28
	service[9] = &WorldMapBase::KillMonsterRequestService;//29
	service[10] = &WorldMapBase::GetSomePlayerRequestService;//32
	service[11] = &WorldMapBase::LeaveWordMapRequestService;//32
	service[12] = &WorldMapBase::LogoutRequestService;//33
}

void WorldMapBase::dispatch_func(int index,object& requst,unsigned int player_uuid)
{
	if(index < 0 || index > MAX_SERVICE_NUM)
	{
		printf("erro messege pack,and index is %d\n",index);
		return ;
	}
	(this->*service[index])(requst,player_uuid);
}
boost::asio::io_service& WorldMapBase::get_io_service()
{
	return world_map_thread;
}

void WorldMapBase::run()
{
	timer->Wait(boost::bind(&WorldMapBase::update_resource,this),500);
	boost::system::error_code error;
	world_map_thread.run(error);
	if (error)
		std::cerr << error.message() << std::endl;
}
bool WorldMapBase::is_update_area(int area,long& last_time)
{
	/*if(area < 0 || area > 99 || last_time > update_time[area])
	{
		return false;
	}
	last_time = time(0);*/
	return true;
}

void WorldMapBase::set_final_update_time(int area)
{
	if(area < 0 || area > 99)
	{
		return ;
	}
	//update_time[area] = time(0);
}

bool WorldMapBase::GetRangePosition(short& x,short& y,unsigned int uuid)
{
	short oldx,oldy;
	if(redis->get_player_worldinfo(oldx,oldy,uuid))
	{
		AddNewPositionToMap(oldx,oldy);
	}
	int map_key_size = map_key.size();
	if(map_key_size > 0)
	{
		int random = RandomUtil::GetARandom() % map_key.size();
		//printf("world map random position is %d,size is %u\n",random,map_key.size());
		int key = map_key[random];
		x = key >> 16;
		y = key & 0x0000ffff;
		printf("x is %d;y is %d\n",x,y);
		//remove this key;
		remaining_position.erase(key);

		//swap key;
		if(map_key_size == 1)
		{
			map_key.clear();
		}
		else
		{
			int last_map_key = map_key[map_key.size() -1];
			remaining_position[last_map_key] = random;
			map_key[random] = last_map_key;
			map_key.pop_back();
		}
		redis->add_new_player(x,y,uuid);
		printf("GatRangePosition is success\n");
		return true;
	}
	printf("map key is empty\n");
	return false;
}

bool WorldMapBase::GetSettlePosition(short x,short y,unsigned int uuid)
{
	short oldx,oldy;
	if(redis->get_player_worldinfo(oldx,oldy,uuid))
	{
		int key = (x << 16) + y;
		printf("key is : %d\n",key);
		if(remaining_position.find(key) == remaining_position.end())
		{
			printf("key is not exsit: %d\n",key);
			return false;
		}
		else
		{
			int index = remaining_position[key];
			int last_map_key = map_key[map_key.size() -1];
			remaining_position[ last_map_key ] = index;
			map_key[index] =  last_map_key;
			map_key.pop_back();
			remaining_position.erase(key);
			redis->add_new_player(x,y,uuid);
			printf("Get Settle position is success\n");
			AddNewPositionToMap(oldx,oldy);
			return true;
		}
	}
}

void WorldMapBase::GetNearbyInfo(short x,short y,WorlMapNearByResponse* nearby)
{

}

void WorldMapBase::AddNewPositionToMap(short x,short y)
{
	int key = (x << 16) + y;
	map_key.push_back(key);
	remaining_position[key] = map_key.size() - 1;
	//printf("add key x: %d,y: %d,success\n",x,y);
}

void WorldMapBase::UpdateWorldMap(short x,short y,unsigned char s)
{
	if(x <0 || y < 0 || x > 640 || y > 640)
	{
		printf("parameter is error");
	}
	else
	{
		world_position[x][y] = s;
	}
}



/////////===============================================================================================
/////////===============================================================================================
//first data init

static unsigned char minBlock[32][32] =
	{
			{1,3,1,4,3,1,5,1,6,3,1,2,3,1,4,6,2,4,1,4,3,1,6,1,1,2,3,6,5,6,4,3},//1
			{2,1,3,1,6,1,2,6,6,1,1,2,5,2,3,4,2,1,11,1,6,1,1,1,6,4,3,1,6,1,1,2},
			{1,1,3,6,6,2,1,3,1,3,1,6,4,6,3,1,5,1,1,5,3,5,1,1,6,4,6,2,1,3,1,6},
			{2,1,4,6,3,5,3,6,1,4,6,1,1,3,4,3,6,1,4,6,6,3,1,2,3,6,1,5,1,4,3,2},
			{4,6,4,6,6,1,1,1,4,2,6,9,1,6,3,1,6,3,1,1,1,1,6,2,1,1,1,1,1,1,3,5},//5
			{3,4,1,1,1,1,2,3,6,2,5,1,1,2,10,6,2,1,2,1,1,6,3,7,3,1,11,1,4,1,4,3},
			{5,1,6,6,1,4,3,2,6,1,6,1,1,1,6,1,3,2,1,6,1,6,4,3,2,5,1,1,3,6,2,6},
			{6,3,6,1,6,1,3,1,8,3,1,6,6,6,1,3,2,3,6,6,2,1,4,3,6,1,1,6,3,2,6,4},
			{4,6,1,2,6,2,1,5,1,4,6,3,6,3,2,1,1,2,6,3,1,6,2,1,3,6,6,6,3,1,3,6},
			{5,1,4,3,6,1,1,6,6,3,6,4,3,1,2,1,3,5,2,3,6,6,3,5,4,5,1,2,1,3,4,5},//10
			{2,6,1,1,2,3,4,1,3,5,4,1,6,6,6,3,1,4,6,1,1,6,3,4,2,4,8,1,2,3,6,1},
			{6,2,1,4,6,3,1,2,4,1,6,2,1,5,3,2,3,1,6,1,6,3,2,3,10,6,5,1,4,9,4,2},
			{3,1,6,5,4,1,2,3,1,2,6,1,6,2,6,3,6,3,6,4,6,6,3,4,3,1,6,1,6,3,6,1},
			{3,1,1,4,6,6,4,1,1,1,6,1,5,4,3,1,6,6,6,2,1,1,2,1,2,4,2,1,6,6,6,1},
			{6,2,1,5,3,6,3,1,11,3,6,1,6,1,2,1,6,5,4,6,6,5,2,1,3,2,3,6,2,3,1,2},//15
			{3,6,6,1,6,3,1,2,1,5,4,2,6,4,1,1,3,4,3,1,3,6,4,3,2,3,2,1,6,3,5,2},
			{2,6,1,1,5,3,6,1,5,2,3,1,3,1,3,5,4,6,3,4,6,6,5,3,4,3,1,2,1,3,1,5},
			{5,4,3,6,3,4,1,5,4,2,1,1,1,6,5,4,6,6,3,10,6,3,1,2,6,5,2,3,6,2,4,2},
			{2,1,2,1,6,2,3,4,3,6,3,6,1,3,4,1,1,5,1,2,1,2,5,1,3,4,6,2,6,2,4,3},
			{4,3,1,6,6,6,3,6,3,6,6,6,6,1,3,1,4,3,6,6,2,1,3,1,3,6,6,4,6,6,3,1},//20
			{3,6,6,6,6,3,2,1,2,3,6,1,6,6,1,3,6,6,1,1,6,6,3,2,6,3,6,6,6,3,1,3},
			{2,5,4,6,2,6,3,1,2,3,6,4,3,6,6,2,6,4,9,6,6,4,2,3,6,3,1,2,1,6,1,2},
			{2,4,3,6,4,2,1,3,1,1,4,2,1,1,2,6,3,5,2,6,2,6,6,8,6,5,6,4,6,1,1,1},
			{3,6,3,6,2,1,6,6,4,3,1,2,1,4,5,6,6,3,6,3,4,6,1,1,6,3,6,3,6,3,4,3},
			{4,3,6,3,1,6,4,1,9,3,6,1,1,3,1,6,2,1,1,6,6,3,1,1,1,5,1,2,3,2,11,1},//25
			{5,6,3,1,1,6,3,5,6,2,6,3,6,6,4,5,6,6,3,1,1,3,1,2,3,1,2,1,5,1,1,1},
			{2,1,1,4,6,2,3,1,6,6,6,2,3,8,2,3,4,5,3,6,1,4,5,1,1,5,6,6,6,4,6,6},
			{2,1,6,1,6,6,6,4,10,1,1,4,6,3,1,3,1,3,4,6,6,3,1,1,3,6,2,1,2,1,3,1},
			{3,4,6,3,2,5,2,3,6,3,1,6,4,1,2,3,1,2,3,6,6,4,3,5,3,1,2,1,6,1,1,4},
			{3,1,6,3,2,5,2,3,2,6,5,6,5,2,1,3,6,3,6,6,6,3,6,3,6,4,6,1,4,1,6,3},//30
			{2,1,6,4,2,6,4,3,2,3,4,6,3,4,3,2,1,3,4,3,2,6,3,4,6,6,4,6,5,2,1,4},
			{6,2,6,3,6,6,6,3,6,4,6,6,5,6,3,1,6,5,2,3,6,4,6,6,6,3,2,3,1,1,2,1}
	};

void WorldMapBase::first_terrain_init()
{
	//init land
	int count = 0;
	for(int y = 0;y<20;y++)
	{
		for(int x = 0;x< 20;x++)
		{
			for(int i = 0;i< 32;i++)
			{
				for(int j = 0;j< 32;j++)
				{
					/*if(i == 5 && j == 1)
					{
						printf("\ndddddddddddddddddddddd %d\n  ",minBlock[i][j]);
					}*/
					world_position[(y*32 + i)][(x*32 + j)] = minBlock[i][j];
					++count;
				}
			}
		}
	}
}

void WorldMapBase::check_remaining_position()
{
	for(int i = 0;i<640;i++)
	{
		for(int j = 0;j< 640;j++)
		{
			unsigned char result = world_position[i][j];
			if(result == 6)
			{
				AddNewPositionToMap(j,i);
			}
		}
	}
}
