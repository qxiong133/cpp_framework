set(EDDYSDK eddy_sdk)

add_executable(servertest servertest.cc)
target_link_libraries(servertest ${EDDYSDK}) 

add_executable(clienttest clienttest.cc)
target_link_libraries(clienttest ${EDDYSDK})

add_executable(boosttest boosttest.cc)
target_link_libraries(boosttest ${EDDYSDK})

add_executable(refservertest refservertest.cc)
target_link_libraries(refservertest ${EDDYSDK}) 

add_executable(refclienttest refclienttest.cc)
target_link_libraries(refclienttest ${EDDYSDK})

PROTOBUF_GENERATE_CPP(MESSAGETEST_SRCS MESSAGETEST_HDRS messagetest.proto)

add_executable(messagetest messagetest.cc ${MESSAGETEST_SRCS} ${MESSAGETEST_HDRS})
target_link_libraries(messagetest ${EDDYSDK} ${PROTOBUF_LIBRARY} ${PROTOBUF_PROTOC_LIBRARY})
if (linux)
target_link_libraries(messagetest pthread)
endif (linux)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS proto.proto)

add_executable(protoserver protoserver.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(protoserver ${EDDYSDK} ${PROTOBUF_LIBRARY} ${PROTOBUF_PROTOC_LIBRARY})
if (linux)
target_link_libraries(protoserver pthread)
endif (linux)

add_executable(protoclient protoclient.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(protoclient ${EDDYSDK} ${PROTOBUF_LIBRARY} ${PROTOBUF_PROTOC_LIBRARY})
if (linux)
target_link_libraries(protoclient pthread)
endif (linux)
