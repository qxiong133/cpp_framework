// =====================================================================================
// 
//       Filename:  tcp_session.h
// 
//    Description:  TCPSession
// 
//        Version:  1.0
//        Created:  2009-12-04 17:24:02
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_SESSION_H_
#define  TCP_SESSION_H_

#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "sdk/tcp_session_handler.h"
#include "sdk/tcp_session_queue.h"

namespace eddy { 

class NetMessage;
class NetMessageFilterInterface;
class TCPIOThread;

// =====================================================================================
//        Class:  TCPSession
//  Description:  TCPSession, no concerning about client or server, deals with
//  message packing, compression, encryption, sending (from the sender's view) 
//  and vice versa.
// =====================================================================================
class TCPSession : public boost::enable_shared_from_this<TCPSession>, 
    public boost::noncopyable {
     public:
      // ====================  TYPEDEFS      =======================================
      typedef boost::shared_ptr<TCPSessionHandler>          HandlerPointer;
      typedef boost::shared_ptr<NetMessageFilterInterface>  FilterPointer;

      // ====================  LIFECYCLE     =======================================
      TCPSession(TCPIOThreadManager& io_thread_manager, 
                 FilterPointer filter);
      ~TCPSession();

      // ====================  ACCESSORS     =======================================
      boost::asio::ip::tcp::socket& socket() { return socket_; }
      TCPIOThread& thread() { return thread_; }
      TCPSessionID id() const { return id_; }
      NetMessageVector& messages_received() { return messages_received_; }
      void set_receive_delay(const boost::posix_time::time_duration& delay) {
        receive_delay_ = delay;
      }

      // ====================  MUTATORS      =======================================

      // ====================  OPERATIONS    =======================================
      void PostMessageList(NetMessageVector& messageList);
      void Close();

     private:
      friend class TCPIOThreadManager;
      // called by TCPIOThreadManager
      void Init(TCPSessionID id);
      // handles read
      void HandleRead(const boost::system::error_code& error, 
                      size_t bytes_transferred);
      // handles write
      void HandleWrite(const boost::system::error_code& error,
                       size_t bytes_transferred);

      // handles close
      void HandleClose();

      // handles receive timer
      void HandleReceiveTimer(const boost::system::error_code& error);
      // ====================  DATA MEMBERS  =======================================
      TCPSessionID                      id_;
      TCPIOThread&                      thread_;
      boost::asio::ip::tcp::socket      socket_;
      FilterPointer                     filter_;
      NetMessageVector                  messages_received_;
      NetMessageVector                  messages_to_be_sent_;
      std::vector<char>                 buffer_to_be_sent_;
      std::vector<char>                 buffer_sending_;
      std::vector<char>                 buffer_receiving_;
      int                               num_handlers_;
      bool                              closed_;
      boost::posix_time::time_duration  receive_delay_;
      boost::asio::deadline_timer       receive_delay_timer_;
    }; // -----  end of class TCPSession  -----

}

#endif   // ----- #ifndef TCP_SESSION_H_  -----
