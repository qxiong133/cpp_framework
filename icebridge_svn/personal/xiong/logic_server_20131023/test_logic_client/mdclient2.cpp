//
//  Majordomo Protocol client example - asynchronous
//  Uses the mdcli API to hide all MDP aspects
//
//  Lets us 'build mdclient' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdcliapi2.hpp"
#include <iostream>


using namespace std;

int main (int argc, char *argv [])
{

    //todo 
    // resend request we do it in the future 
/*
    send(msg){
        brokers_is_available = 0;
        for(i=0;i < brokers.size();i++){
            if(brokers[i].update_time < now) brokers[i].status=0;
            if(brokers[i].status == 0) {send(heartbeats,brokers[i])}
            else{
                brokers_is_available = 1;
            }
        }

        request_to_send_list.push(msg);
        if(brokers_is_available){
            request = request_to_send_list.pop();
            while(request){
                ++count;
                if(brokers[count%3].status){
                    send(brokers[count%3],request);
                    //waiting_for_response.push(request,send_time,1);
                    request = request_to_send_list.pop();
                } 
            }
            ===delete==
             if(resend_time < now){
                for(i=0;i< waiting_for_response.size();i++){
                    if(waiting_for_response[i].repeat >=3){
                        request_to_delete.push(waiting_for_response[i]);
                        delete waiting_for_response[i];}
                    if(waiting_for_response[i].send_time < now && waiting_for_response[i].repeat < 3){
                        ++waiting_for_response[i].repeat; 
                        waiting_for_response[i].send_time = now + waiting_time; 
                        ++count;
                        if(brokers[count%3].status){
                            send(brokers[count%3],waiting_for_response[i].request);
                        } 
                        resend_time = now + waiting_time;
                    }

                }
            }
            ===end delete===
        }
    }

    recv(){
        msg = null;
        ===delete ====
        for(i=0;i< request_to_delete.size();i++){
            delete request_to_delete[i];
            return request_to_delete[i];
        }
        ====end delete===

        poll(router_socket);
        if(item[0] == pollin){
            msg=recv(router_socket);
            brokers[msg.source].update_time=now + 2000msec;
            brokers[msg.source].status=1;
            if(request_to_send_list.size() > 0) send();
            if(msg.server == reply){
                //waiting_for_response.delete(msg.request);
            }
            else{
            }
            return msg;
        }
    }

    code on the brokers:
        recv heartbeat from client and add client to the client_heartbeats_list;
        send heartbeat to the client_heartbeats_list; 

    */


    /* if (argc < 5) {
        printf ("syntax: broker broker01 client01 client02 -v\n");
        return 0;
    }
    char * identity = argv [1];
    char * client01 = argv [2];
    char * client02 = argv [3];

    int verbose = (argc > 1 && strcmp (argv [1], "-v") == 0);
    "tcp://localhost:5555" 
    */


    int verbose = 1;
    mdcli session ("client01",verbose);
    Broker broker("broker01","tcp://localhost:5555");
    Broker broker1("broker02","tcp://localhost:5556");
    session.add_broker(broker);
    session.add_broker(broker1);
    session.connect_to_brokers();

    int count;
    for (count = 0; count < 13; count++) {
        zmsg * request = new zmsg("Hello\0w",7);
        std::string service = "echo";
        size_t len = 0;
        //cout << request->body_data(len)[6] << endl;
        //cout << len << endl;
        cout << "send 13 request"<< endl;
        session.send (service, request);
    }
    for (count = 0; count < 13; ) {
        zmsg *reply = session.recv ();
        size_t len = 0;
        //cout << reply->body_data(len) << endl;
        //cout << len << endl;
        if (reply) {
            cout << "recv " << count <<" reply"<< endl;
            count++;
            delete reply;
        }
        if(s_interrupted) break;
    }
/*    for (count = 0; count < 100; count++) {
        zmsg * request = new zmsg("Hello\0w",7);
        std::string service = "echo";
        size_t len = 0;
        //cout << request->body_data(len)[6] << endl;
        //cout << len << endl;
        cout << "send 13 request"<< endl;
        session.send (service, request);
    }
    for (count = 0; count < 110; ) {
        zmsg *reply = session.recv ();
        size_t len = 0;
        //cout << reply->body_data(len) << endl;
        //cout << len << endl;
        cout << "recv " << count <<" reply"<< endl;
        if (reply) {
            count++;
            delete reply;
        }
        if(s_interrupted) break;
    }
*/ 
    std::cout <<  count << " replies received!!!!" << std::endl;

    return 0;
}
