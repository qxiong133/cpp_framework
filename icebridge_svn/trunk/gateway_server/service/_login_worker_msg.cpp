#include "_login_worker.h"
#include <boost/container/map.hpp>
#include <boost/foreach.hpp>
#include <service/_global_macro.h>
#include <logic_server/mdcliapi2.hpp>
#include <logic_server/zmsg.hpp>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>
#include <boost/foreach.hpp>
#include <list>
#include	<string>
#include <msgpack_convert.hpp>

extern ConfigBuffer config_buffer;
extern VersionInforResponse game_version;
extern mdcli* logic_server;
extern TCPIOThreadManager manager;
extern boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;

extern boost::container::vector< vector<ArmsResponse> > global_arms;

using namespace std;

void LoginWorker::ServerChallengeResponseService(object *request)
{
	ServerChallengeRequest req;
	MsgpackConvert(&req,*request);
	if(req.RandomNumber == challenge_number)
	{
		std::cout<<"++++++++++++++++++++++++++++++++connect Sucess! session id is :"<<session_id()<<std::endl;
		SendMessage(game_version,VersionInforResponse_ID);
	}
	else
	{
		std::cout<<"connect Failed!"<<std::endl;
		Close();
	}
}

void LoginWorker::HeartBeatRequestService(object *request)
{
	last_time = time(0);
}

#include <service/_chat_worker.h>
void OnlineNotifyWorker(unsigned int uuid,int& player_index)
{
	static ChatWorker& chat = ChatWorker::GetInstance();
	player_index = chat.OnlineNotify(uuid);

	std::vector<char> buff;
	typedef boost::uint16_t Header;

	short msgid = LoginRequest_ID;
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
	str = "pvpServer";
	logic_server->send(str,request_zmsg2);
}
void LoginWorker::login_handle2(int diamond)
{
		UuidHandlerPoitiner[player_UUID] = boost::dynamic_pointer_cast<LoginWorker>(shared_from_this());
		OnlineNotifyWorker(player_UUID,my_attribute.player_index);
		printf("\n++++++++++++++++++++++++++%u:login sucess  and UuidHandlerPoitiner size is %u \n",player_UUID,UuidHandlerPoitiner.size());
		login_status = true;
		my_attribute.name = redis.find_user_name(player_UUID);
		my_attribute.avoid_time = 0;
		my_attribute.diamond = diamond;
		my_attribute.workers = 2;
		redis.get_build_map(my_attribute.my_build,player_UUID);
		redis.get_player(my_attribute,player_UUID);
		redis.get_my_build_num(my_attribute.build_num,player_UUID);
		redis.get_build_timer_list(building,player_UUID);
		redis.get_equip_list(myequiplist,my_backpack,backpack_status,my_hero,hero_equip,myherolist,rand_hero,
				email_sale_list,player_UUID);


		//init building;
		long start_time = time(0);
		list<string>  mybuilding = building;
		BOOST_FOREACH(string & str,mybuilding)
		{
			if(my_attribute.my_build.find(str) != my_attribute.my_build.end())
			{

				if(my_attribute.my_build[str].status == 1)
				{
					my_attribute.my_build[str].time -= (start_time -my_attribute.my_build[str].ctime);
					my_attribute.my_build[str].ctime = start_time;
					//printf("my_attribute.my_build[str].ctime is %d,start_time is %d\n",my_attribute.my_build[str].ctime,start_time);

					if(my_attribute.my_build[str].time <= 0)
					{
						build_timer_handle(str);
					}
					else if(my_attribute.my_build[str].time <= 1100)
					{
						redis.set_build(my_attribute.my_build[str],player_UUID);
						en_timer(str);
					}
					else
					{
						redis.set_build(my_attribute.my_build[str],player_UUID);
					}
				}
			}
		}

		SendMessage(my_attribute,MyAttruibuteResponse_ID);
		SendMessage(myequiplist,EquipListResponse_ID);
		SendMessage(myherolist,HeroListResponse_ID);
		int count = myherolist.hero_list.size();
		SendMessage(email_sale_list,EmailSaleListResponse_ID);

		MyArmpsLevelResponse restemp1;
		redis.get_all_arm_max_level(restemp1,global_arms.size() - 1,player_UUID);

		SendMessage(restemp1,MyArmpsLevelResponse_ID);
		//TCPIOThreadManager::SessionHandlerPointer pHandler = manager.GetSessionHandler(0);

		//printf("handler 0;%p user_handler_map[uuid].  %p",&pHandler,);
}
void  LoginWorker::login_handle(int diamond,bool handlemodel)
{
//first send myattribute
	bool towpoint_login  = false;
	if(handlemodel)
	{
		if(UuidHandlerPoitiner.find(player_UUID) != UuidHandlerPoitiner.end())
		{
			if(UuidHandlerPoitiner[player_UUID].get() != NULL)
			{
				UuidHandlerPoitiner[player_UUID]->is_client_off = false;
				UuidHandlerPoitiner[player_UUID]->offline_handler = boost::bind(&LoginWorker::login_handle2,this,diamond);
				UuidHandlerPoitiner[player_UUID]->Close();
				UuidHandlerPoitiner.erase(player_UUID);
				towpoint_login = true;
			}
			printf("%u:login tow point,and first point offline\n",player_UUID);
		}
		SimpleResponse sres;
		sres.msgid = LoginRequest_ID;
		sres.result = 0;
		sres.result = 0;
		SendMessage(sres,SimpleResponse_ID);
	}
	if(!towpoint_login)
	{
		login_handle2(diamond);
	}
}

