// =====================================================================================
// 
//       Filename:  my_filter.h
// 
//    Description:  filter
// 
//        Version:  1.0
//        Created:  2009-12-12 18:50:50
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  MY_FILTER_H_
#define  MY_FILTER_H_

#include    <numeric>
#include    <iostream>

#include    <boost/shared_ptr.hpp>
#include    <boost/make_shared.hpp>
#include    <boost/foreach.hpp>

#include    "sdk/net_message_filter_interface.h"

class MyFilter : public eddy::NetMessageFilterInterface {
 public:
	 typedef boost::uint16_t Header;
  static const size_t kHeaderSize = sizeof(Header);

  MyFilter() {
    header_read_ = false;
  }

  static boost::shared_ptr<MyFilter> Create() {
    return boost::make_shared<MyFilter>();
  }

  struct AddMessageSize {
    size_t operator()(size_t sum, const eddy::NetMessage& message) {
      return sum + kHeaderSize + message.size();
    }
  };

  virtual size_t BytesWannaWrite(eddy::NetMessageVector& messages_to_be_sent) {
    if (messages_to_be_sent.empty())
      return 0;

    return accumulate(messages_to_be_sent.begin(), messages_to_be_sent.end(),
                      0, AddMessageSize());
  }

  virtual size_t Write(eddy::NetMessageVector& messages_to_be_sent, 
                       Buffer& buffer) {
    size_t result = 0;
    BOOST_FOREACH(eddy::NetMessage& message, messages_to_be_sent)
    {
      Header header = message.size();

      buffer.insert(buffer.end(), 
                    reinterpret_cast<const char*>(&header), 
                    reinterpret_cast<const char*>(&header) + sizeof(Header));

      buffer.insert(buffer.end(), message.begin(), message.end());

      size_t bytes_writed = (message.size() + kHeaderSize);
      result      += bytes_writed;
    }

    messages_to_be_sent.clear();
    return result;
  }

  virtual size_t BytesWannaRead() {
    if (!header_read_)
      return kHeaderSize;

    return header_;
  }

  virtual size_t Read(eddy::NetMessageVector& messages_received, 
                      const Buffer& buffer) {
    if (!header_read_) {
      header_ = *(reinterpret_cast<const Header*>(&buffer[0]));
      header_read_ = true;
      return kHeaderSize;
    } else {
      messages_received.push_back(eddy::NetMessage());
      eddy::NetMessage& message = messages_received.back();
      message.Reserve(header_);
      message.Write(&buffer[0], buffer.size());
      header_read_ = false;
      return header_;
    }
  }
 private:
  bool header_read_;
  Header header_;
};

#endif   // ----- #ifndef MY_FILTER_H_  -----
