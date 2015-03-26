/*
 * _chat_worker.h
 *
 *  Created on: Nov 17, 2013
 *      Author: zhongbing
 */

#ifndef CHAT_WORKER_H_
#define CHAT_WORKER_H_
#include <iostream>
#include <msgpack.hpp>
#include <msgpack_id.h>
#include <msgpack_struct.h>
#include "_login_worker.h"
#include <map>
#include <list>
#include "_global_macro.h"
#include <database/redis_chat_worker.h>
#include <boost/shared_ptr.hpp>

#define CHAT_SERVICE_NUM 4
#define IS_GATEWAY

using namespace std;
using namespace msgpack;
typedef boost::shared_ptr<TCPSessionHandler>  SessionHandlerPointer;

class ChatWorker
{
	typedef void (ChatWorker::*Service)(object&,unsigned int);

	typedef struct Player__
	{
		int UUID;
		string name;
		boost::shared_ptr<LoginWorker> handerpointer;
	}Player;

	map<unsigned int,int> UUID_index_map;

private:

	Player player[MAX_CLIENT];

	list<int> remain_player_index;//remain index add player and get a remain

	list<int> online_player_index;

private://module

	RedisChatWorker* redis;

private:

	void register_all_service();

	ChatWorker();

	ChatWorker(const ChatWorker& c){}

public :

	static ChatWorker& GetInstance();

	void Dispatch(int index,object& requst,unsigned int player_uuid);

	~ChatWorker();

	int OnlineNotify(unsigned int UUID);

	void OfflineNotify(unsigned int UUID,int player_index);

	template<class MessageType>
	void SendMessage(const MessageType& message, ResponseID msgID,int index) {
		if(index < 0 || index > MAX_CLIENT)return ;
		NetMessage net_message;
		short msgid = msgID;
		net_message.Write(reinterpret_cast<const char*>(&msgid),sizeof(msgid));
		msgpack::sbuffer m_sbuf;
		msgpack::pack(m_sbuf,message);
		net_message.Write(m_sbuf.data(),m_sbuf.size());
		if(player[index].handerpointer.get()!=NULL)
		{
			player[index].handerpointer->Send(net_message);
		}
		else
		{
			player[index].handerpointer.reset();
		}
		//Send(net_message);
	}

private :// message service relate;

	Service service[CHAT_SERVICE_NUM];

	void WorldChannelMsgRequestService(object& request,unsigned int player_uuid);//1

	void UnionChannelRequestService(object& request,unsigned int player_uuid);//2

	void PrivateChannelRequestService(object& request,unsigned int player_uuid);//3

	void FindFriendRequestService(object& request,unsigned int player_uuid);//4

};

#endif /* CHAT_WORKER_H_ */
