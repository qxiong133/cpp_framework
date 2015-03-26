/*  =====================================================================
    mdcliapi2.c

    Majordomo Protocol Client API (async version)
    Implements the MDP/Worker spec at http://rfc.zeromq.org/spec:7.

    ---------------------------------------------------------------------
    Copyright (c) 1991-2011 iMatix Corporation <www.imatix.com>
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of the ZeroMQ Guide: http://zguide.zeromq.org

    This is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or (at
    your option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program. If not, see
    <http://www.gnu.org/licenses/>.
    =====================================================================

        Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
*/

#ifndef __MDCLIAPI_HPP_INCLUDED__
#define __MDCLIAPI_HPP_INCLUDED__

#include <list>
#include <boost/unordered_map.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "zmsg.hpp"
#include "mdp.h"

//  Structure of our class
//  We access these properties only via class methods


#define HEARTBEAT_INTERVAL  2500      //  msecs

extern void * thread_recv_run_in_gateway(void *);


class Broker{
public:
    Broker(std::string broker_name, std::string endpoint):m_broker_name(broker_name),m_endpoint(endpoint)
    {
        m_status = 0;
        m_client = 0;
        m_heartbeat_at = s_clock();
    }

    ~Broker(){
        if(m_client){
            delete m_client;
            m_client = NULL;
        }
    }

    inline std::string & get_name(){
        return m_broker_name;
    }

    inline int get_status(){
        return m_status;
    }

    inline int set_status(int status){
        return m_status = status;
    }

    inline int64_t get_heartbeat_at(){
        return m_heartbeat_at;
    }

    void set_heartbeat_at(int64_t heartbeat_at){
        m_heartbeat_at = heartbeat_at;
    }

    inline std::string & get_endpoint(){
        return m_endpoint;
    }

    inline zmq::socket_t * get_socket(){
        return m_client;
    }

    void connect_to_broker(zmq::context_t * context,std::string client_name){
       if (m_client) {
           delete m_client;
            m_client = NULL;
       }
       client_name.append("_");
       client_name.append(m_broker_name);
       m_client = new zmq::socket_t (*context, ZMQ_DEALER);
       int linger = 0;
       m_client->setsockopt(ZMQ_IDENTITY, client_name.c_str(), client_name.size());
       m_client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
       m_client->connect (m_endpoint.c_str());
    }

private:
    std::string m_broker_name;
    std::string m_endpoint;
    zmq::socket_t * m_client;     //  Socket to broker
    int m_status;
    int64_t m_heartbeat_at;
};

struct request_t{
    std::string service;
    zmsg * msg;
};

typedef request_t request;





class mdcli {
public:

   //  ---------------------------------------------------------------------
   //  Constructor
   typedef boost::unordered_map<int,Broker> brokers_map;
   typedef std::list<request> zmsg_list;


   mdcli (std::string client_name,int verbose)
   {
       s_version_assert (2, 1);

       m_client_name = client_name;
       m_context = new zmq::context_t (1);
       m_verbose = verbose;
       m_timeout = 2500;           //  msecs
       m_broker_ready = 0;
       m_items = 0;
       m_request_count = 0;
       m_broker_count = 0;
       m_pair = new zmq::socket_t (*m_context, ZMQ_PUSH);
       m_pair->bind("inproc://pair");

       s_catch_signals ();

   }

   //  ---------------------------------------------------------------------
   //  Destructor

   virtual
   ~mdcli ()
   {
       if(m_items)
         free(m_items);
       if(m_context){
           delete m_context;
           m_context = NULL;
       }

       //delete m_requests
       while(!m_requests.empty()){
           request r= m_requests.front();
           delete r.msg;
           m_requests.pop_front();
       }
       m_requests.clear();
       m_brokers.clear();
       s_console("destruct self");
   }


    static mdcli* getInstance(std::string client_name,int verbose){
        static mdcli* self = NULL;
        if(self == NULL) {
            self = new mdcli(client_name, verbose); 
        }
        return self;
    }

    void start_recv(mdcli * self){
        static pthread_t m_recv_thread;
        pthread_create (&m_recv_thread, NULL, thread_recv_run_in_gateway, self);
    }

    //add broker to the brokers_map

   void add_broker(Broker &broker){
        m_brokers.insert(make_pair(m_broker_count, broker));
        ++m_broker_count;
        m_next_broker = m_brokers.begin();
   }



