/*
 * my_handler.h
 *
 *  Created on: Oct 14, 2013
 *      Author: root
 */

#ifndef MY_HANDLER_H_
#define MY_HANDLER_H_
#pragma once
#include    <boost/lexical_cast.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/iostreams/stream.hpp>
#include    <boost/static_assert.hpp>
#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_server.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/container_device.h"
#include    "sdk/net_message_filter_interface.h"
#include    "sdk/net_message.h"
#include  <RandomUtil.h>
#include <msgpack_id.h>
#include <msgpack_struct.h>
#include <boost/function.hpp>
#include <vector>
#include <iostream>
#include <msgpack.hpp>
#include <database/mysql_login_worker.h>
#include <database/redis_chat_worker.h>
#include <mysql_game_configbuff.h>
#include <timer.h>
#include <msgpack_convert.hpp>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

//#define DEBUG_MODE 2

using namespace eddy;
using namespace std;
using namespace boost;
using namespace boost::iostreams;
using namespace msgpack;

#define LOGIN_SERVICE_NUM 33

extern TCPIOThreadManager manager;
typedef void (*WorkerProcess)(const char *, size_t,unsigned int);
typedef  struct worker_status_
{
	bool is_local;
	string name;
	WorkerProcess process;
}WorkerStatus;

class LoginWorker: public TCPSessionHandler
{
private:
	static WorkerStatus worker_status[MAX_REQ_NUM];

	typedef void (LoginWorker::*Service1)(object *) ;

	static MysqlLoginWorker mysql;

	static RedisChatWorker redis;

	static Timer*  heart_timer;

	bool login_status;

public:

	int challenge_number;

	unsigned int player_UUID;

	long last_time;

public:

	static void register_worker(bool is_local,string workername,WorkerProcess process,RequestID start_id,RequestID end_id);

	typedef ContainerDevice<NetMessage> NetMessageDevice;

	static shared_ptr<LoginWorker> Create() {
		return make_shared<LoginWorker>();
	}

	/*
	 * this is a new connection
	 **/

	static void Init();

	LoginWorker();


	virtual void OnConnect();

	virtual void OnMessage(NetMessage& message);

	bool is_client_off;

	boost::function<void ()>    offline_handler;

	virtual void OnClose();

	void SendSbuffer(MyBuffer& buffer,ResponseID msgID);

	template<class MessageType>
	static void SendMessageToOther(const MessageType& message, ResponseID msgID,TCPSessionID sID)
	{
		NetMessage net_message;
		short msgid = msgID;
		net_message.Write(reinterpret_cast<const char*>(&msgid),sizeof(msgid));
		msgpack::sbuffer m_sbuf;
		msgpack::pack(m_sbuf,message);
		net_message.Write(m_sbuf.data(),m_sbuf.size());
		TCPIOThreadManager::SessionHandlerPointer pHandler = manager.GetSessionHandler(sID);
		if(pHandler.get() != NULL){
			pHandler->Send(message);
		}
	}
	template<class MessageType>
	void SendMessage(const MessageType& message, ResponseID msgID) {
		NetMessage net_message;
		short msgid = msgID;
		net_message.Write(reinterpret_cast<const char*>(&msgid),sizeof(msgid));
		msgpack::sbuffer m_sbuf;
		msgpack::pack(m_sbuf,message);
		net_message.Write(m_sbuf.data(),m_sbuf.size());
		Send(net_message);
	}


public :// message service relate;

	static Poco::UUIDGenerator& gen;

	Service1 service1[LOGIN_SERVICE_NUM];

	void login_handle(int diamond,bool handlemodel = true);

	void login_handle2(int diamond);

	void init_new_player(string name,int race);

	void ServerChallengeResponseService(object *request);//1

	void HeartBeatRequestService(object *request);//2

	void ArmsConfigRequestService(object *request);//3

	void BuildConfigRequestService(object *request);//4

	void SkillConfigRequestService(object *request);

	void RegisterRequestService(object *request);//5

	void SetNameRequestService(object *request);//6

	void BindEmailRequestService(object *request);//7

	string last_rand_name;

	bool is_man;

	bool is_get_random_name;

	void GetRandNameRequestService(object *request);//

	void LoginRequestService(object *request);// end -1

	void LogoutRequestService(object *request);// login worker end

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
public :

	MyAttruibuteResponse my_attribute;

	map<string,DeadlineTimer*>  building_timer;

	list<string> building;//init building todo

	void build_timer_handle(string build_sn);

	void en_timer(string build_sn);
//msg handle
	void BuildStatusListRequestService(object *request);//8 end

	void BuildUpgradeRequestService(object *request);//9 end if asset build shouqu resource

	void BuildCreateRequestService(object *request);//10 end

	void GetMyAssetRequestService(object *request);//11  end

	void DressEquipRequestService(object *request);//11  end

	void UnloadEquipRequestService(object *request);//11  end

	void SaleGoodsRequestService(object *request);//11  end

	void BuyMarketGoodsRequestService(object *request);

	void LookMarketGoodsRequestService(object *request);//11  end

	EquipListResponse myequiplist;

	HeroListResponse myherolist;

	bool backpack_status[64];
	EquipResponse my_backpack[64];

	HeroResponse* my_hero[6];

	HeroResponse* rand_hero[3];

	map<string,vector<EquipResponse> > hero_equip;

	EmailSaleListResponse email_sale_list;

	void HeroRecaculate(HeroResponse& h);
	int find_hero_equip_sn(HeroResponse& hero);
	int find_hero_position();

	int GetBackpack();

	void OpenBoxRequestService(object *request);

	void refresh_random_hero(int randomtype);

	void RecruitRequestService(object *request);

	void GetRandHeroRequestService(object *request);

	void DropHeroRequestService(object *request);

	void GetEmailGoodsRequestService(object *request);

	void EmailNextPageRequstService(object *request);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////CancelHeroRequest////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
public://pvp

	void FastFinishRequestService(object *request);

	int get_combatunion_level();

	void UpgradeArmTypeRequestService(object *request);

	void send_result(int result,int msg_id);

	int get_hero_index(string hero_uuid);

	void AppointHeroRequestService(object *request);

	void CancelHeroRequestService(object *request);

	void SetArmsToHeroRequestService(object *request);

	void QuikAttackRequestService(object *request);

	void AttackEnemyRequestService(object *request);

static void PushNotify(MarketGoodsResponse& value);

};
#endif /* MY_HANDLER_H_ */
