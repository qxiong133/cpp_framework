// =====================================================================================
// 
//       Filename:  tcp_client.h
// 
//    Description:  TCPClient
// 
//        Version:  1.0
//        Created:  2009-12-05 17:47:28
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_CLIENT_H_
#define  TCP_CLIENT_H_

#include    <boost/shared_ptr.hpp>
#include    <boost/function.hpp>
#include    <boost/asio/ip/tcp.hpp>

namespace eddy { 

class TCPSession;
class TCPSessionHandler;
class TCPIOThreadManager;
class NetMessageFilterInterface;

// =====================================================================================
//        Class:  TCPClient
//  Description:  TCPClient
// =====================================================================================
class TCPClient {
 public:
  // ====================  TYPEDEFS      =======================================
  typedef boost::shared_ptr<TCPSession>                     SessionPointer;
  typedef boost::shared_ptr<TCPSessionHandler>              SessionHandlerPointer;
  typedef boost::function<SessionHandlerPointer ()>         SessionHandlerCreator;
  typedef boost::shared_ptr<NetMessageFilterInterface>      SessionFilterPointer;
  typedef boost::function<SessionFilterPointer ()>          SessionFilterCreator;

  // ====================  LIFECYCLE     =======================================
  TCPClient(const boost::asio::ip::tcp::resolver::query& query,
            TCPIOThreadManager& io_thread_manager,
            const SessionHandlerCreator& session_handler_creator,
            const SessionFilterCreator& session_filter_creator);

  // ====================  ACCESSORS     =======================================
  boost::asio::io_service& io_service();

  // ====================  MUTATORS      =======================================

  // ====================  OPERATIONS    =======================================

 private:
  static void HandleConnect(SessionPointer session, 
                            SessionHandlerPointer handler,
                            const boost::system::error_code& error);
  // ====================  DATA MEMBERS  =======================================
  boost::asio::ip::tcp::resolver  resolver_;
  TCPIOThreadManager&             io_thread_manager_;
  SessionHandlerCreator           session_handler_creator_;
  SessionFilterCreator            session_filter_creator_;
}; // -----  end of class TCPClient  -----

} // namespace
#endif   // ----- #ifndef TCP_CLIENT_H_  -----
