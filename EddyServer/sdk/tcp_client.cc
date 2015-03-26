// =====================================================================================
// 
//       Filename:  tcp_client.cc
// 
//    Description:  TCPClient
// 
//        Version:  1.0
//        Created:  2009-12-10 11:04:00
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_client.h"

#include    <iostream>

#include    <boost/asio.hpp>
#include    <boost/asio/ip/tcp.hpp>

#include    "sdk/tcp_session.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/tcp_io_thread_manager.h"

namespace eddy { 

using namespace std;
using boost::asio::ip::tcp;

TCPClient::TCPClient(const boost::asio::ip::tcp::resolver::query& query,
                     TCPIOThreadManager& io_thread_manager,
                     const SessionHandlerCreator& session_handler_creator,
                     const SessionFilterCreator& session_filter_creator) 
    : resolver_(io_thread_manager.io_service()),
    io_thread_manager_(io_thread_manager),
    session_handler_creator_(session_handler_creator),
    session_filter_creator_(session_filter_creator) {

      SessionPointer session(new TCPSession(io_thread_manager_, session_filter_creator_()));
      tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);
      tcp::endpoint endpoint = *endpoint_iterator;
      session->socket().async_connect(endpoint,
                                      boost::bind(&TCPClient::HandleConnect,
                                                session,
                                                session_handler_creator_(),
                                                _1));
    }

boost::asio::io_service& TCPClient::io_service() {
  return io_thread_manager_.io_service();
}

void TCPClient::HandleConnect(SessionPointer session, 
                              SessionHandlerPointer handler,
                              const boost::system::error_code& error) {
  if (error) {
    cerr << error.message() << endl;
    return;
  }
  
  session->thread().manager().GetMainThread().Post(
      boost::bind(&TCPIOThreadManager::OnSessionConnect,
                  boost::ref(session->thread().manager()),
                  session,
                  handler));
}

}