void LoginWorker::init_new_player(string name,int race)
{
	//wirte name into redis
	redis.set_user_name(player_UUID,name);
	//set 20000diamond into mysql
	mysql.update_diamond(20000,player_UUID);
	//init meter
	redis.set_material(50000000,50000000,50000000,50000000,player_UUID);
	//init level and record
	if(race < 0 || race > 2)race = 0;
	redis.set_player_base(race,0,1,player_UUID);
	redis.set_backpack_num(12,player_UUID);
	redis.set_buidl_num(10,player_UUID);
	//insert 6 build
	redis.init_arms(player_UUID);

	long start_time = time(0);
    Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
	MyBuildDataResponse res1;
    Poco::UUID uid1 = gen.createRandom();
    res1.build_sn = uid1.toString();
	res1.big_type = 0;//0 simple build;1 tower; 2 factory
	res1.status = 0;
	res1.ctime = start_time;
	res1.time = 0;
	res1.build_type = 1;
	res1.build_level = 3;
	res1.cur_hp = 1000;
	res1.build_skin = 0;
	res1.cur_resource = 0;
	res1.x = 48;
	res1.y = 90;
	redis.set_my_build_num(1,1,player_UUID);
	redis.set_build(res1,player_UUID);
	redis.set_city_hall(res1.build_sn,player_UUID);

	MyBuildDataResponse res2;
    Poco::UUID uid2 = gen.createRandom();
    res2.build_sn = uid2.toString();
	res2.big_type = 0;//0 simple build;1 tower; 2 factory
	res2.status = 0;
	res2.ctime = start_time;
	res2.time = 0;
	res2.build_type = 4;
	res2.build_level = 1;
	res2.cur_hp = 1000;
	res2.build_skin = 0;
	res2.cur_resource = 0;
	res2.x = -400;
	res2.y =90;
	redis.set_my_build_num(4,1,player_UUID);
	redis.set_build(res2,player_UUID);
	redis.set_combat_uuid(res2.build_sn,player_UUID);

	MyBuildDataResponse res3;
    Poco::UUID uid3 = gen.createRandom();
    res3.build_sn = uid3.toString();
	res3.big_type = 1;//0 simple build;1 tower; 2 factory
	res3.status = 0;
	res3.ctime = start_time;
	res3.time = 0;
	res3.build_type = 19;
	res3.build_level = 1;
	res3.cur_hp = 1000;
	res3.build_skin = 0;
	res3.cur_resource = 0;
	res3.x = 412;
	res3.y =195;
	redis.set_my_build_num(19,1,player_UUID);
	redis.set_build(res3,player_UUID);

	MyBuildDataResponse res4;
    Poco::UUID uid4 = gen.createRandom();
    res4.build_sn = uid4.toString();
	res4.big_type = 0;//0 simple build;1 tower; 2 factory
	res4.status = 0;
	res4.ctime = start_time;
	res4.time = 0;
	res4.build_type = 7;
	res4.build_level = 1;
	res4.cur_hp = 1000;
	res4.build_skin = 0;
	res4.cur_resource = 0;
	res4.x = 76;
	res4.y =-255;
	redis.set_my_build_num(7,1,player_UUID);
	redis.set_build(res4,player_UUID);

	MyBuildDataResponse res5;
    Poco::UUID uid5 = gen.createRandom();
    res5.build_sn = uid5.toString();
	res5.big_type = 3;//0 simple build;1 tower; 2 factory
	res5.status = 0;
	res5.ctime = start_time;
	res5.time = 0;
	res5.build_type = 12;
	res5.build_level = 1;
	res5.cur_hp = 1000;
	res5.build_skin = 0;
	res5.cur_resource = 0;
	res5.x = -232;
	res5.y = 90;
	redis.set_my_build_num(12,1,player_UUID);
	redis.set_build(res5,player_UUID);

	MyBuildDataResponse res6;
    Poco::UUID uid6 = gen.createRandom();
    res6.build_sn = uid6.toString();
	res6.big_type = 4;//0 simple build;1 tower; 2 factory
	res6.status = 0;
	res6.ctime = start_time;
	res6.time = 0;
	res6.build_type = 16;
	res6.build_level = 1;
	res6.cur_hp = 1000;
	res6.build_skin = 0;
	res6.cur_resource = 0;
	res6.x = 272;
	res6.y = -78;
	redis.set_my_build_num(16,1,player_UUID);
	redis.set_build(res6,player_UUID);

	MyBuildDataResponse res7;
    Poco::UUID uid7 = gen.createRandom();
    res7.build_sn = uid7.toString();
	res7.big_type = 3;//0 simple build;1 tower; 2 factory
	res7.status = 0;
	res7.ctime = start_time;
	res7.time = 0;
	res7.build_type = 14;
	res7.build_level = 1;
	res7.cur_hp = 1000;
	res7.build_skin = 0;
	res7.cur_resource = 0;
	res7.x = -148;
	res7.y = -15;
	redis.set_my_build_num(11,1,player_UUID);
	redis.set_build(res7,player_UUID);

	MyBuildDataResponse res8;
    Poco::UUID uid8 = gen.createRandom();
    res8.build_sn = uid8.toString();
	res8.big_type = 0;//0 simple build;1 tower; 2 factory
	res8.status = 0;
	res8.ctime = start_time;
	res8.time = 0;
	res8.build_type = 6;
	res8.build_level = 1;
	res8.cur_hp = 1000;
	res8.build_skin = 0;
	res8.cur_resource = 0;
	res8.x = -92;
	res8.y = -141;
	redis.set_my_build_num(6,1,player_UUID);
	redis.set_build(res8,player_UUID);

	MyBuildDataResponse res9;
    Poco::UUID uid9 = gen.createRandom();
    res9.build_sn = uid9.toString();
	res9.big_type = 2;//0 simple build;1 tower; 2 factory
	res9.status = 0;
	res9.ctime = start_time;
	res9.time = 0;
	res9.build_type = 8;
	res9.build_level = 1;
	res9.cur_hp = 1000;
	res9.build_skin = 0;
	res9.cur_resource = 0;
	res9.x = 496;
	res9.y = -36;
	redis.set_my_build_num(8,1,player_UUID);
	redis.set_build(res9,player_UUID);

	MyBuildDataResponse res10;
    Poco::UUID uid10 = gen.createRandom();
    res10.build_sn = uid10.toString();
	res10.big_type = 1;//0 simple build;1 tower; 2 factory
	res10.status = 0;
	res10.ctime = start_time;
	res10.time = 0;
	res10.build_type = 17;
	res10.build_level = 1;
	res10.cur_hp = 1000;
	res10.build_skin = 0;
	res10.cur_resource = 0;
	res10.x = 412;
	res10.y = -225;
	redis.set_my_build_num(17,1,player_UUID);
	redis.set_build(res10,player_UUID);

	MyBuildDataResponse res11;
    Poco::UUID uid11 = gen.createRandom();
    res11.build_sn = uid11.toString();
	res11.big_type = 0;//0 simple build;1 tower; 2 factory
	res11.status = 0;
	res11.ctime = start_time;
	res11.time = 0;
	res11.build_type = 3;
	res11.build_level = 1;
	res11.cur_hp = 1000;
	res11.build_skin = 0;
	res11.cur_resource = 0;
	res11.x = -232;
	res11.y = -246;
	redis.set_my_build_num(3,1,player_UUID);
	redis.set_build(res11,player_UUID);

	MyBuildDataResponse res12;
    Poco::UUID uid12 = gen.createRandom();
    res12.build_sn = uid12.toString();
	res12.big_type = 3;//0 simple build;1 tower; 2 factory
	res12.status = 0;
	res12.ctime = start_time;
	res12.time = 0;
	res12.build_type = 13;
	res12.build_level = 1;
	res12.cur_hp = 1000;
	res12.build_skin = 0;
	res12.cur_resource = 0;
	res12.x = 244;
	res12.y = -309;
	redis.set_my_build_num(13,1,player_UUID);
	redis.set_build(res12,player_UUID);


	MyBuildDataResponse res13;
    Poco::UUID uid13 = gen.createRandom();
    res13.build_sn = uid13.toString();
	res13.big_type = 2;//0 simple build;1 tower; 2 factory
	res13.status = 0;
	res13.ctime = start_time;
	res13.time = 0;
	res13.build_type = 9;
	res13.build_level = 1;
	res13.cur_hp = 1000;
	res13.build_skin = 0;
	res13.cur_resource = 0;
	res13.x = 328;
	res13.y = 90;
	redis.set_my_build_num(9,1,player_UUID);
	redis.set_build(res13,player_UUID);

	MyBuildDataResponse res14;
    Poco::UUID uid14 = gen.createRandom();
    res14.build_sn = uid14.toString();
	res14.big_type = 2;//0 simple build;1 tower; 2 factory
	res14.status = 0;
	res14.ctime = start_time;
	res14.time = 0;
	res14.build_type = 10;
	res14.build_level = 1;
	res14.cur_hp = 1000;
	res14.build_skin = 0;
	res14.cur_resource = 0;
	res14.x = 160;
	res14.y = 216;
	redis.set_my_build_num(10,1,player_UUID);
	redis.set_build(res14,player_UUID);

	MyBuildDataResponse res15;
    Poco::UUID uid15 = gen.createRandom();
    res15.build_sn = uid15.toString();
	res15.big_type = 2;//0 simple build;1 tower; 2 factory
	res15.status = 0;
	res15.ctime = start_time;
	res15.time = 0;
	res15.build_type = 11;
	res15.build_level = 1;
	res15.cur_hp = 1000;
	res15.build_skin = 0;
	res15.cur_resource = 0;
	res15.x = 20;
	res15.y = -99;
	redis.set_my_build_num(11,1,player_UUID);
	redis.set_build(res15,player_UUID);

	MyBuildDataResponse res16;
    Poco::UUID uid16 = gen.createRandom();
    res16.build_sn = uid16.toString();
	res16.big_type = 3;//0 simple build;1 tower; 2 factory
	res16.status = 0;
	res16.ctime = start_time;
	res16.time = 0;
	res16.build_type = 15;
	res16.build_level = 1;
	res16.cur_hp = 1000;
	res16.build_skin = 0;
	res16.cur_resource = 0;
	res16.x = -512;
	res16.y = -36;
	redis.set_my_build_num(15,1,player_UUID);
	redis.set_build(res16,player_UUID);

	MyBuildDataResponse res17;
    Poco::UUID uid17 = gen.createRandom();
    res17.build_sn = uid17.toString();
	res17.big_type = 1;//0 simple build;1 tower; 2 factory
	res17.status = 0;
	res17.ctime = start_time;
	res17.time = 0;
	res17.build_type = 18;
	res17.build_level = 1;
	res17.cur_hp = 1000;
	res17.build_skin = 0;
	res17.cur_resource = 0;
	res17.x = -92;
	res17.y = 153;
	redis.set_my_build_num(18,1,player_UUID);
	redis.set_build(res17,player_UUID);

	MyBuildDataResponse res18;
    Poco::UUID uid18 = gen.createRandom();
    res18.build_sn = uid18.toString();
	res18.big_type = 0;//0 simple build;1 tower; 2 factory
	res18.status = 0;
	res18.ctime = start_time;
	res18.time = 0;
	res18.build_type = 2;
	res18.build_level = 1;
	res18.cur_hp = 1000;
	res18.build_skin = 0;
	res18.cur_resource = 0;
	res18.x = -316;
	res18.y = -99;
	redis.set_my_build_num(2,1,player_UUID);
	redis.set_build(res18,player_UUID);

}

