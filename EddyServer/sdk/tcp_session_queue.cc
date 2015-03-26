// =====================================================================================
// 
//       Filename:  tcp_session_queue.cc
// 
//    Description:  TCPSessionQueue
// 
//        Version:  1.0
//        Created:  2009-12-05 15:14:20
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_session_queue.h"

#include    <cassert>

#include    <boost/bind.hpp> 

#include    "sdk/tcp_session.h"

namespace eddy { 

TCPSessionQueue::~TCPSessionQueue() {
}

bool TCPSessionQueue::Add(SessionPointer session) {
  return map_.insert(std::make_pair(session->id(), session)).second;
}

void TCPSessionQueue::Remove(SessionPointer session) {
  map_.erase(session->id());
}

void TCPSessionQueue::Remove(TCPSessionID id) {
  map_.erase(id);
}

TCPSessionQueue::SessionPointer TCPSessionQueue::Get(TCPSessionID id) {
  SessionMap::iterator it = map_.find(id);

  if (it != map_.end())
    return it->second;

  return SessionPointer();
}

void TCPSessionQueue::Clear() {
  map_.clear();
}

void TCPSessionQueue::CloseAllSession() {
  for (SessionMap::iterator it = map_.begin();
       it != map_.end(); ++it) {
    it->second->Close();
  }
}

} // end of namespace
