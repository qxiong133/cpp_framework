// =====================================================================================
// 
//       Filename:  proto_def.h
// 
//    Description:  define proto message
// 
//        Version:  1.0
//        Created:  2010-05-19 17:39:28
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================

#ifndef  PROTO_DEF_H_
#define  PROTO_DEF_H_

#include    "tests/proto.pb.h"

#define DECLARE_MESSAGE(x) k##x##ID,
enum ProtoMessageIDs {
  kInvalidMessageID,
#include    "tests/proto.h"
  kMaxMessageID,
};
#undef DECLARE_MESSAGE

template <typename MessageType>
class GetMessageID {
 public:
  const static ProtoMessageIDs ID = kInvalidMessageID;
};

#define DECLARE_MESSAGE(x) \
    template <> \
    class GetMessageID<proto::x> { \
     public:\
      const static ProtoMessageIDs ID = k##x##ID; \
    };
#include    "tests/proto.h"
#undef DECLARE_MESSAGE

#endif   // ----- #ifndef PROTO_DEF_H_  -----
