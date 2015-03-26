// =====================================================================================
// 
//       Filename:  testserver.cc
// 
//    Description:  test tcp server
// 
//        Version:  1.0
//        Created:  2009-12-10 11:39:42
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

#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_server.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/net_message_filter_interface.h"
#include    "sdk/timer.h"
#include    "tests/my_filter.h"

using namespace eddy;
using namespace std;
using namespace boost;

class MyHandler : public TCPSessionHandler {
 public:
  static shared_ptr<MyHandler> Create() {
    return make_shared<MyHandler>();
  }
  virtual void OnConnect() {
    cout << session_id() << ":connected" << endl;
  };
  virtual void OnMessage(NetMessage& message) {
#if 0
    copy(message.buffer().begin(), message.buffer().end(),
              ostream_iterator<char>(cout, ""));
    cout << endl;
#endif
    Send(message);
  }
  virtual void OnClose() {
    cout << session_id() << ":closed" << endl;
  }
};

int main(int argc, char** argv) {
  TCPIOThreadManager manager(1);
  unsigned short int port = 20000;

  if (argc == 2)
    port = boost::lexical_cast<unsigned short int>(argv[1]);

  TCPServer server(asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),
                   manager,
                   &MyHandler::Create,
                   &MyFilter::Create);

  manager.Run();
  return 0;
}
