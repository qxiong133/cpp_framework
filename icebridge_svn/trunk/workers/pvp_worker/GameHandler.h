/* 
 * File:   GameHandler.h
 * Author: Edwin Xie
 *
 * Created on 2013年12月5日, 下午4:07
 */

#ifndef GAMEHANDLER_H
#define	GAMEHANDLER_H

#include "LockingQueue.h"
#include "RedisClient.h"

struct msgData {
    size_t len;
    std::string data;
};

class GameHandler {
public:

    GameHandler(LockingQueue<msgData>* msgQueue, RedisClient* redis) : mMsgQueue(msgQueue), mRedis(redis) {
    }
    void handleMsg();
    void sendMsg();

private:
    LockingQueue<msgData>* mMsgQueue;
    RedisClient* mRedis;
};

#endif	/* GAMEHANDLER_H */