void LoginWorker::RegisterRequestService(object *request)
{
	//if(login_status)return;
	RegisterRequest req;
	MsgpackConvert(&req,*request);
	AccountInfoResponse my_account;
	if(redis.exsit_user_name(req.name))
	{
		SimpleResponse sres;
		sres.msgid = RegisterRequest_ID;
		sres.result = NAME_IS_EXSIT;
		sres.sn = 0;
		SendMessage(sres,SimpleResponse_ID);
	}
	else
	{
		mysql.user_register(my_account.account,my_account.passwd,player_UUID);
		login_status = true;
		SendMessage(my_account,AccountInfoResponse_ID);
		init_new_player(req.name,req.race);
		login_handle(20000,false);
	}
}

void LoginWorker::SetNameRequestService(object *request)
{
	SetNameRequest req;
	MsgpackConvert(&req,*request);
	SimpleResponse sres;
	sres.msgid = SetNameRequest_ID;
	if(redis.set_user_name(player_UUID,req.name) && login_status)
	{
		sres.result = 0;
		sres.sn = 0;
	}
	else
	{
		sres.result = UUID_PASSWD_ERROR;
		sres.sn = 0;
	}
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::GetRandNameRequestService(object *request)
{
	GetRandNameRequest req;
	MsgpackConvert(&req,*request);
	if(is_get_random_name)
	{
		redis.add_random_name(last_rand_name,is_man);
	}
	if(req.sex == 1)is_man = false;
	else is_man = true;
	if(redis.get_random_name(last_rand_name,is_man))
	{
		is_get_random_name = true;
		RandNameResponse res;
		res.rand_name = last_rand_name;
		SendMessage(res,RandNameResponse_ID);
	}
	else
	{
		SimpleResponse sres;
		sres.sn = -1;
		sres.msgid = GetRandNameRequest_ID;
		SendMessage(sres,SimpleResponse_ID);
	}
}

void LoginWorker::LoginRequestService(object *request)
{
	if(login_status)return;
	LoginRequest req;
	MsgpackConvert(&req,*request);
	int diamond = 0;
	if(mysql.verify_user(req.account,req.Passwd,player_UUID,diamond))
	{
		login_status = true;
	}
	if(login_status)login_handle(diamond);
	else
	{
		SimpleResponse sres;
		sres.msgid = LoginRequest_ID;
		sres.result = UUID_PASSWD_ERROR;
		sres.sn = 0;
		SendMessage(sres,SimpleResponse_ID);
	}
}

void LoginWorker::LogoutRequestService(object *request)
{
	//broadcast to worker
}

void LoginWorker::BindEmailRequestService(object *request)
{
	BindEmailRequest req;
	MsgpackConvert(&req,*request);

	SimpleResponse sres;
	sres.sn = -1;
	sres.msgid = BindEmailRequest_ID;

	if(mysql.bind_user_email(req.Email,req.Passwd,player_UUID) && login_status)
	{
		sres.result = SUCCESS;
	}
	else
	{
		sres.result = EMAIL_IS_EXSIT;
	}
	SendMessage(sres,SimpleResponse_ID);
}

void LoginWorker::ArmsConfigRequestService(object *request)
{
	BOOST_FOREACH(MyBuffer & value,config_buffer.arms_list_buffer)
	{
		SendSbuffer(value,ArmsListResponse_ID);
	}
}

void LoginWorker::BuildConfigRequestService(object *request)
{
	BOOST_FOREACH(MyBuffer & value,config_buffer.build_list_buffer)
	{
		SendSbuffer(value,BuildListResponse_ID);
	}
	SendSbuffer(config_buffer.city_config,CityLimitListResponse_ID);
}

void LoginWorker::SkillConfigRequestService(object *request)
{
	BOOST_FOREACH(MyBuffer & value,config_buffer.skill_config_buffer)
	{
		SendSbuffer(value,SkillConfigResponse_ID);
	}
}
