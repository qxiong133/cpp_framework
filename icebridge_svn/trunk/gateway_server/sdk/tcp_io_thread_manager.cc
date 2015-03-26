// =====================================================================================
// 
//       Filename:  tcp_io_thread_manager.cc
// 
//    Description:  TCPIOThreadManager
// 
//        Version:  1.0
//        Created:  2009-12-07 18:29:08
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_io_thread_manager.h"

#include    <limits>
#include    <cassert>
#include <sys/socket.h>
#include    "sdk/tcp_session.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/SessionFilter.h"
#define MAX_CONNECTION_NUM 10000
namespace eddy {

using namespace boost;

TCPIOThreadManager::TCPIOThreadManager(size_t thread_num) :
		session_id_generator_(kInvalidTCPSessionID + 1,
				(std::numeric_limits<SessionIDGenerator::IDType>::max())) {
	threads_.reserve(thread_num + 1);

	for (size_t i = 0; i <= thread_num; ++i)
		threads_.push_back(ThreadPointer(new TCPIOThread(i, *this)));
}

TCPIOThreadManager::~TCPIOThreadManager() {
	Stop();
}

void TCPIOThreadManager::OnSessionConnect(SessionPointer session,
		SessionHandlerPointer handler) {

	std::string ipStr = session->socket_.local_endpoint().address().to_string();

	if(!sessionFilter.isAddressAllowed(ipStr) || session_handler_map_.size() > MAX_CONNECTION_NUM){
		//this ip is not allowed,so we will close the socket before init
		session->closeSocketBeforeInit();
		return ;
	}

	TCPSessionID session_id = *(session_id_generator_.Get());

	handler->Init(session_id, session->thread().id(), this);

	if (!session_handler_map_.insert(
			std::make_pair(handler->session_id_, handler)).second)
		assert(false);

	//session_handler_map_.
	session->thread().Post(bind(&TCPSession::Init, session, session_id));

	handler->OnConnect();

}

TCPIOThreadManager::SessionHandlerPointer TCPIOThreadManager::GetSessionHandler(
		TCPSessionID id) const {

	SessionHandlerMap::const_iterator it = session_handler_map_.find(id);

	//why return new SessionHandlerPointer,because it has not registered
	if (it == session_handler_map_.end())
		return SessionHandlerPointer();

	return it->second;
}


void TCPIOThreadManager::Stop() {
	if (threads_.empty())
		return;

	for (size_t i = 0; i < threads_.size(); ++i) {

		if (i == kMainThreadID)
			continue;

		threads_[i]->Stop();
	}

	try {
		for (size_t i = 0; i < threads_.size(); ++i) {

			if (i == kMainThreadID)
				continue;

			threads_[i]->Join();
		}

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	// ensure main thread exits last
	threads_[kMainThreadID]->Stop();
}

//clear things related to session which is closing
void TCPIOThreadManager::OnSessionClose(TCPSessionID id) {
	assert(id != kInvalidTCPSessionID);
	SessionHandlerMap::iterator it = session_handler_map_.find(id);

	if (it == session_handler_map_.end())
		return;

	SessionHandlerPointer handler = it->second;

	if (handler != NULL) {
		handler->OnClose();
		handler->Dispose();
		session_handler_map_.erase(it);
	} else {
		session_handler_map_.erase(it);
	}

	//clear the resource managed by playersManager
	//PlayersManager::sharedInstance()->onPlayerOut(id);

	session_id_generator_.Put(id);
}

void TCPIOThreadManager::Run() {
	for (size_t i = 0; i < threads_.size(); ++i) {
		if (i != kMainThreadID)
			threads_[i]->RunThread();
	}

	threads_[kMainThreadID]->Run();
}

}
