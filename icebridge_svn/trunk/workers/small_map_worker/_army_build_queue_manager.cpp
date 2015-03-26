

#include "_army_build_queue_manager.h"
#include "_small_map.h"


extern  vector<int>  combat_union_limit[6];


TimerQueue::TimerQueue(asio::io_service &io):timer(io), is_block(false)/*,begin_build_time(second_clock::universal_time())*/
{

}

void TimerQueue::push_back(short soldier_type, short soldier_level, short soldier_num, string which_camp, OPERATION operation){

    shared_ptr<SoldierInfoRequest> sir(new SoldierInfoRequest());
    sir->soldier_type = soldier_type;
    sir->soldier_level = soldier_level;
    sir->soldier_num = soldier_num;
    sir->which_camp = which_camp;
    sir->operation =(OPERATION) operation;
    armys_queue.push_back(sir); 

}



ArmysBuildQueue::ArmysBuildQueue(unsigned int uid, SmallMapBase *sm):m_sm(sm){
    //用户登陆时候促发
    /* 从redis读取这个uid的建造队列，计算队列的时间，完成的兵写到redis，没完成的兵开始挂定时器 */
    m_uid = uid; 
    char tmp[150];
    sprintf(tmp,"keys camp#%d* ", m_uid);
    std::vector<std::string> output;
    if(m_sm->redis->multiReadExec(tmp, output)){
        //caculate the timer and insert TimerQueue to camp_queue;
        std::vector<std::string> camp_info;
        for(int i=0; i < output.size(); ++i){
            if(readBuildArmy(output[i], camp_info)){
                newTimerQueueFromRedis(output[i], camp_info); // new a TimerQueue and insert into camp_queue
            }
            camp_info.clear();
        }
    }
}


ArmysBuildQueue::~ArmysBuildQueue(){
    //用户下线的时候促发
    /*取消定时器，把时间和队列写入到redis中*/
    for(hash_map::iterator it = camp_queue.begin(); it != camp_queue.end(); ++it ){
        cancelTimer(it->first);
        writeIntoRedis( it->first, to_iso_string(it->second->begin_build_time), it->second->armys_queue, it->second->is_block);  
    }
}


void ArmysBuildQueue::writeIntoRedis(string which_camp, std::string iso_time_string, vector<shared_ptr<SoldierInfoRequest> > &armys_queue, bool is_block){
    char tmp[150];
    sprintf(tmp,"lpush camp#%d#%s %s %d", m_uid, which_camp.c_str(), iso_time_string.c_str(), is_block);
    string command(tmp);

    for(TimerQueue::iterator_armys_queue it = armys_queue.begin();
                it !=  armys_queue.end();
                ++it){
        sprintf(tmp," %d#%d#%d", (*it)->soldier_type, (*it)->soldier_level, (*it)->soldier_num);
        command.append(tmp);
    }

    sprintf(tmp,"del camp#%d#%s", m_uid, which_camp.c_str());

    //if( ! armys_queue.empty()){
      //armys_queue.clear();
      m_sm->redis->strWriteExec(tmp);
      m_sm->redis->strWriteExec(command);
    //}
}

bool ArmysBuildQueue::readBuildArmy(std::string &camp_id, std::vector<std::string> &output){
    char tmp[150];
    sprintf(tmp,"lrange %s 0 -1", camp_id.c_str());
    return m_sm->redis->multiReadExec(tmp, output);
}


