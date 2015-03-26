// =====================================================================================
// 
//       Filename:  id_generator.h
// 
//    Description:  IDGenerator
// 
//        Version:  1.0
//        Created:  2009-12-06 16:05:39
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  ID_GENERATOR_H_
#define  ID_GENERATOR_H_

#include    <limits>
#include    <deque>

#include    <boost/optional.hpp>

namespace eddy { 

// =====================================================================================
//        Class:  IDGenerator
//  Description:  Generates unique id
// =====================================================================================
template < class T >
  class IDGenerator : public boost::noncopyable {
   public:
    // ====================  TYPEDEFS      =======================================
    typedef T IDType;

    // ====================  LIFECYCLE     =======================================
    IDGenerator(T min_id = (std::numeric_limits<T>::min()), 
                T max_id = (std::numeric_limits<T>::max()), 
                size_t threshold = 4096) 
        : min_(min_id), max_(max_id), next_(min_id), threshold_(threshold) { 
        }

    // ====================  OPERATIONS    =======================================
    boost::optional<T> Get() {
      boost::optional<T> result;

      if (pool_.size() > threshold_) {
        result.reset(pool_.front());
        pool_.pop_front();
        return result;
      }

      if (next_ <= max_) {
        result.reset(next_);
        ++next_;
        return result;
      }

      return result;
    }

    void Put(T id) {
      pool_.push_back(id);
    }

   private:
    // ====================  DATA MEMBERS  =======================================
    T min_;
    T max_;
    T next_;
    size_t threshold_;
    std::deque<T> pool_;
  }; // -----  end of template class IDGenerator  -----

}

#endif   // ----- #ifndef ID_GENERATOR_H_  -----
