// =====================================================================================
// 
//       Filename:  tcp_server.h
// 
//    Description:  tcp server
// 
//        Version:  1.0
//        Created:  2009-12-03 20:57:48
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_SERVER_H_
#define  TCP_SERVER_H_

#include    <boost/asio/ip/tcp.hpp>
#include    <boost/shared_ptr.hpp>
#include    <boost/function.hpp>

namespace eddy { 

class TCPSession;
class TCPSessionHandler;
class TCPIOThreadManager;
class NetMessageFilterInterface;

// =====================================================================================
//        Class:  TCPServer
//  Description:  TCPServer framework
// =====================================================================================
class TCPServer {
 public:
  // ====================  TYPEDEFS      =======================================
  typedef boost::shared_ptr<TCPSession>                     SessionPointer;
  typedef boost::shared_ptr<TCPSessionHandler>              SessionHandlerPointer;
  typedef boost::function<SessionHandlerPointer ()>         SessionHandlerCreator;
  typedef boost::shared_ptr<NetMessageFilterInterface>      SessionFilterPointer;
  typedef boost::function<SessionFilterPointer ()>          SessionFilterCreator;

  // ====================  LIFECYCLE     =======================================
  TCPServer(const boost::asio::ip::tcp::endpoint& endpoint,
            TCPIOThreadManager& io_thread_manager,
            const SessionHandlerCreator& session_handler_creator,
            const SessionFilterCreator& session_filter_creator);

  // ====================  ACCESSORS     =======================================
  boost::asio::io_service& io_service();

  // ====================  MUTATORS      =======================================

  // ====================  OPERATIONS    =======================================

 private:
  void HandleAccept(SessionPointer session, const boost::system::error_code& error);
  // ====================  DATA MEMBERS  =======================================
  boost::asio::ip::tcp::acceptor  acceptor_;
  TCPIOThreadManager&             io_thread_manager_;
  SessionHandlerCreator           session_handler_creator_;
  SessionFilterCreator            session_filter_creator_;
}; // -----  end of class TCPServer  -----

} // ----- end of namespace eddy -----

#endif   // ----- #ifndef TCP_SERVER_H_  -----