void ArmysBuildQueue::completeAtOnce(string which_camp, int diamond_num){
    //use camp_id get the building list
    //caculate time and diamond
    //sub diamond and inster soldier
    //send response
    bool db_operation = true;
    if(camp_queue.find(which_camp) == camp_queue.end()) return -1;

    int build_total_second = 0;
    for(TimerQueue::iterator_armys_queue it = camp_queue[which_camp]->armys_queue.begin();it !=  camp_queue[which_camp]->armys_queue.end();
                ++it){
        build_total_second += (*it)->soldier_num * m_sm->_arms[(*it)->soldier_type][(*it)->soldier_level].build_time;
    }

    ptime now = second_clock::universal_time();
    time_duration td = now - camp_queue[which_camp]->begin_build_time;
    int accelerate_time = build_total_second - td.seconds();
    int diamond_to_sub = (accelerate_time/(60*5)) + 1;

    if(!(m_sm->poco_mysql->subDiamond(m_uid, diamond_to_sub))){
      diamond_to_sub=0;
      db_operation = false;
      cout << "sub diamond from db error,but we have insert soldier into builded queue"  << endl;
    }

    CompleteAtOnceResponse res;
    res.which_camp = which_camp;

    if(db_operation){
        for(TimerQueue::iterator_armys_queue it = camp_queue[which_camp]->armys_queue.begin();it !=  camp_queue[which_camp]->armys_queue.end();
                    ++it){
            //insert soldier into builded army queue
            int soldier_id = -1;
            for(int i=0; i < (*it)->soldier_num ; i++){
                if(insertToBuildedArmyQueue((*it)->soldier_type, (*it)->soldier_level, soldier_id)){
                    char tmp[120];
                    sprintf(tmp,"%d", soldier_id);
                    soldier s;
                    s.category = (*it)->soldier_type;
                    s.level = (*it)->soldier_level;
                    s.sid = tmp;
                    s.camp_uuid = "";
                    res.soldiers_info.push_back(s);
                    continue;
                }
                cout << "insert into builded error in complete at once fuction"  << endl;
                break;
            }
        }


        cancelTimer(which_camp);
        camp_queue[which_camp]->armys_queue.clear();
        camp_queue[which_camp]->begin_build_time = now;
        camp_queue[which_camp]->is_block = false;
        writeIntoRedis(which_camp, to_iso_string(camp_queue[which_camp]->begin_build_time), camp_queue[which_camp]->armys_queue, camp_queue[which_camp]->is_block);
    }

    res.diamond_num = diamond_to_sub;
    m_sm->sender->SendToPlayer( res, CompleteAtOnceResponse_ID, m_uid);
}


void ArmysBuildQueue::getCampInfo(){
    CampInfoResponse res;
    for(hash_map::iterator it = camp_queue.begin(); it != camp_queue.end(); ++it ){
        // if is_block is true  rest
        VectorCampInfoResponse tmp;
        res.camps_info.insert(make_pair(it->first, tmp));
        res.camps_info[it->first].camp_info = getSoldiersVector(it->second->armys_queue);
        res.camps_info[it->first].time_remain= time_remain(it->first);
        printf("camps_info map first %s\n",it->first.c_str());
        cout << "camps_info map time remain"  << res.camps_info[it->first].time_remain << endl;
        for(vector<CampInfo>::iterator itr = res.camps_info[it->first].camp_info.begin(); itr != res.camps_info[it->first].camp_info.end(); ++itr){
            cout << "camps_info detail type "  << (*itr).soldier_type << endl;
            cout << "camps_info detail level "  << (*itr).soldier_level << endl;
            cout << "camps_info detail num "  << (*itr).soldier_num << endl;
        }

    }

    m_sm->sender->SendToPlayer( res, CampInfoResponse_ID, m_uid);
}

void ArmysBuildQueue::getBuildedSoldiersInfo(){
    soldiersInfoResponse res;
    m_sm->redis->getSoldiersByPlayerID(m_uid, res.soldier_info);
    m_sm->sender->SendToPlayer( res, soldiersInfoResponse_ID, m_uid);
}

vector<CampInfo> ArmysBuildQueue::getSoldiersVector(vector<shared_ptr<SoldierInfoRequest> > &armys_queue){
    vector<CampInfo> v;
    CampInfo tmp;
    for(TimerQueue::iterator_armys_queue it = armys_queue.begin();
                it !=  armys_queue.end();
                ++it){
        tmp.soldier_type =  (*it)->soldier_type;
        tmp.soldier_level =  (*it)->soldier_level;
        tmp.soldier_num =  (*it)->soldier_num;
        v.push_back(tmp);
    }
    return v;
}


