/*
 * redis_word_map.h
 *
 *  Created on: Nov 15, 2013
 *      Author: zhongbing
 */

#ifndef REDIS_WORD_MAP_H_
#define REDIS_WORD_MAP_H_
#include <stdio.h>
#include <redis_base.h>
#include <map>
#include <msgpack_struct.h>

#include <RandomData.hpp>
#include <vector>
class RedisWordMap
{
private:

	RedisBase* redis;

	RandomData<PlayerSimpleInfo>  simple_player_data;

	friend class WorldMapBase;//worldmap,

	void word_map_player_init(unsigned char world_position[640][640],NearByPlayerInfoResponse  player_info[6400]);

	void add_new_player(short x,short y,unsigned int UUID);

	bool get_player_worldinfo(short& x,short& y ,unsigned int UUID);

	void save_player_worldinfo(short x,short y,unsigned int UUID);

	void get_palyer_last(short& x,short& y,unsigned int UUID);

	void get_player_att(WorldMapPlayerInfoResponse & p,unsigned int UUID);

	bool get_random_player(vector<PlayerSimpleInfo>& PlayerSimpleInfo,int n);

	void add_random_player(PlayerSimpleInfo & PlayerSimpleInfo);

	RedisWordMap();

	RedisWordMap(const RedisWordMap& r){

		this->redis = r.redis;

	}



public:
	virtual ~RedisWordMap();

};
#endif /* REDIS_WORD_MAP_H_ */
