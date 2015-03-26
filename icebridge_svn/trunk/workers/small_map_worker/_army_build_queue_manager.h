
#ifndef __ARMYSBUILDQUEUEMANAGER_H__
#define __ARMYSBUILDQUEUEMANAGER_H__

#include "msgpack_struct.h"
#include "mysql_game_configbuff.h"
//#include "GameLog.h"

#include "boost/shared_ptr.hpp"
#include "boost/asio.hpp"
#include "boost/unordered_map.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <stdlib.h> 

#include <vector>

using namespace boost;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

class SmallMapBase;

enum OPERATION { DEL= -1, NONE=0, ADD=1}; 
class TimerQueue{
    public:
        TimerQueue(asio::io_service &io);

        void push_back(short soldier_type, short soldier_level, short soldier_num, string which_camp, OPERATION operation);
             
        typedef vector<shared_ptr<SoldierInfoRequest> >::iterator iterator_armys_queue;

        asio::deadline_timer timer;//hang up on io_service
        boost::posix_time::ptime begin_build_time; //the soldier build being time 
        vector<shared_ptr<SoldierInfoRequest> > armys_queue; //存储建造的兵的队列
        bool is_block;
};



class ArmysBuildQueue{
    public:
        typedef shared_ptr<TimerQueue> timer_queue_ptr;
        typedef boost::unordered_map<string, shared_ptr<TimerQueue> > hash_map;

        ArmysBuildQueue(unsigned int uid, SmallMapBase *sm);

        ~ArmysBuildQueue();

        int addSoldiers(short soldier_type, short soldier_level, short soldier_num, string which_camp);

        int delSoliders(short soldier_type, short soldier_level, short soldier_num, string which_camp);

        void cancelTimer(string which_camp);

        void registerTime(string which_camp, ptime begin_build_time = not_a_date_time, int secs = 0);


        void timeIsUp(const boost::system::error_code& err, string which_camp);

        bool readBuildArmy(std::string &camp_id, std::vector<std::string> &output);

        void newTimerQueueFromRedis(std::string &camp_string, std::vector<std::string> &camp_info);

        bool insertToBuildedArmyQueue(short soldier_type, short soldier_level, int &soldier_id);


        void writeIntoRedis(string which_camp, std::string iso_time_string, vector<shared_ptr<SoldierInfoRequest> > &armys_queue, bool is_block);

        void sendProcessResultToUser(short soldier_type, short soldier_level, short soldier_num, string which_camp, short operation, bool status, int wood, int stone, int iron, int concrete); 

        void sendSoldierIsComplete(short soldier_type, short soldier_level, short soldier_num, string which_camp, short operation, string soldier_id);
 
        bool addResource(int &wood, int &stone , int &iron, int &concrete ); 

        bool subResource(int &wood, int &stone , int &iron, int &concrete ); 

        void getCampInfo(); 

        void getBuildedSoldiersInfo(); 

        vector<CampInfo> getSoldiersVector(vector<shared_ptr<SoldierInfoRequest> > &armys_queue);

        int time_remain(string which_camp);

        bool soldier_is_unlock(short soldier_type, short soldier_level);

        void completeAtOnce(string which_camp, int diamond_num);


private:
        hash_map camp_queue; //camp's id  --> timer and the list of the soldier queue
        SmallMapBase * m_sm;
        unsigned int m_uid;
};



class ArmysBuildQueueManager{
public:
    typedef shared_ptr<ArmysBuildQueue> armys_build_queue_ptr;
    typedef  boost::unordered_map<int, shared_ptr<ArmysBuildQueue> > hash_map;

    ArmysBuildQueueManager(SmallMapBase *sm);
    
    void uidOnline(unsigned int uid);

    void uidOffline(unsigned int uid);

    armys_build_queue_ptr findUid(unsigned int uid);

    bool deleteUid(unsigned int uid);

    bool requestProcess(unsigned int uid,  shared_ptr<SoldierInfoRequest> info);

    void changeSoldierCampRequest(unsigned int player_uuid, ChangeSoldierCampRequest *req);

    void completeAtOnceRequest(unsigned int player_uuid, CompleteAtOnceRequest *req);

private:
    hash_map m_uid2buildqueue; //table manage uid --> ArmysBuildQueue*
    SmallMapBase * m_sm;
};

#endif