int ArmysBuildQueue::time_remain(string which_camp){

    if(camp_queue.find(which_camp) == camp_queue.end()) return -1;
    if(camp_queue[which_camp]->is_block) return 0;
    if(camp_queue[which_camp]->armys_queue.empty()) return 0;

    shared_ptr<SoldierInfoRequest> sir_ptr = camp_queue[which_camp]->armys_queue.front();

    ptime now = second_clock::universal_time();
    ptime time_up = camp_queue[which_camp]->begin_build_time + seconds(m_sm->_arms[sir_ptr->soldier_type][sir_ptr->soldier_level].build_time);

    time_duration td = time_up - now;
    return td.total_seconds(); 

}

bool ArmysBuildQueue::soldier_is_unlock(short soldier_type, short soldier_level){
    char tmp[130];
    sprintf(tmp,"hget user#%d combat_union_uid", m_uid);
    std::string output;
    m_sm->redis->strReadExec(tmp,output);
    if (output.empty()) return false;
    int combat_union_id = atoi(output.c_str());
    if (combat_union_id >= 6) {
        printf("combat_union_id is greate than 6, combat_union_id is %d",combat_union_id);
        return false;
    }
    return (soldier_level <= combat_union_limit[combat_union_id][soldier_type]);

}

void ArmysBuildQueue::newTimerQueueFromRedis(std::string &camp_string, std::vector<std::string> &camp_info){
    //camp_string=camp#uid#1
    //key(camp#uid#camp_id) ---> value(ptime_iso_string->soldier_type#soldier_level#soldier_num->...)
    std::string key("#");

    unsigned found = camp_string.rfind(key);
    string camp_id = "";
    if (found != std::string::npos)
        camp_id = camp_string.substr(found+1);
    if (camp_info.size() < 3 || camp_id.empty()){
        printf("camp_id is wrong: %s\n" ,camp_string.c_str());
        return;
    }

    ptime now = second_clock::universal_time();
    ptime last(from_iso_string(camp_info[camp_info.size() - 1]));

    time_duration td = now - last;
         
    vector< string > split_vec; 
    timer_queue_ptr tq(new TimerQueue(m_sm->io));
    camp_queue[camp_id] = tq;
    tq->is_block = atoi(camp_info[camp_info.size() - 2].c_str());
    bool mark_insert_directly = false;
    if (tq->is_block)
      mark_insert_directly = true;
    else
      mark_insert_directly = false;
    //caclulate time if td is positive, add the soldier to the army list
    //else update begin_build_time = now() - duration and hang up timer at begin_build_time + soldier_build_time 
    for(int i=(camp_info.size() - 3); i >= 0  ; --i ){
        split_vec.clear(); 
        split( split_vec, camp_info[i], is_any_of("#"), token_compress_on );
        if (split_vec.size() != 3) continue;
        short soldier_type = atoi(split_vec[0].c_str());
        short soldier_level = atoi(split_vec[1].c_str());
        short soldier_num = atoi(split_vec[2].c_str());
        if ( !soldier_type || !soldier_level || !soldier_num) continue;
        if(mark_insert_directly){
           tq->push_back(soldier_type, soldier_level, soldier_num, camp_id, ADD);
        }else{
            for(; soldier_num >= 0 ; ){
                if ((td - seconds(m_sm->_arms[soldier_type][soldier_level].build_time)).is_negative() ){
                    // if td is negative ,
                    mark_insert_directly = true;

                    tq->push_back(soldier_type, soldier_level, soldier_num, camp_id, ADD);
                    ptime begin_build_time = now - td;
                    registerTime( camp_id, begin_build_time);
                    break;
                }
                //todo insert batch
                td = td - seconds(m_sm->_arms[soldier_type][soldier_level].build_time);
                int soldier_id = -1;
                if(insertToBuildedArmyQueue(soldier_type, soldier_level, soldier_id)){
                  tq->is_block = false;
                  --soldier_num;
                }
                else{
                // if population is full, we put all the data to the timer_queue
                    mark_insert_directly = true;
                    tq->is_block = true;

                    tq->push_back(soldier_type, soldier_level, soldier_num, camp_id, ADD);
                    break;
                    //registerTimer()
                    //tq->begin_build_time = second_clock::universal_time();
                }
            } 
        }
    }

    if (tq->is_block){
        registerTime( camp_id, now, 30);
    }

}


