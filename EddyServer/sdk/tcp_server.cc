// =====================================================================================
// 
//       Filename:  tcp_server.cc
// 
//    Description:  TCPServer
// 
//        Version:  1.0
//        Created:  2009-12-05 13:16:33
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_server.h"

#include    <iostream>

#include    <boost/bind.hpp>
#include    <boost/asio.hpp>
#include    <boost/asio/ip/tcp.hpp>

#include    "sdk/tcp_session.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/tcp_io_thread_manager.h"

namespace eddy { 

using namespace std;
using boost::asio::ip::tcp;

TCPServer::TCPServer(const tcp::endpoint& endpoint,
                     TCPIOThreadManager& io_thread_manager,
                     const SessionHandlerCreator& session_handler_creator,
                     const SessionFilterCreator& session_filter_creator) 
    : acceptor_(io_thread_manager.io_service(), endpoint),
    io_thread_manager_(io_thread_manager),
    session_handler_creator_(session_handler_creator), 
    session_filter_creator_(session_filter_creator) {

      SessionPointer session(new TCPSession(io_thread_manager_, session_filter_creator_()));
      acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.async_accept(session->socket(),
                             boost::bind(&TCPServer::HandleAccept,
                                         this,
                                         session,
                                         boost::asio::placeholders::error));
    }

void TCPServer::HandleAccept(SessionPointer session,
                             const boost::system::error_code& error) {
  if (error) {
    cerr << error.message() << endl;
    assert(false);
    return;
  }

  SessionHandlerPointer handler = session_handler_creator_();
  io_thread_manager_.OnSessionConnect(session, handler);

  SessionPointer new_session(new TCPSession(io_thread_manager_, session_filter_creator_()));
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&TCPServer::HandleAccept,
                                     this,
                                     new_session,
                                     boost::asio::placeholders::error));
}

boost::asio::io_service& TCPServer::io_service() {
  return io_thread_manager_.io_service();
}

}