   //  ---------------------------------------------------------------------
   //  Connect or reconnect to broker

   void connect_to_brokers ()
   {
       s_console("m_brokers size is %d", m_brokers.size());
       m_items = (zmq::pollitem_t *)malloc((m_brokers.size()+1) * sizeof(zmq::pollitem_t));
       for(brokers_map::iterator p = m_brokers.begin();p != m_brokers.end(); ++p){
           //cout << p->first  << "---" << p->second.get_status() << endl;
           p->second.connect_to_broker(m_context, m_client_name);
           if (m_verbose)
             s_console ("I: connecting to broker at %s", p->second.get_endpoint().c_str());
           m_items[p->first] = {*(p->second.get_socket()), 0, ZMQ_POLLIN, 0 };
       }

   }


   //  ---------------------------------------------------------------------
   //  Set request timeout

   void
   set_timeout (int timeout)
   {
       m_timeout = timeout;
   }

    int send_requests(){
        // check if any broker alive
        for(brokers_map::iterator p = m_brokers.begin();p != m_brokers.end(); ++p){
            if(p->second.get_heartbeat_at() < s_clock() ){
                 p->second.set_status(0);
            }
            if(p->second.get_status() == 0){
                zmsg * not_use = new zmsg("");
                send_to_broker(p->second.get_socket(),(char*)MDPW_HEARTBEAT,not_use);
                }
            else{
                s_console("set m_broker_ready 1");
                m_broker_ready = 1;
                }
        }

        if(!m_broker_ready)
            return 0;
        // send the request to the brokers
        if(m_verbose)
            s_console("m_broker_ready");

        boost::mutex::scoped_lock lock(requests_queue_mutex);
        while(!m_requests.empty() && m_brokers.begin() != m_brokers.end()){
            if(m_next_broker->second.get_status()){
                if(m_verbose)
                  s_console("send request to broker");
                request r= m_requests.front();
                m_requests.pop_front();
                send_to_broker(m_next_broker->second.get_socket(),(char*)r.service.c_str(),r.msg);
            }
            if(++m_next_broker == m_brokers.end()) m_next_broker = m_brokers.begin();
        }
        return 0;
    }


   //  ---------------------------------------------------------------------
   //  Send request to broker
   //  Takes ownership of request message and destroys it when sent.

   void send(std::string &service, zmsg *&request_p){
        s_console("enter send function");
        m_broker_ready = 0;
        zmsg * msg = request_p;
        request_p = NULL;
        boost::mutex::scoped_lock lock(requests_queue_mutex);
        m_requests.push_back({service,msg});
        send_signal();
    }

    //send signal to the recv thread,so the thread can send m_requests to the brokers
    void send_signal(){
        zmsg * msg = new zmsg("s");
        msg->send(*m_pair);
        if(msg){
            delete msg;
            msg = NULL;
        }
    }

   void send_to_broker(zmq::socket_t * socket,char* command_or_service, zmsg* &msg){
       assert (msg);
       zmsg *request = msg;
       msg = NULL;
       //  Prefix request with protocol frames
       //  Frame 0: empty (REQ emulation)
       //  Frame 1: "MDPCxy" (six bytes, MDP/Client x.y)
       //  Frame 2: Service name (printable string)
       request->push_front (command_or_service);
       request->push_front ((char*)MDPC_CLIENT);
       request->push_front ((char*)"");
       if (m_verbose) {
           s_console ("I: call send_to_broker");
           request->dump ();
       }
       request->send (*socket);
       if(request){
           delete request;
           request = NULL;
       }
    }

    friend void * thread_recv_run_in_gateway (void *arg);

   //  ---------------------------------------------------------------------
   //  Returns the reply message or NULL if there was no reply. Does not
   //  attempt to recover from a broker failure, this is not possible
   //  without storing all unanswered requests and resending them all...

private:
   std::string m_client_name;
   zmq::context_t * m_context;
   int m_verbose;                //  Print activity to stdout
   int m_timeout;                //  Request timeout
   int m_broker_ready;           //  are any broker ready
   int m_request_count;          //  count the request
   int m_broker_count;          //  count the broker
   zmq::pollitem_t * m_items;
   brokers_map m_brokers;
   brokers_map::iterator m_next_broker;
   zmsg_list m_requests;
   boost::mutex requests_queue_mutex;
   zmq::socket_t  *m_pair;     //  Socket for two thread communicate

};

#endif
