// =====================================================================================
// 
//       Filename:  tcp_session_handler.h
// 
//    Description:  TCPSessionHandler
// 
//        Version:  1.0
//        Created:  2009-12-04 17:27:10
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_SESSION_HANDLER_H_
#define  TCP_SESSION_HANDLER_H_

#include    <boost/noncopyable.hpp>
#include    <boost/enable_shared_from_this.hpp>
#include    <boost/date_time/posix_time/ptime.hpp>
#include    <boost/asio/deadline_timer.hpp>

#include    "sdk/tcp_defs.h"
#include    "sdk/net_message.h"

namespace eddy { 

class TCPSession;
class TCPIOThreadManager;

// =====================================================================================
//        Class:  TCPSessionHandler
//  Description:  Handles TCPSession
// =====================================================================================
class TCPSessionHandler : public boost::enable_shared_from_this<TCPSessionHandler>,
    public boost::noncopyable {
      friend class TCPIOThreadManager;
     public:
      // ====================  TYPEDEFS      =======================================

      // ====================  LIFECYCLE     =======================================
      TCPSessionHandler();
      virtual ~TCPSessionHandler();

      // ====================  ACCESSORS     =======================================
      TCPSessionID session_id() const { return session_id_; }
      TCPIOThreadID session_thread_id() const { return session_thread_id_; }
      NetMessageVector& messages_to_be_sent() { return messages_to_be_sent_; }
      TCPIOThreadManager* io_thread_manager() { return io_thread_manager_; }

      // ====================  OPERATIONS    =======================================
      // sends message to remote endpoint, the content of message would be consumed
      void Send(NetMessage& message);

      // closes the session, the underlying TCPSession will wait until all data
      // in write-buffer has been sent, then notify this->OnClose().
      void Close();

      // true if the session is closed
      bool IsClosed() { return kInvalidTCPSessionID == session_id_; }

      // sets send delay
      void SetSendDelay(const boost::posix_time::time_duration& delay);

      // sets receive delay
      void SetReceiveDelay(const boost::posix_time::time_duration& delay);

      // call when connection complete
      virtual void OnConnect() = 0;                 

      // call when NetMessage received
      virtual void OnMessage(NetMessage& message) = 0;               

      // call when TCPSession is closed.After call,this object will be deleted.
      virtual void OnClose() = 0;

     private:
      // called by thread manager
      void Init(TCPSessionID session_id, 
                TCPIOThreadID session_thread_id,
                TCPIOThreadManager* manager);
      void Dispose();
      void HandleSendTimer(const boost::system::error_code& error);

      // ====================  DATAMEMBERS   =======================================
      TCPSessionID                        session_id_;
      TCPIOThreadID                       session_thread_id_;
      TCPIOThreadManager*                 io_thread_manager_;
      NetMessageVector                    messages_to_be_sent_;
      boost::posix_time::time_duration    send_delay_;
      boost::asio::deadline_timer*        send_delay_timer_;
    }; // -----  end of class TCPSessionHandler  -----

}

#endif   // ----- #ifndef TCP_SESSION_HANDLER_H_  -----
