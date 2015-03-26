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
#include   <stdio.h>
extern long GlobalTime;

namespace eddy {

TCPSession::TCPSession(TCPIOThreadManager& io_thread_manager,
		FilterPointer filter) :
		id_(kInvalidTCPSessionID), thread_(io_thread_manager.GetThread()), socket_(
				thread_.io_service()), filter_(filter), num_handlers_(0), closed_(
				false), receive_delay_(boost::posix_time::not_a_date_time), receive_delay_timer_(
				thread_.io_service()),lastReceiveDataTime(2) {
	buffer_to_be_sent_.reserve(16);
	buffer_sending_.reserve(16);
	buffer_receiving_.resize(16);

	activeClosed = false;

}

TCPSession::~TCPSession() {

}

//post message list to the io_service
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

	//peter what if this session is sending buffer,what will be returned ??
	if (buffer_sending_.empty()) {    // not sending

		buffer_sending_.swap(buffer_to_be_sent_);

		++num_handlers_;
		boost::asio::async_write(socket_,
				boost::asio::buffer(&buffer_sending_[0],
						buffer_sending_.size()),
				boost::bind(&TCPSession::HandleWrite, shared_from_this(), _1,
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

	//why size of -1 will lead this operation
	if (bytes_wanna_read == size_t(-1)) {
		this->buffer_receiving_.resize(this->buffer_receiving_.capacity());

		++num_handlers_;
		waitingRead = true;
		socket_.async_read_some(
				boost::asio::buffer(&buffer_receiving_[0],
						buffer_receiving_.size()),
				boost::bind(&TCPSession::HandleRead, this, _1, _2));
	} else {
		buffer_receiving_.resize(bytes_wanna_read);
		++num_handlers_;
		waitingRead = true;
		boost::asio::async_read(socket_,
				boost::asio::buffer(&buffer_receiving_[0], bytes_wanna_read),
				boost::bind(&TCPSession::HandleRead, this, _1, _2));
	}
}

namespace {

//send the messages to the handler associated with session id
void SendMessageListToHandler(TCPIOThreadManager& manager,
		boost::shared_ptr<TCPSession> session, NetMessageVector* messages) {
	boost::shared_ptr<TCPSessionHandler> handler = manager.GetSessionHandler(
			session->id());

	if (handler == NULL)
		return;

	for_each(messages->begin(), messages->end(),
			boost::bind(&TCPSessionHandler::OnMessage, handler, _1));

	delete messages;
}

//main thread is used to send the current session's received messages
void PackMessageList(boost::shared_ptr<TCPSession> session) {

	//message received is empty
	if (session->messages_received().empty())
		return;

	NetMessageVector* messages(new NetMessageVector);
	messages->swap(session->messages_received());

	//every handler is executing in the main thread ??
	session->thread().manager().GetMainThread().Post(
			boost::bind(&SendMessageListToHandler,
					boost::ref(session->thread().manager()), session,
					messages));
}

//this uses the current thread to send the message to be dealt to the handler hold by the thread
void SendMessageListDirectly(boost::shared_ptr<TCPSession> session) {
	boost::shared_ptr<TCPSessionHandler> handler =
			session->thread().manager().GetSessionHandler(session->id());

	if (handler == NULL)
		return;

	for_each(session->messages_received().begin(),
			session->messages_received().end(),
			boost::bind(&TCPSessionHandler::OnMessage, handler, _1));

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

	//get the last active time
	lastReceiveDataTime = GlobalTime;

	--num_handlers_;
	waitingRead = true;
	assert(num_handlers_ >= 0);

	if (error || closed_) {
		closed_ = true;
		HandleClose();
		return;
	}

	bool wanna_post = messages_received_.empty();

	buffer_receiving_.resize(bytes_transferred);

	size_t bytes_read = filter_->Read(messages_received_, buffer_receiving_);
	assert(bytes_read == bytes_transferred);

	this->buffer_receiving_.clear();

	wanna_post = wanna_post && !this->messages_received_.empty();

	if (wanna_post) {
		if (receive_delay_.is_not_a_date_time()) {
			if (thread_.id() == TCPIOThreadManager::kMainThreadID)
				thread_.Post(
						boost::bind(&SendMessageListDirectly,
								shared_from_this()));
			else
				thread_.Post(boost::bind(&PackMessageList, shared_from_this()));
		} else {
			receive_delay_timer_.expires_from_now(receive_delay_);
			receive_delay_timer_.async_wait(
					boost::bind(&TCPSession::HandleReceiveTimer, this, _1));
		}
	}

	size_t bytes_wanna_read = this->filter_->BytesWannaRead();

	if (bytes_wanna_read == 0)
		return;

	if (bytes_wanna_read == size_t(-1)) {
		//why should resize the buffer,since the bytes wanna is -1
		this->buffer_receiving_.resize(this->buffer_receiving_.capacity());
		++num_handlers_;
		waitingRead = true;
		this->socket_.async_read_some(
				boost::asio::buffer(&this->buffer_receiving_[0],
						this->buffer_receiving_.size()),
				boost::bind(&TCPSession::HandleRead, this, _1, _2));
	} else {
		this->buffer_receiving_.resize(bytes_wanna_read);
		++num_handlers_;
		waitingRead = true;
		boost::asio::async_read(this->socket_,
				boost::asio::buffer(&this->buffer_receiving_[0],
						bytes_wanna_read),
				boost::bind(&TCPSession::HandleRead, this, _1, _2));
	}
}

void TCPSession::HandleWrite(const boost::system::error_code& error,
		size_t bytes_transferred) {

	//restartTimer();

	--num_handlers_;
	assert(num_handlers_ >= 0);

	if (error) {
		closed_ = true;
		HandleClose();
		return;
	}

	this->buffer_sending_.clear();

	if (this->buffer_to_be_sent_.empty()) {
		size_t bytes_wanna_write = this->filter_->BytesWannaWrite(
				this->messages_to_be_sent_);

		if (bytes_wanna_write == 0) {
			if (closed_)
				HandleClose();
			return;
		}

		this->buffer_to_be_sent_.reserve(
				this->buffer_to_be_sent_.size() + bytes_wanna_write);

		this->filter_->Write(this->messages_to_be_sent_,
				this->buffer_to_be_sent_);
	}

	this->buffer_sending_.swap(this->buffer_to_be_sent_);
	//num_handlers means how many handlers have hang in the session ,in the socket
	++num_handlers_;
	boost::asio::async_write(this->socket_,
			boost::asio::buffer(&this->buffer_sending_[0],
					this->buffer_sending_.size()),
			boost::bind(&TCPSession::HandleWrite, this, _1, _2));

}

void TCPSession::HandleClose() {
	//peter the number of handler means what ??
	//if (num_handlers_ > 1  || (num_handlers_ == 1 && waitingRead == false))
	//	return;

	if (num_handlers_ > 0 || activeClosed) {//todo
		return;
	}

	TCPIOThread& main_thread = thread_.manager().GetMainThread();

	//inform the mainManager
	main_thread.Post(
			boost::bind(&TCPIOThreadManager::OnSessionClose, &thread_.manager(),
					id_));

	boost::system::error_code ec;
	//shut down both the send stream and receive stream,how many states socket have ??
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

	if (ec && ec != boost::asio::error::not_connected) {
		std::cerr << ec.message() << std::endl;
	}

	//cancel the receive timer
	receive_delay_timer_.cancel();
	PackMessageList(shared_from_this());

	socket_.close();

	//remove self from the queue which is hold by the current thread
	thread_.session_queue().Remove(id_);
}

void TCPSession::closeSocketBeforeInit(){

    socket_.close();
}

void TCPSession::Close(bool isWaitSend) {

	if (closed_)
		return;

	closed_ = true;

	HandleClose();
}

long TCPSession::getNotActiveTime() {

	return GlobalTime - lastReceiveDataTime;

}

void TCPSession::activeClose() {
	activeClosed = true;
	TCPIOThread& main_thread = thread_.manager().GetMainThread();

	//inform the mainManager
	main_thread.Post(
			boost::bind(&TCPIOThreadManager::OnSessionClose, &thread_.manager(),
					id_));

	boost::system::error_code ec;
	//shut down both the send stream and receive stream,how many states socket have ??
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

	if (ec && ec != boost::asio::error::not_connected) {
		std::cerr << ec.message() << std::endl;
	}

	//cancel the receive timer
	receive_delay_timer_.cancel();
	PackMessageList(shared_from_this());

	socket_.close();

	//remove self from the queue which is hold by the current thread
	thread_.session_queue().Remove(id_);
}
} // namespace
