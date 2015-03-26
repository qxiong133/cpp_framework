// =====================================================================================
// 
//       Filename:  protoserver.cc
// 
//    Description:  test protobuf
// 
//        Version:  1.0
//        Created:  2010-05-19 17:15:04
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    <iostream>

#include    <boost/lexical_cast.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/iostreams/stream.hpp>
#include    <boost/static_assert.hpp>

#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_server.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/container_device.h"
#include    "sdk/net_message_filter_interface.h"
#include    "tests/my_filter.h"
#include    "tests/proto_def.h"

using namespace eddy;
using namespace std;
using namespace boost;
using namespace boost::iostreams;

class MyHandler : public TCPSessionHandler {
 public:
  static shared_ptr<MyHandler> Create() {
    return make_shared<MyHandler>();
  }

  virtual void OnConnect() {
    cout << session_id() << ":connected" << endl;
  };

  virtual void OnMessage(NetMessage& message) {
    DispatchMessage(message);
  }

  virtual void OnClose() {
    cout << session_id() << ":closed" << endl;
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

  template <class MessageType>
  void MessageHandler(MessageType& message) {
    cout << "Unhandled message:" << GetMessageID<MessageType>::ID << endl;
  }

  void DispatchMessage(NetMessage& net_message) {
    typedef ContainerDevice<NetMessage> NetMessageDevice;
    stream<NetMessageDevice>  io(net_message);
    boost::uint16_t id = 0;
    io >> id;

    switch (id)
    {
#define DECLARE_MESSAGE(x) case k##x##ID: \
      {\
        proto::x message;\
        message.ParseFromIstream(&io);\
        MessageHandler(message);\
      }\
      break;
#include    "proto.h"
#undef DECLARE_MESSAGE
      default:
        std::cout << "Unknown message:" << id << endl;
    }
  }

  void MessageHandler(proto::MessageA& message) {
    cout << "MessageA received:" << endl;
    cout << message.DebugString() << endl;;
  }

  void MessageHandler(proto::MessageB& message) {
    cout << "MessageB received:" << endl;
    cout << message.DebugString() << endl;;
  }

  void MessageHandler(proto::MessageC& message) {
    cout << "MessageC received:" << endl;
    cout << message.DebugString() << endl;;
  }

  // add more message handles here, they will be automatically registered.
};


int main(int argc, char** argv) {
  TCPIOThreadManager manager(3);  // sync interval
  unsigned short int port = 20000;

  if (argc == 2)
    port = boost::lexical_cast<unsigned short int>(argv[1]);

  TCPServer server(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),
                   manager,
                   &MyHandler::Create,
                   &MyFilter::Create);

  manager.Run();
  return 0;
}
