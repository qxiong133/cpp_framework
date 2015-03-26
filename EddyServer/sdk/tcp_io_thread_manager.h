// =====================================================================================
// 
//       Filename:  tcp_io_thread_manager.h
// 
//    Description:  TCPIOThreadManager
// 
//        Version:  1.0
//        Created:  2009-12-06 16:03:48
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_IO_THREAD_MANAGER_H_
#define  TCP_IO_THREAD_MANAGER_H_

#include    <vector>

#include    <boost/date_time/posix_time/posix_time.hpp>

#include    "sdk/tcp_defs.h"
#include    "sdk/tcp_io_thread.h"
#include    "sdk/id_generator.h"

namespace eddy { 

class TCPSession;
class TCPSessionHandler;

// =====================================================================================
//        Class:  TCPIOThreadManager
//  Description:  Manages TCPIOThreads
// =====================================================================================
class TCPIOThreadManager : public boost::noncopyable {
 public:
  // ====================  CONSTANTS     =======================================
  const static TCPIOThreadID kMainThreadID = 0;

  // ====================  TYPEDEFS      =======================================
  typedef boost::shared_ptr<TCPIOThread>        ThreadPointer;
  typedef boost::shared_ptr<TCPSession>         SessionPointer;
  typedef boost::shared_ptr<TCPSessionHandler>  SessionHandlerPointer;
  typedef boost::unordered_map<TCPSessionID, 
          SessionHandlerPointer>              SessionHandlerMap;

  // ====================  LIFECYCLE     =======================================
  TCPIOThreadManager(size_t thread_num);
  ~TCPIOThreadManager();

  // ====================  ACCESSORS     =======================================
  boost::asio::io_service& io_service() { 
    return threads_[kMainThreadID]->io_service(); 
  }
  // ====================  MUTATORS      =======================================

  // ====================  OPERATIONS    =======================================
  // Gets thread by the specific id
  TCPIOThread& GetThread(TCPIOThreadID id) { return *threads_[id]; }
  // Gets thread by load balancing, under multi-thread curcumstance, never
  // returns the main thread.
  TCPIOThread& GetThread() {
    if (threads_.size() == 1) return *threads_[kMainThreadID]; // single thread
    if (++next_thread_id_ >= threads_.size()) next_thread_id_ = 1;
    return *threads_[next_thread_id_];
  };
  // Gets the main thread
  TCPIOThread& GetMainThread() {
    return *threads_[kMainThreadID];
  }

  // Must be called in the main thread.
  // Adds the session to any thread, don't keep the pointer of session after 
  // this function called.
  void OnSessionConnect(SessionPointer session, SessionHandlerPointer handler);
  // Async call when session is closed.
  void OnSessionClose(TCPSessionID id);
  void Run();
  void Stop();
  SessionHandlerPointer GetSessionHandler(TCPSessionID id) const; 
 private:
  // ====================  DATA MEMBERS  =======================================
  typedef IDGenerator<TCPSessionID>       SessionIDGenerator;
  std::vector<ThreadPointer>              threads_;
  SessionHandlerMap                       session_handler_map_;
  SessionIDGenerator                      session_id_generator_;
  TCPIOThreadID                           next_thread_id_;
}; // -----  end of class TCPIOThreadManager  -----

}

#endif   // ----- #ifndef TCP_IO_THREAD_MANAGER_H_  -----
