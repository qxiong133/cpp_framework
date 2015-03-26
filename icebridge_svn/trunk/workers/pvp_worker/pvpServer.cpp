#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include "Timer.h"
#include "LockingQueue.h"
#include "GameHandler.h"
#include "mdwrkapi.hpp"
#include "RedisClient.h"
//#include "mysql_base.h"
#include "GameLog.h"
#include "PlayerManager.h"
#include "base_function.hpp"
#include "Defines.h"
#include "BattleMapManager.h"
#include "msgpack_struct.h"
#include "GameConfig.h"

using namespace std;


//MysqlBase* mysql_main_thread;

utils::Timer worldTimer(100, false); /**< Timer for world tics set to 100 ms */
int worldTime = 0; /**< Current world time in 100ms ticks */
bool running = true; /**< Determines if server keeps running */

//消息队列
LockingQueue<msgData>* msgQueue;

//在线玩家, <uid, ctime>
std::map<unsigned int, long>* onlinePlayers;

/** Callback used when SIGQUIT signal is received. */
void closeGracefully(int) {
    running = false;
}

void handleOriginalMsg(const char* data, size_t size) {
    std::cout << "data = " << data << " size = " << size << endl;
    std::string tmpStr;
    tmpStr.assign(data, size);
    msgData tmpData;
    tmpData.len = size;
    tmpData.data = tmpStr;
    msgQueue->push(tmpData);
}

mdwrk* msgWorker;
WorkerSender* msgSender;
RedisClient* mRedis;

int main() {
    //mysql_main_thread = MysqlBase::get_instance(5);
    mRedis = new RedisClient(GameConfig::getInstance().getString("redisMaster.ip", "127.0.0.1"), GameConfig::getInstance().getInt("redisMaster.port", 6379));
    //worker name; tcp address; 服务名称; 消息处理句柄
    msgWorker = mdwrk::getInstance("pvpServer01", GameConfig::getInstance().getString("router.address"), "pvpServer", 1, handleOriginalMsg);
    msgSender = new WorkerSender(msgWorker);
    onlinePlayers = new std::map<unsigned int, long>();
    msgQueue = new LockingQueue<msgData>();
    GameHandler *gameHandler = new GameHandler(msgQueue, mRedis);

    //==================test =======================
    /*std::vector<soldier> soldiers;
    bool ret = mRedis->getSoldiersByPlayerID(132, soldiers);
    std::cout << ret << std::endl;*/

    //=====================test===================
    int elapsedWorldTicks;
    // Initialize world timer
    worldTimer.start();
    while (running) {
        elapsedWorldTicks = worldTimer.poll();
        if (elapsedWorldTicks > 0) {
            worldTime += elapsedWorldTicks;

            if (elapsedWorldTicks > 1) {
                //"Not enough time to calculate "
            };
            // Print world time at 10 second intervals to show we're alive
            if (worldTime % 100 == 0) {
                //TODO
                cout << worldTime << endl;
            }
            gameHandler->handleMsg();
            updateBattleMaps();
            //gameHandler->handleMsg();
        } else {
            worldTimer.sleep();
        }
    }
}