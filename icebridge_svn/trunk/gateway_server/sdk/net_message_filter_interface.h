// =====================================================================================
// 
//       Filename:  net_message_fileter_interface.h
// 
//    Description:  filters net messages
// 
//        Version:  1.0
//        Created:  2009-12-12 15:16:13
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  NET_MESSAGE_FILTER_INTERFACE_H_
#define  NET_MESSAGE_FILTER_INTERFACE_H_

#include    <vector>

#include    "sdk/net_message.h"

namespace eddy { 

// =====================================================================================
//        Class:  NetMessageFilterInterface
//  Description:  filters net messages, encodes/decodes header,
//  compreses/decompresess, encrypts/decrypts, etc.
// =====================================================================================
class NetMessageFilterInterface {
 public:
  typedef std::vector<char> Buffer;
  // ====================  LIFECYCLE     =======================================
  virtual ~NetMessageFilterInterface() {}

  // ====================  OPERATIONS    =======================================
  // queries bytes wanna write as the size of pre-allocated memory, 
  // for the purpose of efficiency.
  virtual size_t BytesWannaWrite(NetMessageVector& messages_to_be_sent) = 0;
  // writes ALL messsages in messages_to_be_sent to out_buffer.
  // returns bytes writed.
  virtual size_t Write(NetMessageVector& messages_to_be_sent, 
                       Buffer& out_buffer) = 0;
  // queries bytes wanna read, return size_t(-1) means read some.
  virtual size_t BytesWannaRead() = 0; 
  // pacakets raw data into NetMessge, appends it to messges_received.
  virtual size_t Read(NetMessageVector& messages_received, 
                      const Buffer& in_buffer) = 0;
}; // -----  end of class NetMessageFilterInterface  -----

} // 
#endif   // ----- #ifndef NET_MESSAGE_FILTER_INTERFACE_H_  -----
