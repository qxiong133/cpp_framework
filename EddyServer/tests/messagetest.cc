// =====================================================================================
// 
//       Filename:  messagetest.cc
// 
//    Description:  test protobuf
// 
//        Version:  1.0
//        Created:  2010-05-18 22:12:58
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    <iostream>
#include    <string>

#include    <boost/iostreams/stream.hpp>

#include    "tests/messagetest.pb.h"

#include    "sdk/net_message.h"
#include    "sdk/container_device.h"

int main() {
  using namespace std;
  using namespace eddy;
  using namespace boost::iostreams;

  // from messagetest.proto
  messagetest::MessageA message1;
  messagetest::MessageA message2;

  message1.set_name("my message");
  message1.set_id(69);
  
  typedef ContainerDevice<NetMessage> NetMessageDevice;
  NetMessage net_message;
  stream<NetMessageDevice>  io(net_message);

  int a = 15;
  io << a;
  message1.SerializeToOstream(&io);
  io.seekg(0, std::ios_base::beg);

  int b = 0;
  io >> b;
  message2.ParseFromIstream(&io);

  assert(a == b);
  assert(message1.DebugString() == message2.DebugString());
  return 0;
}
