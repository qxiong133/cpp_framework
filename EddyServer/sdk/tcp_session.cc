// =====================================================================================
// 
//       Filename:  tcp_session.cc
// 
//    Description:  TCPSession
// 
//        Version:  1.0
//        Created:  2009-12-05 13:49:41
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include    "sdk/tcp_session.h"

#include    <iostream>
#include    <cassert>

#include    <boost/asio.hpp> 
#include    <boost/asio/error.hpp> 
#include    <boost/bind.hpp> 

#include    "sdk/tcp_io_thread_manager.h"
#include    "sdk/tcp_io_thread.h"
#include    "sdk/tcp_session_queue.h"
#include    "sdk/tcp_session_handler.h"
#include    "sdk/net_message_filter_interface.h"

namespace eddy { 

TCPSession::TCPSession(TCPIOThreadManager& io_thread_manager,
                       FilterPointer filter)
    : id_(kInvalidTCPSessionID),
    thread_(io_thread_manager.GetThread()),
    socket_(thread_.io_service()),
    filter_(filter),
    num_handlers_(0),
    closed_(false),
    receive_delay_(boost::posix_time::not_a_date_time), 
    receive_delay_timer_(thread_.io_service()) {
      buffer_to_be_sent_.reserve(16);
      buffer_sending_.reserve(16);
      buffer_receiving_.resize(16);
    }

TCPSession::~TCPSession() {
}

void TCPSession::PostMessageList(NetMessageVector& messageList) {
  if (closed_)
    return;

  assert(messages_to_be_sent_.empty());
  messages_to_be_sent_.swap(messageList);

  size_t bytes_wanna_write = filter_->BytesWannaWrite(messages_to_be_sent_);

  if (bytes_wanna_write == 0)
    return;

  buffer_to_be_sent_.reserve(bytes_wanna_write);
  filter_->Write(messages_to_be_sent_, buffer_to_be_sent_);

  messages_to_be_sent_.clear();

  if (buffer_sending_.empty()) {    // not sending
    buffer_sending_.swap(buffer_to_be_sent_);
    ++ num_handlers_;
    boost::asio::async_write(socket_, 
                             boost::asio::buffer(&buffer_sending_[0], buffer_sending_.size()),
                             boost::bind(&TCPSession::HandleWrite, shared_from_this(),
                                         _1,
                                         _2));
  }
}

void TCPSession::Init(TCPSessionID id) {
  assert(id_ == kInvalidTCPSessionID);
  id_ = id;

  boost::asio::ip::tcp::no_delay option(true);
  socket_.set_option(option);

  if (!thread_.session_queue().Add(shared_from_this())) 
    assert(false);

  size_t bytes_wanna_read = filter_->BytesWannaRead();

  if (bytes_wanna_read == 0) 
    return;

  if (bytes_wanna_read == size_t(-1)) {
    this->buffer_receiving_.resize(this->buffer_receiving_.capacity());
    ++ num_handlers_;
    socket_.async_read_some(boost::asio::buffer(&buffer_receiving_[0], 
                                                buffer_receiving_.size()),
                            boost::bind(&TCPSession::HandleRead, this,
                                        _1,
                                        _2));
  } else {
    buffer_receiving_.resize(bytes_wanna_read);
    ++ num_handlers_;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&buffer_receiving_[0], bytes_wanna_read),
                            boost::bind(&TCPSession::HandleRead, this,
                                        _1,
                                        _2));
  }
}

namespace {

void SendMessageListToHandler(TCPIOThreadManager& manager,
                              TCPSessionID id,
                              NetMessageVector* messages) {
  boost::shared_ptr<TCPSessionHandler> handler = manager.GetSessionHandler(id);

  if (handler == NULL)
    return;

  for_each(messages->begin(), messages->end(),
           boost::bind(&TCPSessionHandler::OnMessage, 
                       handler, _1));

  delete messages;
}

void PackMessageList(boost::shared_ptr<TCPSession> session) {
  if (session->messages_received().empty())
    return;

  NetMessageVector* messages(new NetMessageVector);
  messages->swap(session->messages_received());
  session->thread().manager().GetMainThread().Post(
      boost::bind(&SendMessageListToHandler,
                  boost::ref(session->thread().manager()),
                  session->id(),
                  messages));
}

void SendMessageListDirectly(boost::shared_ptr<TCPSession> session) {
  boost::shared_ptr<TCPSessionHandler> handler = 
      session->thread().manager().GetSessionHandler(session->id());

  if (handler == NULL)
    return;

  for_each(session->messages_received().begin(), 
           session->messages_received().end(),
           boost::bind(&TCPSessionHandler::OnMessage, 
                       handler, _1));

  session->messages_received().clear();
}

} // 

