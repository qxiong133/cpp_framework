/*
 * _chat_worker.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: zhongbing
 */
#include "_chat_worker.h"
#include <boost/container/map.hpp>

#ifdef IS_GATEWAY
#include "_login_worker.h"
#endif
extern boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;
void ChatProcess(const char * data, size_t size,unsigned int player_uuid){

	int size_temp =size-2;
	int index =  *((unsigned short *)(data))- WorldChannelMsgRequest_ID;
	msgpack::object request;
	if(size_temp)
	{
		msgpack::zone z;
		msgpack::unpack_return ret = msgpack::unpack(data + 2,size_temp, NULL, &z, &request);
		if(ret != msgpack::UNPACK_SUCCESS || index < 0 || index > CHAT_SERVICE_NUM)//service.size())
		{
			std::cout<<"unpack error or out of array limit"<<std::endl;
			return ;
		}
		else
		{
			static ChatWorker& chat = ChatWorker::GetInstance();
			chat.Dispatch(index,request,player_uuid);
		}
	}
}

void ChatWorker::Dispatch(int index,object& requst,unsigned int player_uuid)
{
	if(index < 0 || index > CHAT_SERVICE_NUM)
	{
		printf("erro messege pack,and index is %d\n",index);
		return ;
	}
	(this->*service[index])(requst,player_uuid);
}


ChatWorker& ChatWorker::GetInstance()
{
	static ChatWorker chat;
	return chat;
}

ChatWorker::ChatWorker()
{
#ifdef IS_GATEWAY
	LoginWorker::register_worker(true,"chatworker",ChatProcess,WorldChannelMsgRequest_ID,FindFriendRequest_ID);
#endif

	for (int i = MAX_CLIENT - 1; i >= 0; i--) {

		remain_player_index.push_back(i);
	}

	register_all_service();

	redis = new RedisChatWorker();

	redis->get_market_list();
}
ChatWorker::~ChatWorker()
{
	delete redis;
}
void ChatWorker::register_all_service()
{
	/*WorldChannelMsgRequest_ID,//17
	UnionChannelRequest_ID,//18
	PrivateChannelRequest_ID,//19
	FindFriendRequest_ID//20*/
	service[0] = &ChatWorker::WorldChannelMsgRequestService;//8
	service[1] = &ChatWorker::UnionChannelRequestService;//9
	service[2] = &ChatWorker::PrivateChannelRequestService;//10
	service[3] = &ChatWorker::FindFriendRequestService;//11
}

int ChatWorker::OnlineNotify(unsigned int UUID)
{
	if(UuidHandlerPoitiner.find(UUID) != UuidHandlerPoitiner.end() && remain_player_index.size() > 0)
	{
		int index =remain_player_index.back();
		remain_player_index.pop_back();
		UUID_index_map[UUID] = index;
		player[index].handerpointer = UuidHandlerPoitiner[UUID];
		online_player_index.push_back(index);
		player[index].UUID = UUID;
		player[index].name = redis->find_user_name(UUID);
		printf("insert uuid_index_map success");

		return index;
	}
	else
	{
		printf("haner is null,please check it");
		return -1;
	}
}

void ChatWorker::OfflineNotify(unsigned UUID,int player_index)
{
	map<unsigned int,int>::iterator it = UUID_index_map.find(UUID);
	if( it != UUID_index_map.end())
	{
		remain_player_index.push_back(UUID_index_map[UUID]);
		player[it->second].handerpointer.reset();
	//printf(""UUID_index_map[UUID]);
		online_player_index.remove(UUID_index_map[UUID]);
		UUID_index_map.erase(it);
	}
	//online_player_index.erase(*(UUID_it_map[UUID]));

}
void ChatWorker::WorldChannelMsgRequestService(object& request,unsigned int player_uuid)
{
	WorldChannelMsgRequest req;
		MsgpackConvert(&req,request);
	if(req.my_index  >= 0 && req.my_index < MAX_CLIENT)
	{
		WorldChannelMsgResponse res;
		res.msg = req.msg;
		res.name = player[req.my_index].name;
		res.player_index = req.my_index;

		for(list<int>::iterator it = online_player_index.begin();it != online_player_index.end();it++)
		{
			//std::cout<<"use count *it" <<*it<<"  "<<  player[*it].handerpointer.use_count() << std::endl;
			SendMessage(res,WorldChannelMsgResponse_ID,*it);
		}
	}
	else
	{
		printf("parameter is erro\n");
	}
	//array index
}

void ChatWorker::UnionChannelRequestService(object& request,unsigned int player_uuid)
{
	UnionChannelRequest req;
		MsgpackConvert(&req,request);
	if(req.my_index < 0 || req.my_index > MAX_CLIENT)
	{
		printf("UnionChannelRequestService index is error\n");
		return ;
	}
	else
	{

	}
}

void ChatWorker::PrivateChannelRequestService(object& request,unsigned int player_uuid)
{
	PrivateChannelRequest req;
		MsgpackConvert(&req,request);
	if(req.my_index < 0 || req.my_index > MAX_CLIENT || req.player_index < 0 ||
			req.player_index > MAX_CLIENT || player[req.player_index].handerpointer.get() == NULL
			|| req.player_index == req.my_index)
	{
		printf("index is error");
		return ;
	}
	PrivateChannelResponse res;
	res.msg = req.msg;
	res.name =  player[req.my_index].name;
	res.player_index = req.my_index;
	SendMessage(res,PrivateChannelResponse_ID,req.player_index);
}

void ChatWorker::FindFriendRequestService(object& request,unsigned int player_uuid)
{
	FindFriendRequest req;
		MsgpackConvert(&req,request);
	FindFriendResponse res;
	if(redis->find_user_uuid(req.name,player_uuid) && req.my_index > 0 && req.my_index < MAX_CLIENT)
	{
		if(UUID_index_map.find(player_uuid) != UUID_index_map.end())
		{
			res.flag = 0;
			int index = UUID_index_map[player_uuid];
			res.name = player[index].name;
			res.player_index = index;
		}
	}
	else
	{
		res.flag = -1;
	}
	SendMessage(res,FindFriendResponse_ID,req.my_index);
}
