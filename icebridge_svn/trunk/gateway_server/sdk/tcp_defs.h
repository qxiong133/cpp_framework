// =====================================================================================
// 
//       Filename:  tcp_defs.h
// 
//    Description:  Some definations
// 
//        Version:  1.0
//        Created:  2009-12-07 21:40:14
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  TCP_DEFS_H_
#define  TCP_DEFS_H_

#include    <boost/cstdint.hpp>

namespace eddy { 

typedef boost::uint32_t TCPSessionID;
typedef boost::uint32_t TCPIOThreadID;
const TCPSessionID kInvalidTCPSessionID = 0;

}

#endif   // ----- #ifndef TCP_DEFS_H_  -----