bool ArmysBuildQueue::insertToBuildedArmyQueue(short soldier_type, short soldier_level, int &soldier_id){
//todo
   if( soldier_type >= m_sm->_arms.size() || soldier_level >= m_sm->_arms[soldier_type].size()) return false;
    ArmsResponse m_arms =  m_sm->_arms[soldier_type][soldier_level];
    short population = m_arms.population; 
    return m_sm->redis->insertToBuildedArmyQueue(m_uid, soldier_type, soldier_level, population, soldier_id);
}



bool ArmysBuildQueue::subResource(int &wood, int &stone , int &iron, int &concrete ){

    bool ret = m_sm->redis->manPlayerResources(m_uid, 2, wood, stone, iron, concrete);

    std::vector<std::string> output;
    m_sm->redis->getPlayerResources(m_uid, output);                                       
    if(output.size() != 4){
        wood = 0;
        stone = 0;
        iron = 0;
        concrete = 0;
    }else{
        wood = atoi(output[0].c_str());
        stone = atoi(output[1].c_str());
        iron = atoi(output[2].c_str());
        concrete = atoi(output[3].c_str());
    }
    return ret;
}

bool ArmysBuildQueue::addResource(int &wood, int &stone , int &iron, int &concrete ){

    bool ret = m_sm->redis->manPlayerResources(m_uid, 1, wood, stone, iron, concrete);
    std::vector<std::string> output;
    m_sm->redis->getPlayerResources(m_uid, output);                                       
    if(output.size() != 4){
        wood = 0;
        stone = 0;
        iron = 0;
        concrete = 0;
    }else{
        wood = atoi(output[0].c_str());
        stone = atoi(output[1].c_str());
        iron = atoi(output[2].c_str());
        concrete = atoi(output[3].c_str());
    }
    return ret;
}





int ArmysBuildQueue::addSoldiers(short soldier_type, short soldier_level, short soldier_num, string which_camp){
    //if the timer_status is false,insert the army and registerTime
    //if army_type in the queue add num
    //else insert the army_type to the end of the list

    //todo here is a bug
    //here not judge soldier_level, so we add hight level to the lower level queue
    //if we judge soldier_level, we also need to modified delSoliders logic
    /*
       1.解锁
       2.扣资源 bool subResource(int wood, int stone , int iron, int concrete ) 

       3.如果队列是空,挂载定时器
       4.时间到, 人口够，继续挂载定时器，或者队列空，定时器取消.如果人口不够，30秒的轮询, 更新begin_build_time

        bool insertToBuildedArmyQueue(short soldier_type, short soldier_level, short population);
        bool addResource(int wood, int stone , int iron, int concrete ) 
    */

    printf("enter addSoldiers");

    if(camp_queue.find(which_camp) == camp_queue.end()) { 
        timer_queue_ptr tq(new TimerQueue(m_sm->io));
        camp_queue[which_camp] = tq;
    }

   if( soldier_type >= m_sm->_arms.size() || soldier_level >= m_sm->_arms[soldier_type].size()) return 0;
   if( soldier_is_unlock(soldier_type, soldier_level) ) return 0;
    ArmsResponse m_arms =  m_sm->_arms[soldier_type][soldier_level];

    int wood = m_arms.need_wood * soldier_num;
    int stone = m_arms.need_stone * soldier_num;
    int iron = m_arms.need_iron * soldier_num;
    int concrete = m_arms.need_soil * soldier_num;

    if(!subResource(wood, stone , iron, concrete)) {

		printf("subResource fail\n");
        sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, ADD, false, wood,  stone, iron, concrete); 
        return 0; 
    }

    printf("subResource success\n");

    for(TimerQueue::iterator_armys_queue it = camp_queue[which_camp]->armys_queue.begin();
                it !=  camp_queue[which_camp]->armys_queue.end();
                ++it){
        if((*it)->soldier_type == soldier_type && (*it)->soldier_level == soldier_level){
            (*it)->soldier_num += soldier_num;
            printf("add num to a existed camp\n");
            sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, ADD, true, wood,  stone, iron, concrete); 
            return soldier_num;
        }
    }

    bool is_empty = camp_queue[which_camp]->armys_queue.empty();
    camp_queue[which_camp]->push_back(soldier_type, soldier_level, soldier_num, which_camp, ADD);
    printf("push back a struct to the camp queue\n");
    if(is_empty) registerTime( which_camp, second_clock::universal_time());
    sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, ADD, true, wood,  stone, iron, concrete); 
    return soldier_num;
}



