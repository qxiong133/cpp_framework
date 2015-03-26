// =====================================================================================
// 
//       Filename:  net_message.h
// 
//    Description:  NetMessage
// 
//        Version:  1.0
//        Created:  2009-12-04 20:11:36
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  NET_MESSAGE_H_
#define  NET_MESSAGE_H_

#include    <vector>
#include    <iosfwd>

#include    <boost/noncopyable.hpp>
#include    <boost/shared_ptr.hpp>

namespace eddy { 

// =====================================================================================
//        Class:  NetMessage
//  Description:  NetMessage
// =====================================================================================
class NetMessage {
 public:
  static const size_t kDynamicThreshold = 128;
  // ====================  TYPEDEFS      =======================================
  // STL-like iterator support
  typedef char              char_type;
  typedef char_type         value_type;
  typedef char_type*        iterator;
  typedef const char_type*  const_iterator;
  typedef size_t            size_type;

  // ====================  LIFECYCLE     =======================================
  NetMessage();

  explicit NetMessage(size_t reserved_size);

  NetMessage(const NetMessage& other);

  // ====================  ACCESSORS     =======================================
  bool              is_dynamic() const;
  // STL-like support
  size_t            size() const;
  iterator          begin();
  const_iterator    begin() const;
  iterator          end();
  const_iterator    end() const;
  bool              empty() const;
  void              clear();
  void              insert(iterator position, 
                           const_iterator first, 
                           const_iterator last);

  // ====================  OPERATIIONS   =======================================
  void  Swap(NetMessage& other);
  // Reserves space to expand the content to at least the given size.
  void  Reserve(size_t size);
  // Changes storage strategy to dynamic mode
  void  SetDynamic();
  // Writes content 
  std::streamsize Write(const char_type* data, std::streamsize n);

  // ====================  OPERATORS     =======================================
  NetMessage& operator = (const NetMessage& other);

 private:
  typedef std::vector<char_type> DynamicVector;
  // ====================  DATA MEMBERS  =======================================
  size_t      static_size_;
  char_type   static_data_[kDynamicThreshold];
  boost::shared_ptr<DynamicVector> dynamic_data_;
}; // -----  end of class NetMessage  -----

typedef std::vector<NetMessage>   NetMessageVector;

//---------------------------- impl -------------------------
inline NetMessage::NetMessage() : static_size_(0) { }


inline NetMessage::NetMessage(size_t reserved_size) : static_size_(0) {
  this->Reserve(reserved_size);
}

inline NetMessage::NetMessage(const NetMessage& other) {
  *this = other;
}

inline bool NetMessage::is_dynamic() const {
  return dynamic_data_;
}


inline size_t NetMessage::size() const {
  if (this->is_dynamic()) {
    return dynamic_data_->size();
  } else {
    return static_size_;
  }
}


inline NetMessage::iterator NetMessage::begin() {
  if (this->is_dynamic()) {
    if (dynamic_data_->empty())
      return NULL;
    else
      return &(*dynamic_data_)[0];
  } else {
    return static_data_;
  }
}


inline NetMessage::const_iterator NetMessage::begin() const {
  if (this->is_dynamic()) {
    if (dynamic_data_->empty())
      return NULL;
    else
      return &(*dynamic_data_)[0];
  } else {
    return static_data_;
  }
}


inline NetMessage::iterator NetMessage::end() {
  if (this->is_dynamic()) {
    if (dynamic_data_->empty())
      return NULL;
    else
      return &(*dynamic_data_)[0] + dynamic_data_->size();
  } else {
    return static_data_ + static_size_;;
  }
}


inline NetMessage::const_iterator NetMessage::end() const {
  if (this->is_dynamic()) {
    if (dynamic_data_->empty())
      return NULL;
    else
      return &(*dynamic_data_)[0] + dynamic_data_->size();
  } else {
    return static_data_ + static_size_;;
  }
}


inline bool NetMessage::empty() const {
  return size() == 0;
}


inline void NetMessage::clear() {
  dynamic_data_.reset();
  static_size_ = 0;
}


}

#endif   // ----- #ifndef NET_MESSAGE_H_  -----