void TCPSession::HandleReceiveTimer(const boost::system::error_code& error) {
  if (error)
    return;

  if (thread_.id() == TCPIOThreadManager::kMainThreadID)
    SendMessageListDirectly(shared_from_this());
  else
    PackMessageList(shared_from_this());
}

void TCPSession::HandleRead(const boost::system::error_code& error,
                            size_t bytes_transferred) {
  -- num_handlers_;
  assert(num_handlers_ >= 0);

  if (error || closed_) {
    closed_ = true;
    HandleClose();
    return;
  }

  bool wanna_post = messages_received_.empty();

  buffer_receiving_.resize(bytes_transferred);

  size_t bytes_read = filter_->Read(messages_received_,
                                    buffer_receiving_);
  assert(bytes_read == bytes_transferred);

  this->buffer_receiving_.clear();

  wanna_post = wanna_post && !this->messages_received_.empty();

  if (wanna_post) {
    if (receive_delay_.is_not_a_date_time()) {
      if (thread_.id() == TCPIOThreadManager::kMainThreadID) 
        thread_.Post(boost::bind(&SendMessageListDirectly, shared_from_this()));
      else
        thread_.Post(boost::bind(&PackMessageList, shared_from_this()));
    } else {
      receive_delay_timer_.expires_from_now(receive_delay_);
      receive_delay_timer_.async_wait(boost::bind(&TCPSession::HandleReceiveTimer,
                                                  this, _1));
    }
  }

  size_t bytes_wanna_read = this->filter_->BytesWannaRead();

  if (bytes_wanna_read == 0)
    return;

  if (bytes_wanna_read == size_t(-1)) {
    this->buffer_receiving_.resize(this->buffer_receiving_.capacity());
    ++ num_handlers_;
    this->socket_.async_read_some(boost::asio::buffer(&this->buffer_receiving_[0], 
                                                      this->buffer_receiving_.size()),
                                  boost::bind(&TCPSession::HandleRead, this,
                                              _1,
                                              _2));
  } else {
    this->buffer_receiving_.resize(bytes_wanna_read);
    ++ num_handlers_;
    boost::asio::async_read(this->socket_,
                            boost::asio::buffer(&this->buffer_receiving_[0], bytes_wanna_read),
                            boost::bind(&TCPSession::HandleRead, this,
                                        _1,
                                        _2));
  }
}

void TCPSession::HandleWrite(const boost::system::error_code& error,
                             size_t bytes_transferred) {
  -- num_handlers_;
  assert(num_handlers_ >= 0);

  if (error) {
    closed_ = true;
    HandleClose();
    return;
  }

  this->buffer_sending_.clear();

  if (this->buffer_to_be_sent_.empty()) {
    size_t bytes_wanna_write 
        = this->filter_->BytesWannaWrite(this->messages_to_be_sent_);

    if (bytes_wanna_write == 0 ) {
	  if (closed_)
        HandleClose();
      return;
    }

    this->buffer_to_be_sent_.reserve(this->buffer_to_be_sent_.size() 
                                     + bytes_wanna_write);

    this->filter_->Write(this->messages_to_be_sent_, 
                         this->buffer_to_be_sent_);
  }

  this->buffer_sending_.swap(this->buffer_to_be_sent_);
  ++ num_handlers_;
  boost::asio::async_write(this->socket_, 
                           boost::asio::buffer(&this->buffer_sending_[0], 
                                               this->buffer_sending_.size()),
                           boost::bind(&TCPSession::HandleWrite, this,
                                       _1,
                                       _2));
}

void TCPSession::HandleClose() {
  if (num_handlers_ > 0)
    return;

  TCPIOThread& main_thread = thread_.manager().GetMainThread();

  main_thread.Post(boost::bind(&TCPIOThreadManager::OnSessionClose,
                               &thread_.manager(),
                               id_));

  boost::system::error_code ec;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  if (ec && ec != boost::asio::error::not_connected) {
    std::cerr << ec.message() << std::endl;
  }

  receive_delay_timer_.cancel();
  PackMessageList(shared_from_this());

  socket_.close();

  thread_.session_queue().Remove(id_);
}

void TCPSession::Close() {
  if (closed_)
    return;

  closed_ = true;

  HandleClose();
}

} // namespace