int ArmysBuildQueue::delSoliders(short soldier_type, short soldier_level, short soldier_num, string which_camp){
    //delete the soldier in the queue
    //if the soldier is the first one,cancel the timer then build the next army_type 
    //else delete the num of the army_type
   if( soldier_type >= m_sm->_arms.size() || soldier_level >= m_sm->_arms[soldier_type].size()) return 0;

    printf("enter delSoldier\n");

   int wood = 0;
   int stone = 0;
   int iron = 0;
   int concrete = 0;


   if(camp_queue.find(which_camp) == camp_queue.end()){ 
        //addResource(wood, stone , iron, concrete ); 
       sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, DEL, false, wood,  stone, iron, concrete); 
       return 0;
   }
   ArmsResponse m_arms =  m_sm->_arms[soldier_type][soldier_level];

   wood = (m_arms.need_wood/2) * soldier_num;
   stone = (m_arms.need_stone/2) * soldier_num;
   iron = (m_arms.need_iron/2) * soldier_num;
   concrete = (m_arms.need_soil/2) * soldier_num;

   TimerQueue::iterator_armys_queue it = camp_queue[which_camp]->armys_queue.begin();
   if( it != camp_queue[which_camp]->armys_queue.end() && (*it)->soldier_type == soldier_type && soldier_level == (*it)->soldier_level ){
       if((*it)->soldier_num - soldier_num <= 0){
           cancelTimer(which_camp);
           camp_queue[which_camp]->armys_queue.erase(it);
           registerTime( which_camp, second_clock::universal_time());

           printf("delSoldier delete the first struct\n");
       }else{
           (*it)->soldier_num -= soldier_num; 
           printf("delSoldier sub a num from the first struct\n");
       }
       addResource(wood, stone , iron, concrete);
       sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, DEL, true, wood,  stone, iron, concrete); 
       return soldier_num;
   }

   if(camp_queue[which_camp]->armys_queue.size() > 1){

       for(TimerQueue::iterator_armys_queue itr = camp_queue[which_camp]->armys_queue.begin();
                   itr !=  camp_queue[which_camp]->armys_queue.end();
                   ++itr){
           if((*itr)->soldier_type == soldier_type && (*itr)->soldier_level == soldier_level ){
               (*itr)->soldier_num -= soldier_num;
               if ((*itr)->soldier_num <= 0){
                   camp_queue[which_camp]->armys_queue.erase(itr);
                   printf("delSoldier delete a struct is not the first struct\n");
                   itr--;
               }
               addResource(wood, stone , iron, concrete ); 
               sendProcessResultToUser(soldier_type, soldier_level, soldier_num, which_camp, DEL, true, wood,  stone, iron, concrete); 
               return soldier_num;
           }
       }
   }
    return 0;
}

void ArmysBuildQueue::cancelTimer(string which_camp){
    //cancel the timer
    if(camp_queue.find(which_camp) == camp_queue.end()) return;
    if(camp_queue[which_camp]->armys_queue.empty()) return;
    camp_queue[which_camp]->timer.cancel();
}

