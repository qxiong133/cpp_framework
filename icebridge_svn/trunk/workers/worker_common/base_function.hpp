/*
 * BaseFunction.h
 *
 *  Created on: Nov 12, 2013
 *      Author: zhongbing
 */

#ifndef BASEFUNCTION_H_
#define BASEFUNCTION_H_
#include <vector>
#include "mdwrkapi.hpp"
#include "msgpack_id.h"

using namespace std;

class WorkerSender
{
	mdwrk* session;
	WorkerSender(){}
	WorkerSender(const WorkerSender& w){}
public:
	WorkerSender(mdwrk* session_a)
	{
		session = session_a;
	}

	template<class MessageType>
	void SendToPlayer(const MessageType& message,Response_ID respon_id,unsigned int player_uuid)
	{
		vector<char> msg_buff;

		short player_count = 1;
		unsigned int player_uuid_need = player_uuid;

		msg_buff.insert(
				msg_buff.end(), reinterpret_cast<const char*>(&player_count),
				reinterpret_cast<const char*>(&player_count) + sizeof(short));

		msg_buff.insert(
				msg_buff.end(), reinterpret_cast<const char*>(&player_uuid_need),
				reinterpret_cast<const char*>(&player_uuid_need) + sizeof(int));

		short msgid = (short)respon_id;

		msg_buff.insert(msg_buff.end(),reinterpret_cast<const char*>(&msgid),
				reinterpret_cast<const char*>(&msgid) + sizeof(short));

		msgpack::sbuffer m_sbuf;
		msgpack::pack(m_sbuf,message);

		msg_buff.insert(msg_buff.end(),m_sbuf.data(),m_sbuf.data() + m_sbuf.size());

		session->send(msg_buff.data(),msg_buff.size());
		printf("====================================================size is %d",msg_buff.size());
	}

	template<class MessageType>
	void SendToPlayerList(const MessageType& message,Response_ID respon_id,vector<unsigned int>* player_uuid_list)
	{
		if(player_uuid_list->size() == 0)return;
		vector<char> msg_buff;

		short player_count = player_uuid_list->size();

		msg_buff.insert(
				msg_buff.end(), reinterpret_cast<const char*>(&player_count),
				reinterpret_cast<const char*>(&player_count) + sizeof(short));

		msg_buff.insert(
				msg_buff.end(), reinterpret_cast<const char*>(player_uuid_list->data()),
				reinterpret_cast<const char*>(player_uuid_list->data()) + sizeof(int)*player_uuid_list->size());

		short msgid = (short)respon_id;
		msg_buff.insert(msg_buff.end(),reinterpret_cast<const char*>(&msgid),
				reinterpret_cast<const char*>(&msgid) + sizeof(short));

		msgpack::sbuffer m_sbuf;
		msgpack::pack(m_sbuf,message);

		msg_buff.insert(msg_buff.end(),m_sbuf.data(),m_sbuf.data() + m_sbuf.size());

		session->send(msg_buff.data(),msg_buff.size());
		printf("====================================================size is %d",msg_buff.size());
	}
};


#endif /* BASEFUNCTION_H_ */
