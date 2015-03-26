// =====================================================================================
// 
//       Filename:  my_filter.h
// 
//    Description:  filter
// 
//        Version:  1.0
//        Created:  2009-12-12 18:50:50
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  MY_FILTER_H_
#define  MY_FILTER_H_

#include    <numeric>
#include    <iostream>
#include    <string>
#include    <boost/shared_ptr.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/foreach.hpp>
#include    "sdk/TeaAlgorithm.h"
#include    "sdk/net_message_filter_interface.h"
#include   "boost/shared_ptr.hpp"
#include   "AESSBox.h"
#include <msgpack_id.h>
#include <msgpack_struct.h>
#include "service/_global_macro.h"
extern unsigned char global_aesbox[TOTALBOXES][256];

using namespace boost;
class MyFilter: public eddy::NetMessageFilterInterface {
private:
	const unsigned int bytesToEncrypt;

public:
	typedef boost::uint16_t Header;
	static const size_t kHeaderSize = sizeof(Header);

	MyFilter():bytesToEncrypt(BYTES_TO_ENCRYPT)
	{
		header_read_ = false;
	}

	~MyFilter(){
	}

	static boost::shared_ptr<MyFilter> Create() {
		return boost::make_shared<MyFilter>();
	}

	struct AddMessageSize {
		size_t operator()(size_t sum, const eddy::NetMessage& message) {
			return sum + kHeaderSize + message.size();
		}
	};

	virtual size_t BytesWannaWrite(
			eddy::NetMessageVector& messages_to_be_sent) {
		if (messages_to_be_sent.empty())
			return 0;

		return accumulate(messages_to_be_sent.begin(),
				messages_to_be_sent.end(), 0, AddMessageSize());
	}

	virtual size_t Write(eddy::NetMessageVector& messages_to_be_sent,
			Buffer& buffer) {
		size_t result = 0;
		BOOST_FOREACH(eddy::NetMessage& message, messages_to_be_sent){
		unsigned short ID = *((unsigned short *)message.begin());

		if( message.size() > 65535)//ran max short number
		{
			printf("message is so large!please resign this message,message id is %us",ID);
			return 0;
		}

		Header header = message.size();

		if(ID != ServerChallengeResponse_ID){

			buffer.insert(buffer.end(), reinterpret_cast<const char*>(&header),
														reinterpret_cast<const char*>(&header) + sizeof(Header));
			//Encryption
			TeaAlgorithm::sharedInstance()->encrypt((unsigned char *)(message.begin()+2),(unsigned int )message.size() -2 ,pBox);

		}else{

			int headerlength = header;
			header += EXCHANGETIMES*2;

			buffer.insert(buffer.end(), reinterpret_cast<const char*>(&header),
											reinterpret_cast<const char*>(&header) + sizeof(Header));
			//if it is the indecator to begin encrypt,wo write the code previously
		    buffer.insert(buffer.end(), message.begin(), message.begin()+headerlength);

		    message.clear();
		    //generate a aessbox and use this box as the key related to this session
		  //  memset(pBox,0,sizeof(pBox[256]));
		   // pRandoms = NULL;
		    AESSBox::getBox(pBox,pRandoms);
		    fillMessageWithRandoms(pRandoms,message);

		    printf("\n");
		    for(int i = 0;i< 256;i++)
		    {
		    	printf(" %d ",pBox[i]);
		    }
		    printf("\n");
		}

    	//encryptMessage(message);
		buffer.insert(buffer.end(), message.begin(), message.end());//is error
		size_t bytes_writed = (message.size() + kHeaderSize);

		result += bytes_writed;
	}

		messages_to_be_sent.clear();
		return result;
	}

	virtual size_t BytesWannaRead() {
		if (!header_read_)
			return kHeaderSize;

		return header_;
	}

	virtual size_t Read(eddy::NetMessageVector& messages_received,
			const Buffer& buffer) {
		if (!header_read_) {
			header_ = *(reinterpret_cast<const Header*>(&buffer[0]));
			header_read_ = true;
			return kHeaderSize;

		} else {

			messages_received.push_back(eddy::NetMessage());
			eddy::NetMessage& message = messages_received.back();
			message.Reserve(header_);

			message.Write(&buffer[0], buffer.size());
			//printf("\nbuffer is %u  and message size is %u\n",buffer.size(),message.size());
			if (!decryptMessage(message))
            {
            	printf("%ld date.%ld hour:decryptMessage falied!\n",time(0)/86400,time(0)/3600);
            }
			header_read_ = false;
			return header_;
		}
	}

private:

	void encryptMessage(eddy::NetMessage& message) {
		/*

		//unsigned short ID = *((unsigned short *)message.begin());

        if( ID != ServerChallenge_ID ){



        }else {



        }
        */

	}

	bool decryptMessage(eddy::NetMessage& message) {

		unsigned short ID = *((unsigned short *)(message.begin()));

		if(ID >= ServerChallengeResponse_ID && message.size() > 3 &&  message.size() < 65535)
		{/*this should be id who has not be encrypted*/
			TeaAlgorithm::sharedInstance()->decrypt((unsigned char *)(message.begin()+2),message.size()-2,pBox);
		}
		else
		{
			return true;
		}
	}

    void fillMessageWithRandoms(unsigned char *pRandoms,NetMessage &message){
    	//send the rendoms to client which has the length of EXCHANGETIMES
        message.Write((char *)pRandoms,EXCHANGETIMES*2);
    }

	bool header_read_;
	Header header_;
private:
    unsigned char *pRandoms;
	unsigned char pBox[256];
};

#endif   // ----- #ifndef MY_FILTER_H_  -----
