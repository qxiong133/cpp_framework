
#ifndef SYSTEM_SMALL_MAP_H_
#define SYSTEM_SMALL_MAP_H_

#include <boost/container/vector.hpp>
#include <boost/container/map.hpp>
#include <msgpack.hpp>
#include <base_function.hpp>
#include <msgpack_struct.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/container/map.hpp>
#include <string.h>

#include <mysql_base.h>
#include "RedisClient.h"
#include "MysqlManager.h"
//#include "GameLog.h"

#include "_army_build_queue_manager.h"

#define MAX_SERVICE_NUM 10


using namespace msgpack;

class SmallMapBase
{

    typedef void (SmallMapBase::*Service)(object&,unsigned int);
    friend class ArmysBuildQueue;
    friend class ArmysBuildQueueManager;

protected:

	//modual object
	RedisClient* redis;

    //MysqlBase* mysql_main_thread;

	Service service[MAX_SERVICE_NUM];

	boost::asio::io_service io;

	boost::asio::io_service::work*  work_;

    boost::container::vector< vector<ArmsResponse> > &_arms;

    //boost::container::vector< vector<TowerResponse> > &_tower;

    //boost::container::vector< vector<BuildResponse> > &_build;

    ArmysBuildQueueManager* _armys_manager;
    
    int services_len;

public ://important

	WorkerSender * sender;

	mdwrk* session;

    MysqlManager *poco_mysql;

	void dispatch_func(int index,object& requst,unsigned int player_uuid);

	static SmallMapBase& GetInstance();

	boost::asio::io_service& get_io_service();

	void run();

private://private function

	SmallMapBase();

	SmallMapBase(const SmallMapBase&){}

	void register_all_service();

	~SmallMapBase();

    void ReadArms();

    void ReadTower();

    void ReadBuild();

private :// message service relate;

	void test(object& request,unsigned int player_uuid);
	void login(object& request,unsigned int player_uuid);
	void logout(object& request,unsigned int player_uuid);
	void processSoldiersRequest(object& request,unsigned int player_uuid);
    void changeSoldierCampRequest(object& request,unsigned int player_uuid);
    void completeAtOnceRequest(object& request,unsigned int player_uuid);
	//void getCampInfoRequest(object& request,unsigned int player_uuid);

};

#endif /* SYSTEM_SMALL_MAP_H_ */
