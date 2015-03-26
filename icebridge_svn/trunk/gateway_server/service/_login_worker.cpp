#include "_login_worker.h"
#include "service/_global_function.h"
#include <logic_server/mdcliapi2.hpp>
#include <stdint.h>
#include <msgpack.hpp>
#include <time.h>
#include <boost/bind.hpp>
#include <service/_global_macro.h>
#include <boost/container/map.hpp>
#include <boost/foreach.hpp>
#include <service/_chat_worker.h>

extern mdcli* logic_server;
MysqlLoginWorker LoginWorker::mysql;
RedisChatWorker LoginWorker::redis;

boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;

LoginWorker::LoginWorker(){

	offline_handler = NULL;

	is_client_off = true;

	login_status = false;

	last_time = 0;

	is_get_random_name = false;

	player_UUID = 0;

	memset(backpack_status,0,64);

	for(int i = 0;i < 6;i++)
	{
		my_hero[i] = NULL;
	}
	for(int i = 0;i < 3;i++)
	{
		rand_hero[i] = NULL;
	}
}
WorkerStatus LoginWorker::worker_status[MAX_REQ_NUM] = {};

void LoginWorker::register_worker(bool is_local,string workername,WorkerProcess process,RequestID start_id,RequestID end_id)
{
	if((int)start_id > (int)end_id ||start_id <= ServerChallengeRequest_ID
			|| end_id > (ServerChallengeRequest_ID + MAX_REQ_NUM))
	{
		printf("id is error id");
	}
	else
	{
		for(int i = start_id - ServerChallengeRequest_ID;i<=end_id - ServerChallengeRequest_ID;i++)
		{
			worker_status[i].is_local = is_local;
			worker_status[i].name = workername;
			worker_status[i].process = process;
		}
	}
}

void LoginWorker::OnConnect(){

	/*
	ServerChallengeRequest_ID = 1000,//1
	HeartBeatRequest_ID,//2
	ArmsConfigRequest_ID,//3
	BuildConfigRequest_ID,//4
	SkillConfigRequest_ID,//5
	RegisterRequest_ID,//6
	SetNameRequest_ID,//7
	BindEmailRequest_ID,//8
	AttackEnemyRequest_ID,//9
	BuildStatusListRequest_ID,//10
	BuildCreateRequest_ID,//11
	BuildUpgradeRequest_ID,//12
	GetMyAssetRequest_ID,//13
	LoginRequest_ID,//14
	GetRandNameRequest_ID,//15
	DressEquipRequest_ID,//16
	UnloadEquipRequest_ID,//17
	SaleGoodsRequest_ID,//18
	LookMarketGoodsRequest_ID,//19
	BuyMarketGoodsRequest_ID,//20
	OpenBoxRequest_ID,//21
	RecruitRequest_ID,//22
	GetRandHeroRequest_ID,//23
	DropHeroRequest_ID,//24
	GetEmailGoodsRequest_ID,//25
	EmailNextPageRequst_ID,//26
	FastFinishRequest_ID,//27
	UpgradeArmTypeRequest_ID,//28
	AppointHeroRequest_ID,//29
	CancelHeroRequest_ID,//30
	SetArmsToHeroRequest_ID,//31
	QuikAttackRequest_ID,//32
	LogoutRequest_ID,//33
	 * */
	service1[0] = &LoginWorker::ServerChallengeResponseService;
	service1[1] = &LoginWorker::HeartBeatRequestService;
	service1[2] = &LoginWorker::ArmsConfigRequestService;
	service1[3] = &LoginWorker::BuildConfigRequestService;
	service1[4] = &LoginWorker::SkillConfigRequestService;
	service1[5] = &LoginWorker::RegisterRequestService;
	service1[6] = &LoginWorker::SetNameRequestService;
	service1[7] = &LoginWorker::BindEmailRequestService;
	service1[8] = &LoginWorker::AttackEnemyRequestService;
	service1[9] = &LoginWorker::BuildStatusListRequestService;
	service1[10] = &LoginWorker::BuildCreateRequestService;
	service1[11] = &LoginWorker::BuildUpgradeRequestService;
	service1[12] = &LoginWorker::GetMyAssetRequestService;
	service1[13] = &LoginWorker::LoginRequestService;
	service1[14] = &LoginWorker::GetRandNameRequestService;
	service1[15] = &LoginWorker::DressEquipRequestService;
	service1[16] = &LoginWorker::UnloadEquipRequestService;
	service1[17] = &LoginWorker::SaleGoodsRequestService;
	service1[18] = &LoginWorker::LookMarketGoodsRequestService;
	service1[19] = &LoginWorker::BuyMarketGoodsRequestService;
	service1[20] = &LoginWorker::OpenBoxRequestService;
	service1[21] = &LoginWorker::RecruitRequestService;
	service1[22] = &LoginWorker::GetRandHeroRequestService;
	service1[23] = &LoginWorker::DropHeroRequestService;

	service1[24] = &LoginWorker::GetEmailGoodsRequestService;
	service1[25] = &LoginWorker::EmailNextPageRequstService;
	service1[26] = &LoginWorker::FastFinishRequestService;
	service1[27] = &LoginWorker::UpgradeArmTypeRequestService;
	service1[28] = &LoginWorker::AppointHeroRequestService;
	service1[29] = &LoginWorker::CancelHeroRequestService;
	service1[30] = &LoginWorker::SetArmsToHeroRequestService;
	service1[31] = &LoginWorker::QuikAttackRequestService;

	service1[32] = &LoginWorker::LogoutRequestService;
	ServerChallengeResponse scMessage;
	scMessage.RandomNumber =time(0)%100000;
	challenge_number = scMessage.RandomNumber + 1;
	SendMessage(scMessage,ServerChallengeResponse_ID);
//	SendMessage(scMessage,ServerChallengeResponse_ID);
}

