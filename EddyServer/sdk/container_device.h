// =====================================================================================
// 
//       Filename:  container_device.h
// 
//    Description:  ContainerDevice, implements the device concept in
//                  boost::iostreams, from boost/iostreams/example.
// 
//        Version:  1.0
//        Created:  2010-05-14 21:12:26
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#include <algorithm>                       // copy, min
#include <iosfwd>                          // streamsize
#include <boost/iostreams/categories.hpp>  // source_tag
#include <boost/iostreams/positioning.hpp> // stream_offset

namespace eddy {
template<typename Container>
class ContainerDevice {
public:
    typedef typename Container::value_type          char_type;
    typedef boost::iostreams::seekable_device_tag   category;
    typedef boost::iostreams::stream_offset         stream_offset;
    ContainerDevice(Container& container) 
        : container_(container), pos_(0)
        { }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        using namespace std;
        streamsize amt = static_cast<streamsize>(container_.size() - pos_);
        streamsize result = (min)(n, amt);
        if (result != 0) {
            std::copy( container_.begin() + pos_, 
                       container_.begin() + pos_ + result, 
                       s );
            pos_ += result;
            return result;
        } else {
            return -1; // EOF
        }
    }
    std::streamsize write(const char_type* s, std::streamsize n)
    {
        using namespace std;
        streamsize result = 0;
        if (pos_ != container_.size()) {
            streamsize amt = 
                static_cast<streamsize>(container_.size() - pos_);
            result = (min)(n, amt);
            std::copy(s, s + result, container_.begin() + pos_);
            pos_ += result;
        }
        if (result < n) {
            container_.insert(container_.end(), s + result, s + n);
            pos_ = container_.size();
        }
        return n;
    }
    stream_offset seek(stream_offset off, std::ios_base::seekdir way)
    {
        using namespace std;

        // Determine new value of pos_
        stream_offset next;
        if (way == std::ios_base::beg) {
            next = off;
        } else if (way == std::ios_base::cur) {
            next = pos_ + off;
        } else if (way == std::ios_base::end) {
            next = container_.size() + off - 1;
        } else {
            throw ios_base::failure("bad seek direction");
        }

        // Check for errors
        if (next < 0 || next >= container_.size())
            throw ios_base::failure("bad seek offset");

        pos_ = next;
        return pos_;
    }

    Container& container() { return container_; }
private:
    typedef typename Container::size_type   size_type;
    Container&  container_;
    stream_offset   pos_;
};

} // End namespace eddy
