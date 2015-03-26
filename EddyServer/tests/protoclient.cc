// =====================================================================================
// 
//       Filename:  protoclient.cc
// 
//    Description:  test proto client
// 
//        Version:  1.0
//        Created:  2010-05-19 18:22:06
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    <iostream>
#include    <list>
#include    <memory>

#include    <boost/asio/ip/tcp.hpp>
#include    <boost/iostreams/stream.hpp>

#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_client.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/net_message_filter_interface.h"
#include    "sdk/container_device.h"
#include    "tests/my_filter.h"
#include    "tests/proto_def.h"

using namespace eddy;
using namespace std;
using namespace boost;
using namespace boost::iostreams;

class MyHandler : public TCPSessionHandler {
 public:
  MyHandler() {
  }

  static shared_ptr<MyHandler> Create() {
    return make_shared<MyHandler>();
  }

  virtual void OnConnect() {
    using namespace proto;
    {
      MessageA message;
      message.set_name("message A");
      SendProtoMessage(message);
    }

    {
      MessageB message;
      message.set_name("message B");
      message.set_int_val(99);
      SendProtoMessage(message);
    }

    {
      MessageB message;
      message.set_name("message C");
      SendProtoMessage(message);
    }
  }

  virtual void OnMessage(NetMessage& message) {
  }

  template <class MessageType>
  void SendProtoMessage(const MessageType& message) {
    typedef ContainerDevice<NetMessage> NetMessageDevice;
    BOOST_STATIC_ASSERT(GetMessageID<MessageType>::ID != kInvalidMessageID);
    boost::uint16_t id = GetMessageID<MessageType>::ID;
    NetMessage net_message;
    stream<NetMessageDevice>  io(net_message);
    io << id;
    message.SerializeToOstream(&io);
    io.flush();
    Send(net_message);
  }

  virtual void OnClose() {
    cout << "closed" << endl;
  }
};

int main(int argc, char** argv) {
  TCPIOThreadManager manager(1);

  const char* port = "20000";

  if (argc == 2)
    port = argv[1];

  shared_ptr<TCPClient> p(new TCPClient(asio::ip::tcp::resolver_query("127.0.0.1", port),
                                        manager,
                                        &MyHandler::Create,
                                        &MyFilter::Create));

  manager.Run();
  return 0;
}
