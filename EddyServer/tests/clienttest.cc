// =====================================================================================
// 
//       Filename:  testclient.cc
// 
//    Description:  test tcp client
// 
//        Version:  1.0
//        Created:  2009-12-10 13:16:40
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

#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_client.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/net_message_filter_interface.h"
#include    "tests/my_filter.h"

using namespace eddy;
using namespace std;
using namespace boost;
using namespace boost::posix_time;

NetMessage message;

const char kLongMessage[] = { "This is a longlonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong\n"
  "message.\n" };
class MyHandler : public TCPSessionHandler {
 public:
  MyHandler() {
    time = microsec_clock::local_time();
  }

  static shared_ptr<MyHandler> Create() {
    return make_shared<MyHandler>();
  }

  virtual void OnConnect() {
    cout << "connected" << endl;
    Send(message);
  }

  virtual void OnMessage(NetMessage& message) {
    ptime new_time = microsec_clock::local_time();
    cout << new_time - time << endl;
    time = new_time;
#if 0
    copy(message.begin(), message.end(),
         ostream_iterator<char>(cout, ""));
#endif
    message = ::message;

    Send(message);
  }

  virtual void OnClose() {
    cout << "closed" << endl;
  }
 private:
  ptime time;
};

int main(int argc, char** argv) {
  message.Reserve(sizeof(kLongMessage));
  message.Write(kLongMessage, sizeof(kLongMessage));
  TCPIOThreadManager manager(0);

  list<shared_ptr<TCPClient> > list;

  const char* port = "20000";

  if (argc == 2)
    port = argv[1];

  for (size_t i=0; i<3000; ++i) {
    shared_ptr<TCPClient> p(new TCPClient(asio::ip::tcp::resolver_query("127.0.0.1", port),
                                          manager,
                                          &MyHandler::Create,
                                          &MyFilter::Create));
    list.push_back(p);
  }

  manager.Run();
  return 0;
}