void ArmysBuildQueue::registerTime(string which_camp, ptime begin_build_time, int secs){
    //  if the queue is empty,do nothing
    //  if the timer is false,register and set timer_status is true
    //  update begin_build_time
    //
    if(camp_queue[which_camp]->armys_queue.empty()) return;
    printf("enter registerTime\n");

    shared_ptr<SoldierInfoRequest> sir_ptr = camp_queue[which_camp]->armys_queue.front();
    if(begin_build_time.is_not_a_date_time()){
        cout << "is not a date time" << endl;
        camp_queue[which_camp]->begin_build_time = camp_queue[which_camp]->timer.expires_at();
    }else{
        cout << "is a date time" << endl;
        camp_queue[which_camp]->begin_build_time = begin_build_time;
    }
        cout << "secs is " << secs << endl;
    if(secs == 0){
        camp_queue[which_camp]->timer.expires_at(camp_queue[which_camp]->begin_build_time + seconds(m_sm->_arms[sir_ptr->soldier_type][sir_ptr->soldier_level].build_time));
        cout << "wait time is" << m_sm->_arms[sir_ptr->soldier_type][sir_ptr->soldier_level].build_time << endl;
        cout << "time is up at " << to_simple_string(camp_queue[which_camp]->begin_build_time + seconds(m_sm->_arms[sir_ptr->soldier_type][sir_ptr->soldier_level].build_time)) << endl;
        //printf("wait time is %d\n",m_sm->_arms[sir_ptr->soldier_type][sir_ptr->soldier_level].build_time);
    }
    else{
        camp_queue[which_camp]->timer.expires_at(camp_queue[which_camp]->begin_build_time + seconds(secs));
    }
    //camp_queue[which_camp]->timer.expires_at(second_clock::universal_time()+ seconds(5));
    camp_queue[which_camp]->timer.async_wait(boost::bind(&ArmysBuildQueue::timeIsUp, this, boost::asio::placeholders::error, which_camp));

}


void ArmysBuildQueue::timeIsUp(const boost::system::error_code& err, string which_camp){
    //delete num of the first soldier type, don't call delSoliders
    //send message
    if(err){
        cout << "timer is cancel" << endl;
        return;
    }


    if(camp_queue.find(which_camp) == camp_queue.end()) return;
    if(camp_queue[which_camp]->armys_queue.empty()) return;


    printf("camp id is %s is time up\n", which_camp.c_str());
    shared_ptr<SoldierInfoRequest> sir_ptr = camp_queue[which_camp]->armys_queue.front();
    int soldier_id = -1;
    if(insertToBuildedArmyQueue(sir_ptr->soldier_type, sir_ptr->soldier_level, soldier_id)){
        camp_queue[which_camp]->is_block = false;
        char tmp[120];
        sprintf(tmp,"%d", soldier_id);
        if(sir_ptr->soldier_num == 1) 
          camp_queue[which_camp]->armys_queue.erase(camp_queue[which_camp]->armys_queue.begin());
        else
          (sir_ptr->soldier_num)--;

        sendSoldierIsComplete(sir_ptr->soldier_type, sir_ptr->soldier_level, 1, which_camp, sir_ptr->operation, tmp);

        if(camp_queue[which_camp]->armys_queue.empty()) return;

        registerTime(which_camp);
    }else{
        camp_queue[which_camp]->is_block = true;
        // every 30 second read redis,judge the uid total population is ready
        // update camp_queue[which_camp]->begin_build_time = now
        registerTime(which_camp, not_a_date_time, 30);
    }

}

void ArmysBuildQueue::sendProcessResultToUser(short soldier_type, short soldier_level, short soldier_num, string which_camp, short operation, bool status, int wood, int stone, int iron, int concrete){ 
    

    printf("send result to the user");

    if(camp_queue.find(which_camp) != camp_queue.end()) {
    writeIntoRedis(which_camp, to_iso_string(camp_queue[which_camp]->begin_build_time), camp_queue[which_camp]->armys_queue, camp_queue[which_camp]->is_block);  
    }

    SoldierProcessResponse res;
    res.soldier_type = soldier_type;
    res.soldier_level = soldier_level;
    res.soldier_num = soldier_num;
    res.which_camp = which_camp;
    res.operation = operation;
    res.operation_status = status;
    res.wood = wood;
    res.stone = stone;
    res.iron = iron;
    res.concrete = concrete;

	cout << "result is" << endl ;
	cout << "res.soldier_type " << res.soldier_type << endl;
	cout << "res.soldier_num" << res.soldier_num  << endl;
	cout << "res.which_camp" << res.which_camp << endl;
	cout << "res.operation" <<   res.operation << endl;
	cout << "res.operation_status" <<   res.operation_status << endl;
	cout << "res.wood " <<   res.wood << endl;
	cout << "res.stone" <<   res.stone << endl;
	cout << "res.iron" <<   res.iron << endl;
	cout << "res.concrete" <<   res.concrete << endl;

    m_sm->sender->SendToPlayer( res, SoldierProcessResponse_ID, m_uid);
}


