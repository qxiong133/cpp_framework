// =====================================================================================
// 
//       Filename:  tcp_io_thread.cc
// 
//    Description:  TCPIOThread
// 
//        Version:  1.0
//        Created:  2009-12-06 20:22:27
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_io_thread.h"

#include    <iostream>
#include    <functional>

#include    <boost/thread.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/bind/apply.hpp>
#include    "sdk/tcp_io_thread_manager.h"
#include <service/_login_worker.h>
#include <boost/foreach.hpp>
#include <boost/container/map.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include "../database/redis_chat_worker.h"
#define CHECKTIME 1000
#define FIREDTIME 300

extern vector<MarketGoodsResponse> gloabal_market;
extern boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >    UuidHandlerPoitiner;
namespace eddy {

using namespace boost;

void HeartCheck()
{
	static int cur_count = 0;
	cur_count++;
	long now_time = time(0);
	typedef  boost::unordered_map<unsigned int,boost::shared_ptr<LoginWorker> >::value_type const_pair;
	if(cur_count % 12 == 0)
	{
		BOOST_FOREACH(MarketGoodsResponse& value,gloabal_market)
		{
			value.remain_time -= 60;
			if(value.remain_time < 0)
			{
				LoginWorker::PushNotify(value);
				//LoginWorker.
			}
		}
	}
	if(cur_count >= 200)
	{
		cur_count = 0;
		BOOST_FOREACH(const_pair value,UuidHandlerPoitiner)
		{
			if(value.second.get() != NULL)
			{
				//std::cout<< UuidHandlerPoitiner.size()<<std::endl;
				if(value.second->last_time != 0)
				{
					if(now_time - value.second->last_time > 500)
					{
						value.second->is_client_off = false;
						value.second->Close();
						UuidHandlerPoitiner.erase(value.first);
						//std::cout << "use count "<< value.second.use_count()<< "  " << UuidHandlerPoitiner.size() <<std::endl;
						//value.second.reset();
						//printf("enter HeartCheck  \n");
					}
				}
				list<string> mybuilding = value.second->building;
				BOOST_FOREACH(string & str,mybuilding)
				{
					if(value.second->my_attribute.my_build.find(str) != value.second->my_attribute.my_build.end())
					{
						if(value.second->my_attribute.my_build[str].status == 1)
						{
							value.second->my_attribute.my_build[str].time -= 1100;
							if(value.second->my_attribute.my_build[str].time <= 0)
							{
								value.second->build_timer_handle(str);
							}
							else if( value.second->my_attribute.my_build[str].time <= 1100)
							{
								value.second->en_timer(str);
							}
						}
					}
				}
			}
		}
		return ;
	}
	BOOST_FOREACH(const_pair& value,UuidHandlerPoitiner)
	{
		if(value.second.get() != NULL)
		{
			//std::cout<< UuidHandlerPoitiner.size()<<std::endl;
			if(value.second->last_time != 0)
			{
				if(now_time - value.second->last_time > 500)
				{
					value.second->is_client_off = false;
					value.second->Close();
					UuidHandlerPoitiner.erase(value.first);
					std::cout << "use count "<< value.second.use_count()<< "  " << UuidHandlerPoitiner.size() <<std::endl;
					//printf("enter HeartCheck  \n");
				}
			}
		}
	}
}


TCPIOThread::TCPIOThread(TCPIOThreadID id, TCPIOThreadManager& manager) :
		id_(id), manager_(manager), work_(NULL) {

}

void TCPIOThread::RunThread() {

	if (thread_ != NULL) //already running
		return;

	//pSessionManager.reset(new SessionManager(io_service_, session_queue_));
	//pSessionManager->init(boost::posix_time::seconds(CHECKTIME), FIREDTIME);

	thread_ = shared_ptr < thread
			> (make_shared < thread > (bind(&TCPIOThread::Run, this)));
}

void TCPIOThread::Run() {
	if (work_ == NULL)
		work_ = new boost::asio::io_service::work(io_service_);

	if(id_ == TCPIOThreadManager::kMainThreadID){
		main_thread_heart_timer = new eddy::Timer(io_service_);
#ifdef DEBUG_MODE
		main_thread_heart_timer->Wait(boost::bind(HeartCheck),30000);
#else
		main_thread_heart_timer->Wait(boost::bind(HeartCheck),5000);
#endif
		//check heartbeat per 5
		//pGlobalTimeManager.reset(new GlobalTimeManager(io_service_));
		//pGlobalTimeManager->init();
	}

	boost::system::error_code error;
	io_service_.run(error);

	if (error)
		std::cerr << error.message() << std::endl;
}

void TCPIOThread::Join() {
	if (thread_ != NULL)
		thread_->join();
}

void TCPIOThread::Stop() {
	if (work_ != NULL) {
		delete work_;
		work_ = NULL;
	}

	session_queue_.CloseAllSession();
}

}
