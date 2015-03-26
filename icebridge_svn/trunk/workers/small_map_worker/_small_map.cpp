#include "_small_map.h"
#include <stdlib.h>

#include "GameConfig.h"
#include "msgpack_convert.hpp"

extern boost::container::vector< vector<ArmsResponse> > global_arms;

extern MysqlBase* mysql_main_thread ;


void process(const char * data,int size){
    static SmallMapBase& smallmap = SmallMapBase::GetInstance();

    cout << "process function " ; 

    int player_uuid = *((int*)(data+2));
    short message_id = *((short*)(data + 6));
    msgpack::zone z;
    msgpack::object request;
    if(message_id == LoginRequest_ID){

        cout << "enter login";
        smallmap.dispatch_func(0,request,player_uuid);
 	if(data)
  	{
		free((void*)data);
 	}
        return;
    }

    if(message_id == LogoutRequest_ID){
        cout << "enter logout";
        smallmap.dispatch_func(1,request,player_uuid);
	if(data)
	{
		free((void*)data);
	}
        return;
    }

    msgpack::unpack_return ret = msgpack::unpack((const char*)(data + 8), size-8, NULL, &z, &request);
	if(ret == msgpack::UNPACK_SUCCESS)
	{
		smallmap.dispatch_func(message_id - SoldierInfoRequest_ID + 2,request,player_uuid);
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
   static SmallMapBase& smallmap = SmallMapBase::GetInstance();
    char *  p_data = (char *)malloc(size);
    memcpy(p_data,data,size);
    smallmap.get_io_service().post(boost::bind(process,p_data, size));
}


SmallMapBase& SmallMapBase::GetInstance()
{
	static SmallMapBase wordbase;
	return wordbase;
}



SmallMapBase::SmallMapBase():_arms(global_arms)
{
	work_ = new boost::asio::io_service::work(io);
        
    poco_mysql = new MysqlManager();
    
	register_all_service();

    printf("redis ip is %s\n",GameConfig::getInstance().getString("redisMaster.ip").c_str());
    printf("redis port is %d\n", GameConfig::getInstance().getInt("redisMaster.port"));

	session = mdwrk::getInstance("smallmap_worker01", "tcp://localhost:5555", "logic", 1,handler_in_worker);

	sender = new WorkerSender(session);

	redis = new RedisClient(GameConfig::getInstance().getString("redisMaster.ip"),GameConfig::getInstance().getInt("redisMaster.port"));
	//redis = new RedisClient();

    //mysql_main_thread = MysqlBase::get_instance(1);

    std::cout << "init data begin" << std::endl;
    InitGlobalData();
    std::cout << "init data end" << std::endl;
	printf("start SmallMapBase::SmallMapBase\n");
    std::cout << "army manager begin" << std::endl;
    _armys_manager = new ArmysBuildQueueManager(this);
    std::cout << "army manager end" << std::endl;
	printf("start ArmysBuildQueueManager\n");

/*
    shared_ptr<SoldierInfoRequest> tmp(new SoldierInfoRequest());
    tmp->soldier_type = 12;
    tmp->soldier_num = 4;
    tmp->soldier_level = 2;
    tmp->which_camp = 1;
    tmp->operation = ADD;
    _armys_manager->requestProcess(123, tmp);
    tmp->soldier_num = 3;
    tmp->operation = DEL;
    _armys_manager->requestProcess(123, tmp);

    _armys_manager->uidOffline(123);
*/

}

SmallMapBase::~SmallMapBase()
{
    if(session)
      delete session;

    if(redis)
      delete redis;

    if(sender)
      delete sender;

    if(work_)
      delete work_;

    if(poco_mysql)
      delete poco_mysql;
}

void SmallMapBase::register_all_service()
{
	service[0] = &SmallMapBase::login;//8
	service[1] = &SmallMapBase::logout;//8
	service[2] = &SmallMapBase::processSoldiersRequest;//8
	service[3] = &SmallMapBase::changeSoldierCampRequest;//8
	service[4] = &SmallMapBase::completeAtOnceRequest;//8
    services_len = 4;
}

void SmallMapBase::dispatch_func(int index,object& requst,unsigned int player_uuid)
{
	if(index < 0 || index > services_len)
	{
		printf("erro messege pack,and index is %d\n",index);
		return ;
	}
	(this->*service[index])(requst,player_uuid);
}

boost::asio::io_service& SmallMapBase::get_io_service()
{
	return io;
}

void SmallMapBase::run()
{
	boost::system::error_code error;
	io.run(error);
	if (error)
		std::cerr << error.message() << std::endl;
}


void SmallMapBase::test(object& request, unsigned int player_uuid)
{

    std::cout << "test" << std::endl;
}

void SmallMapBase::processSoldiersRequest(object& request,unsigned int player_uuid){

    cout << "processSoldiersRequest convert" ;
    SoldierInfoRequest req;
    //request.convert(&req);
    MsgpackConvert(&req, request);


    cout << "processSoldiersRequest convert end" ;

    shared_ptr<SoldierInfoRequest> tmp(new SoldierInfoRequest());
    tmp->soldier_type = req.soldier_type;
    tmp->soldier_level = req.soldier_level;
    tmp->soldier_num = req.soldier_num;
    tmp->which_camp = req.which_camp;
    tmp->operation = req.operation;
    cout << "in SmallMapBase::processSoldiersRequest we recevied type" << tmp->soldier_type ;
    cout << "in SmallMapBase::processSoldiersRequest we recevied level" << tmp->soldier_level;
    cout << "in SmallMapBase::processSoldiersRequest we recevied num" << tmp->soldier_num;
    cout << "in SmallMapBase::processSoldiersRequest we recevied which_camp" << tmp->which_camp;
    cout << "in SmallMapBase::processSoldiersRequest we recevied operation" << tmp->operation;
    _armys_manager->requestProcess(player_uuid, tmp);
}

void SmallMapBase::login(object& request,unsigned int player_uuid){
   _armys_manager->uidOnline(player_uuid);

}

void SmallMapBase::logout(object& request,unsigned int player_uuid){
   _armys_manager->uidOffline(player_uuid);
}

void SmallMapBase::changeSoldierCampRequest(object& request,unsigned int player_uuid){

    ChangeSoldierCampRequest req;
    //request.convert(&req);
    MsgpackConvert(&req, request);
    _armys_manager->changeSoldierCampRequest(player_uuid, &req);

}

void SmallMapBase::completeAtOnceRequest(object& request,unsigned int player_uuid){
    CompleteAtOnceRequest req;
    //request.convert(&req);
    MsgpackConvert(&req, request);
    _armys_manager->completeAtOnceRequest(player_uuid, &req);
}
 


