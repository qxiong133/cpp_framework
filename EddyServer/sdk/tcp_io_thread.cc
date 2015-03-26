// =====================================================================================
// 
//       Filename:  tcp_io_thread.cc
// 
//    Description:  TCPIOThread
// 
//        Version:  1.0
//        Created:  2009-12-06 20:22:27
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_io_thread.h"

#include    <iostream>
#include    <functional>

#include    <boost/thread.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/bind/apply.hpp>

#include    "sdk/tcp_io_thread_manager.h"

namespace eddy { 

using namespace boost;

TCPIOThread::TCPIOThread(TCPIOThreadID id, TCPIOThreadManager& manager) 
    : id_(id), manager_(manager), work_(NULL) {
    }

void TCPIOThread::RunThread() {
  if (thread_ != NULL) //already running
    return;

  thread_ = shared_ptr<thread>(make_shared<thread>(bind(&TCPIOThread::Run, this)));
}

void TCPIOThread::Run() {
  if (work_ == NULL)
    work_ = new boost::asio::io_service::work(io_service_);

  boost::system::error_code error;
  io_service_.run(error);

  if (error) 
    std::cerr << error.message() << std::endl; 
}

void TCPIOThread::Join() {
  if (thread_ != NULL)
    thread_->join();
}

void TCPIOThread::Stop() {
  if (work_ != NULL) {
    delete work_;
    work_ = NULL;
  }

  session_queue_.CloseAllSession();
}

}