void LoginWorker::OnMessage(NetMessage& message) {
		if(login_status)
		{
			last_time = time(0);
		}
		unsigned int index =  *((unsigned short *)(message.begin()))- ServerChallengeRequest_ID;
		if(index < 0 || index > MAX_REQ_NUM -1)return ;
		if(index >= 0 && index < LOGIN_SERVICE_NUM)
		{
			int size = message.size()-2;
			msgpack::object request;
			if(size)
			{
				try
				{
					msgpack::zone z;
					msgpack::unpack_return ret = msgpack::unpack(message.begin()+2,size, NULL, &z, &request);
					if(ret != msgpack::UNPACK_SUCCESS)//service.size())
					{
						std::cout<<"unpack error or out of array limit"<<std::endl;
						return ;
					}
					else
					{
						(this->*service1[index])(&request);
					}
				}
				catch(...)
				{
					Close();
				}
			}
		}
		else if(worker_status[index].is_local && login_status)//send logic server
		{
			if(worker_status[index].process)
			{
				worker_status[index].process(message.begin(),message.size(),player_UUID);
			}
		}
		else
		{
		/*	if(index == msgCurBattleInfoRequest_ID - ServerChallengeRequest_ID)//test msg
			{
				int size = message.size()-2;
				msgpack::object request;
							if(size)
							{
								try
								{
									msgpack::zone z;
									msgpack::unpack_return ret = msgpack::unpack(message.begin()+2,size, NULL, &z, &request);
									if(ret != msgpack::UNPACK_SUCCESS)//service.size())
									{
										std::cout<<"unpack error or out of array limit"<<std::endl;
										return ;
									}
									else
									{
										msgCurBattleInfoRequest req;
										MsgpackConvert(&req,request);
										printf("unpack success");
									}
								}
								catch(...)
								{

								}
			}
			}*/
			if(login_status == false)return;
			std::vector<char> buff;
			typedef boost::uint16_t Header;
			Header header = message.size() + sizeof(player_UUID);
			buff.insert(buff.end(), reinterpret_cast<const char*>(&header),
																		reinterpret_cast<const char*>(&header) + sizeof(Header));
			buff.insert(buff.end(), reinterpret_cast<const char*>(&player_UUID),
																		reinterpret_cast<const char*>(&player_UUID) + sizeof(player_UUID));
			buff.insert(buff.end(),message.begin(),message.begin() + message.size());

			zmsg * request = new zmsg(buff.data(),buff.size());
			if(worker_status[index].name != "" && worker_status[index].name != " ")
			{
				logic_server->send(worker_status[index].name,request);
			}
		}
}

void OffLineNotifyWorker(unsigned int uuid,int player_index)
{

	static ChatWorker& chat = ChatWorker::GetInstance();
	chat.OfflineNotify(uuid,player_index);

	std::vector<char> buff;
	typedef boost::uint16_t Header;

	short msgid = LogoutRequest_ID;
	Header header = sizeof(msgid) + sizeof(uuid);
	buff.insert(buff.end(), reinterpret_cast<const char*>(&header),
																reinterpret_cast<const char*>(&header) + sizeof(Header));
	buff.insert(buff.end(), reinterpret_cast<const char*>(&uuid),
																reinterpret_cast<const char*>(&uuid) + sizeof(uuid));
	buff.insert(buff.end(), reinterpret_cast<const char*>(&msgid),
																reinterpret_cast<const char*>(&msgid) + sizeof(msgid));
	zmsg * request_zmsg = new zmsg(buff.data(),buff.size());
	string str = "logic";
	logic_server->send(str,request_zmsg);
	zmsg * request_zmsg2 = new zmsg(buff.data(),buff.size());
	str = "worldmap";
	logic_server->send(str,request_zmsg2);

	zmsg * request_zmsg3 = new zmsg(buff.data(),buff.size());
	str = "pvpServer";
	logic_server->send(str,request_zmsg3);
}

void LoginWorker::OnClose() {
	if(login_status == false && is_get_random_name == true)
	{
		redis.add_random_name(last_rand_name,is_man);
	}
	cout << session_id() << ":closed" << endl;
	if(UuidHandlerPoitiner.find(player_UUID) != UuidHandlerPoitiner.end())
	{
		OffLineNotifyWorker(player_UUID,my_attribute.player_index);
		if(is_client_off)
		{
			UuidHandlerPoitiner.erase(player_UUID);
			printf("\nClient off \n");
		}
	}

	for(int i= 0;i< 6;i++)
	{
		if(my_hero[i])
		{
			delete my_hero[i];
		}
	}
	for(int i = 0;i< 3;i++)
	{
		if(rand_hero[i])
		{
			delete rand_hero[i];
		}
	}
	typedef const map<string,DeadlineTimer*>::value_type const_pair1;
	BOOST_FOREACH(const_pair1& value,building_timer)
	{
		if(value.second)
		{
			delete value.second;
		}
	}
	building_timer.clear();
	if(offline_handler != NULL)
	{
		offline_handler();
		offline_handler = NULL;
	}
}
void LoginWorker::SendSbuffer(MyBuffer& buffer,ResponseID msgID)
{
	NetMessage net_message;
	short msgid = msgID;
	net_message.Write(reinterpret_cast<const char*>(&msgid),sizeof(msgid));
	net_message.Write(buffer.data(),buffer.size());

	Send(net_message);
}