void ArmysBuildQueue::sendSoldierIsComplete(short soldier_type, short soldier_level, short soldier_num, string which_camp, short operation, string soldier_id){ 

    printf("time is up send resutl to user");

    writeIntoRedis(which_camp, to_iso_string(camp_queue[which_camp]->begin_build_time), camp_queue[which_camp]->armys_queue, camp_queue[which_camp]->is_block);  
    TimeIsUpResponse res; 
    res.soldier_type = soldier_type;
    res.soldier_level = soldier_level;
    res.soldier_num = soldier_num;
    res.soldier_type = soldier_type;
    res.which_camp = which_camp;
    res.operation = ADD;
    res.soldier_id = soldier_id;
	cout << "result is" << endl ;
	cout << "res.soldier_type " << res.soldier_type << endl;
	cout << "res.soldier_num" << res.soldier_num  << endl;
	cout << "res.which_camp" << res.which_camp << endl;
	cout << "res.operation" <<   res.operation << endl;
	cout << "res.soldier_id" <<   res.soldier_id << endl;
	//cout << "res.operation_status" <<   res.operation_status << endl;
	

    m_sm->sender->SendToPlayer( res, TimeIsUpResponse_ID, m_uid);
}






ArmysBuildQueueManager::ArmysBuildQueueManager(SmallMapBase *sm):m_sm(sm)
{ //read config from mysql,and fill data into army_config_table


}

void ArmysBuildQueueManager::uidOnline(unsigned int uid){
    // call findUid
    armys_build_queue_ptr abq = findUid(uid);
    abq->getCampInfo();
    abq->getBuildedSoldiersInfo();
}

void ArmysBuildQueueManager::uidOffline(unsigned int uid){
    //delete the uid in the table
    deleteUid(uid);
}

ArmysBuildQueueManager::armys_build_queue_ptr ArmysBuildQueueManager::findUid(unsigned int uid){
    //if find return it 
    //else new one and return it
    if( m_uid2buildqueue.find(uid) == m_uid2buildqueue.end() ) 
        m_uid2buildqueue[uid] = armys_build_queue_ptr(new ArmysBuildQueue(uid, m_sm));
    return m_uid2buildqueue[uid];
}

bool ArmysBuildQueueManager::deleteUid(unsigned int uid){
    //if find, we delete it,return true
    //if not find, return false
    if( m_uid2buildqueue.find(uid) == m_uid2buildqueue.end() ) 
        return false;
    m_uid2buildqueue.erase(m_uid2buildqueue.find(uid));
    return true;
}


bool ArmysBuildQueueManager::requestProcess(unsigned int uid,  shared_ptr<SoldierInfoRequest> info){
    //get the ArmysBuildQueue* and operate it.
    cout << "enter requestProcess" << endl ;
   printf("enter requestProcess\n");
 
    armys_build_queue_ptr abq = findUid(uid);
    if (info->operation == ADD){
        abq->addSoldiers(info->soldier_type, info->soldier_level, info->soldier_num, info->which_camp);
    }else if( info->operation == DEL){
        abq->delSoliders(info->soldier_type, info->soldier_level, info->soldier_num, info->which_camp);
    }
}

void ArmysBuildQueueManager::changeSoldierCampRequest(unsigned int player_uuid, ChangeSoldierCampRequest *req){
    for(vector<SoldierCampInfo >::iterator it = req->soldier_camp_info.begin(); it != req->soldier_camp_info.end(); ++it){
        if (it->camp_uuid == "")
          m_sm->redis->cancelDfdSoldier(player_uuid, it->sid);
        else
          m_sm->redis->setDfdSoldier(player_uuid, it->sid, it->camp_uuid);
    }

}

void ArmysBuildQueueManager::completeAtOnceRequest(unsigned int player_uuid, CompleteAtOnceRequest *req){
    armys_build_queue_ptr abq = findUid(player_uuid);
    abq->completeAtOnce(req->camp_id, req->diamond_num);
}




