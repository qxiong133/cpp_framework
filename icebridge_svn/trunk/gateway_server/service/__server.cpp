#include "sdk/tcp_io_thread_manager.h"
#include "sdk/net_message.h"
#include "sdk/AESSBox.h"
#include <RandomUtil.h>
#include "_login_worker.h"
#include "sdk/my_filter.h"
#include <iostream>
#include "_global_function.h"
#include <mysql_game_configbuff.h>
#include <pthread.h>
#include <mysql_base.h>
#include <boost/container/vector.hpp>
#include <logic_server/mdcliapi2.hpp>
#include "_chat_worker.h"
#include <iostream>
#include <boost/container/map.hpp>
#include <cstdio>
#include <exception>
// sync interval

TCPIOThreadManager manager(5);
extern MysqlBase* mysql_main_thread;
RedisBase* redis_main_thread;

long volatile GlobalTime = 0;

mdcli * logic_server = mdcli::getInstance("client01",1);

Broker broker("broker01", "tcp://localhost:5555");

extern boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;

void handler_in_gateway(const char * data, size_t size)
{
	//std::cout<<"enter handler success!"<<std::endl;
	short usercount = *((short*)data);
	int offset = sizeof(short) + sizeof(uint32_t) * usercount;
	size -= offset;
	if(size < 0 || size > 10240000)
	{
		//printf("erro data and data`s address is %p and size is %d\n",data,size);
	}
	else
	{
		//printf("data addres is %p,size is %d\n",(const char*)(data + offset),size);

		eddy::NetMessage net_message;

		net_message.Write((const char*)(data + offset),size);

		for(int i = 0;i< usercount;i++)
		{
			 uint32_t uuid = *((uint32_t*)(data +2+ i*4));
			 if(UuidHandlerPoitiner[uuid].get()!=NULL)
			 {
				 UuidHandlerPoitiner[uuid]->Send(net_message);
				 //printf("logis to gateway is success!\n");
			 }
			 //printf("uuid is %d\n",uuid);
		}
	}
}

void init_data() {

	mysql_main_thread = MysqlBase::get_instance(5);

	redis_main_thread =  RedisBase::get_instance();

	logic_server->add_broker(broker);
	logic_server->connect_to_brokers();
	logic_server->start_recv(logic_server);
    std::string a("worldmap");
    zmsg * msg= new zmsg("aa");
	logic_server->send(a,msg);

	InitGlobalData();

}

void initBeforeStart() {
	//regist worker
	LoginWorker::register_worker(false,"worldmap",NULL,WorldMapNearByRequest_ID,LeaveWordMapRequest_ID);
	LoginWorker::register_worker(false,"logic",NULL,SoldierInfoRequest_ID,CompleteAtOnceRequest_ID);
	LoginWorker::register_worker(false,"pvpServer",NULL,msgStartAttack_ID,msgAtkerStopBattleRequest_ID);
	LoginWorker::register_worker(false,"python_service",NULL,SearchGroupRequest_ID,GetTop50UserResquest_ID);
	//init global data;
	init_data();

	//generate lots of randoms for later use
	RandomUtil::GenerateLotsOfRandom();

	//generate the original box and this box can be specified by the argument
	AESSBox::GenerateOriginalBoxe(/*default is 0x0100*/);

	ChatWorker& chat = ChatWorker::GetInstance();
	//register message_request and service

	//test();
}
int main(int argc, char** argv) {

	initBeforeStart();
	unsigned short int port = 20000;

	if (argc == 2)
		port = boost::lexical_cast<unsigned short int>(argv[1]);

	TCPServer server(
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),
			manager, &LoginWorker::Create, &MyFilter::Create);
	try {
		manager.Run();
	}
	catch (std::exception& e) {
		std::cout << e.what() << "please write message" << std::endl;
	}
	return 0;
}

