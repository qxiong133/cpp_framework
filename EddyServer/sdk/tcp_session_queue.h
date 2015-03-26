// =====================================================================================
// 
//       Filename:  tcp_session_queue.h
// 
//    Description:  TCPSessionQueue
// 
//        Version:  1.0
//        Created:  2009-12-04 17:14:24
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_SESSION_QUEUE_H_
#define  TCP_SESSION_QUEUE_H_

#include    <deque>

#include    <boost/shared_ptr.hpp>
#include    <boost/unordered_map.hpp>

#include    "sdk/tcp_defs.h"

namespace eddy { 

class TCPSession;

// =====================================================================================
//        Class:  TCPSessionQueue
//  Description:  TCPSessionQueue, manages TCPSession
// =====================================================================================
class TCPSessionQueue {
 public:
  // ====================  TYPEDEFS      =======================================
  typedef boost::shared_ptr<TCPSession>   SessionPointer;

  // ====================  LIFECYCLE      =======================================
  ~TCPSessionQueue();

  // ====================  OPERATIONS    =======================================
  bool Add(SessionPointer session);
  void Remove(SessionPointer session);
  void Remove(TCPSessionID id);
  void Clear();
  void CloseAllSession();
  SessionPointer Get(TCPSessionID id);

 private:
  // ====================  DATA MEMBERS  =======================================
  typedef boost::unordered_map<TCPSessionID, SessionPointer> SessionMap;
  SessionMap map_;
}; // -----  end of class TCPSessionQueue  -----

}

#endif   // ----- #ifndef TCP_SESSION_QUEUE_H_  -----
