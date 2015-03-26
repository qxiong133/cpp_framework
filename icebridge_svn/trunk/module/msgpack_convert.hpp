#ifndef MSGPACKCONVERT_H_
#define MSGPACKCONVERT_H_
#pragma once
#include <msgpack.hpp>

//MysqlBase* mysql_main_thread;
using namespace  msgpack;
template<class MessageType>
static bool MsgpackConvert(MessageType* message,object& obj)
{
	bool result = true;
	try
	{
		obj.convert(message);
	}
	catch(...)
	{
		result = false;
		printf("type erro or menber is not exactly assign");
		//handle erro ,close client or repeat send this msg exactly;
	}
	return result;
}

#endif /* MY_HANDLER_H_ */
