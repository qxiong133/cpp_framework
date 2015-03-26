#include "_world_map.h"
#include <stdlib.h>
#include <redis_word_map.h>
#include <time.h>
#include <boost/foreach.hpp>
#include <msgpack_struct.h>

extern NearByResourceResponse  resource[6400];
extern NearByPlayerInfoResponse  player_info[6400];
extern NearByMapMonsterResponse  monster[6400];
extern unsigned char world_position[640][640];
void WorldMapBase::check_remain_coord(unsigned char checktype,int big_area_num,vector<coord>  remain_coord[100])
{
	//4 3   400
	coord c;
	for(int k = 0;k < 100;k++)
	{
		remain_coord[k].clear();

		int area_x = (big_area_num * 100 + k)%80;
		int area_y = (big_area_num * 100 + k)/80;

		for(int j = area_y * 8;j<  area_y*8 + 8;j++)
		{
			for(int i = area_x*8;i< area_x*8 + 8;i++)
			{
				if(world_position[j][i] == checktype)
				{
					c.x = i;
					c.y = j;
					remain_coord[k].push_back(c);
				}
			}
		}
	}

	//test
/*	for(unsigned int i = 0;i<remain_coor.size();i++)
	{
		printf("coord x= %d,coord y = %d\n",remain_coor[i].x,remain_coor[i].y);
	}*/
}

void WorldMapBase::update_wood(int i,vector<coord>  resource_coord[100],long cur_time)
{
	WorldMapResourceResponse rres;
	int coordx,coordy;
	for(int coord_index = 0;coord_index < 100;coord_index++)
	{
		bool is_update = false;
		int area = i*100 + coord_index;
		resource[area].area = area;
		//printf("area is %d\n",area);
		for(unsigned int j = 0;j< resource_coord[coord_index].size();j++)
		{
			if(0 == RandomUtil::GetARandom()%2)
			{
				coordx = resource_coord[coord_index][j].x << 16;
				coordy = resource_coord[coord_index][j].y;
				world_position[resource_coord[coord_index][j].x][resource_coord[coord_index][j].y] = 0xff;
				rres.resource_type = 3;
				resource[area].resource[(coordx + coordy)] = rres;
				//printf("resource[area].area %d ,x %d,y %d\n",resource[area].area,resource_coord[coord_index][j].x,resource_coord[coord_index][j].y);
				is_update = true;
			}
		}
		if(is_update)
		{
			//printf("area_list[4].size is \n");
			last_update[area] = cur_time;
		}
	}

}
void WorldMapBase::update_stone(int i,vector<coord>  resource_coord[100],long cur_time)
{

	WorldMapResourceResponse rres;
	int coordx,coordy;
	for(int coord_index = 0;coord_index < 100;coord_index++)
	{
		bool is_update = false;
		int area = i*100 + coord_index;
		resource[area].area = area;
	//	printf("area is %d\n",area);
		for(unsigned int j = 0;j< resource_coord[coord_index].size();j++)
		{
			if(0 == RandomUtil::GetARandom()%2)
			{
				coordx = resource_coord[coord_index][j].x << 16;
				coordy = resource_coord[coord_index][j].y;
				world_position[resource_coord[coord_index][j].x][resource_coord[coord_index][j].y] = 0xff;
				rres.resource_type = 4;
				resource[area].resource[(coordx + coordy)] = rres;
				//printf("resource[area].area %d ,x %d,y %d\n",resource[area].area,resource_coord[coord_index][j].x,resource_coord[coord_index][j].y);
				is_update = true;
			}
		}
		if(is_update)
		{
			//printf("area_list[4].size is \n");
			last_update[area] = cur_time;
		}
	}
}

void WorldMapBase::update_iron(int i,vector<coord>  resource_coord[100],long cur_time)
{

	WorldMapResourceResponse rres;
	int coordx,coordy;
	for(int coord_index = 0;coord_index < 100;coord_index++)
	{
		bool is_update = false;
		int area = i*100 + coord_index;
		resource[area].area = area;
		for(unsigned int j = 0;j< resource_coord[coord_index].size();j++)
		{
			//if(0 == RandomUtil::GetARandom()%2)
			{
				coordx = resource_coord[coord_index][j].x << 16;
				coordy = resource_coord[coord_index][j].y;
				world_position[resource_coord[coord_index][j].x][resource_coord[coord_index][j].y] = 0xff;
				rres.resource_type = 5;
				resource[area].resource[(coordx + coordy)] = rres;
				//printf("resource[area].area %d ,x %d,y %d\n",resource[area].area,resource_coord[coord_index][j].x,resource_coord[coord_index][j].y);
				is_update = true;
			}
		}
		if(is_update)
		{
			//printf("area_list[4].size is \n");
			last_update[area] = cur_time;
		}
	}

}
void WorldMapBase::update_resource()
{//64  10*10 update
	//printf("enter update_resource2\n");

	static vector<coord>  resource_coord[100];
	static int i =  -1;
	if( ++i == 64) i = 0;
	long update_time = time(0);
	check_remain_coord(3,i,resource_coord);
	update_wood(i,resource_coord,update_time);

	check_remain_coord(4,i,resource_coord);
	update_stone(i,resource_coord,update_time);

	check_remain_coord(5,i,resource_coord);
	update_iron(i,resource_coord,update_time);

	check_remain_coord(2,i,resource_coord);
	update_monstor(i,resource_coord,update_time);

/*
	for(int coord_index = 0;coord_index < 100;coord_index++)
	{
		bool is_update = false;
		int area = i*100 + coord_index;
		if(last_update[area] == update_time)
		{
			sender->SendToPlayerList(resource[area],NearByResourceResponse_ID,&all_map_player_uuid);
		}
	}*/
//	sender->SendToPlayerList(resource[area],NearByResourceResponse_ID,&all_map_player_uuid);
}

void WorldMapBase::update_monstor(int i,vector<coord>  resource_coord[100],long cur_time)
{
	WorldMapMonsterResponse rres;
	int coordx,coordy;
	for(int coord_index = 0;coord_index < 100;coord_index++)
	{
		bool is_update = false;
		int area = i*100 + coord_index;
		monster[area].area = area;
		for(unsigned int j = 0;j< resource_coord[coord_index].size();j++)
		{
			//if(0 == RandomUtil::GetARandom()%2)
			{
				coordx = resource_coord[coord_index][j].x << 16;
				coordy = resource_coord[coord_index][j].y;
				world_position[resource_coord[coord_index][j].x][resource_coord[coord_index][j].y] = 0xff;
				//rres.resource_type = 5;
				monster[area].monster[(coordx + coordy)] = rres;
				//printf("resource[area].area %d ,x %d,y %d\n",resource[area].area,resource_coord[coord_index][j].x,resource_coord[coord_index][j].y);
				is_update = true;
			}
		}
		if(is_update)
		{
			//printf("area_list[4].size is \n");
			last_update[area] = cur_time;
		}
	}
}
