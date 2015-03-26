/*  =====================================================================
    mdwrkapi.hpp

    Majordomo Protocol Worker API
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

#ifndef __MDWRKAPI_HPP_INCLUDED__
#define __MDWRKAPI_HPP_INCLUDED__

#include "zmsg.hpp"
#include "mdp.h"

//  Reliability parameters
#define HEARTBEAT_LIVENESS  5000       //  3-5 is reasonable

extern void * thread_recv_run(void *);

typedef void (*handler_of_worker)(const char * data, size_t size);

//  Structure of our class
//  We access these properties only via class methods
class mdwrk {
public:

   //  ---------------------------------------------------------------------
   //  Constructor

    mdwrk (std::string worker_name, std::string broker, std::string service, int verbose, handler_of_worker handler)
    {
        s_version_assert (2, 1);

        m_handler = handler;
        m_worker_name= worker_name;
        m_broker = broker;
        m_broker = broker;
        m_service = service;
        m_context = new zmq::context_t (1);
        m_worker = 0;
        m_expect_reply = false;
        m_verbose = verbose;
        m_heartbeat = 2500;     //  msecs
        m_reconnect = 2500;     //  msecs

        s_catch_signals ();
        connect_to_broker ();
        //m_pair = new zmq::socket_t (*m_context, ZMQ_PAIR);
        m_pair = new zmq::socket_t (*m_context, ZMQ_PUSH);
        m_pair->bind("inproc://pair");
    }

    //  ---------------------------------------------------------------------
    //  Destructor

    virtual
    ~mdwrk ()
    {
		if(m_worker){
			delete m_worker;
			m_worker = NULL;
		}
		if(m_pair){
			delete m_pair;
			m_pair = NULL;
		}
		if(m_context){
			delete m_context;
			m_context = NULL;
		}
    }

    static mdwrk* getInstance(std::string worker_name, std::string broker, std::string service, int verbose, handler_of_worker handler){
        static mdwrk * self = NULL;
        static pthread_t m_recv_thread;
        if(self == NULL) {
            self = new mdwrk(worker_name, broker, service, verbose, handler); 
            pthread_create (&m_recv_thread, NULL, thread_recv_run, self);
        }
        return self;
    }

    //  ---------------------------------------------------------------------
    //  Send message to broker
    //  If no _msg is provided, creates one internally
    void send_to_broker(char *command, std::string key, std::string value, zmsg *_msg)
    {
        zmsg *msg = _msg? new zmsg(*_msg): new zmsg ();

        //  Stack protocol envelope to start of message
        if (value.length() != 0) {
            msg->push_front ((char*)value.c_str());
        }
        if (key.length() != 0) {
            msg->push_front ((char*)key.c_str());
        }
        msg->push_front (command);
        msg->push_front ((char*)MDPW_WORKER);
        msg->push_front ((char*)"");

        if (m_verbose && ((int) *command) != 4) {
            s_console ("I: sending %s to broker",
                mdps_commands [(int) *command]);
            msg->dump ();
        }
        msg->send (*m_worker);
		if(msg){
			delete msg;
			msg = NULL;
		}
    }

    //  ---------------------------------------------------------------------
    //  Connect or reconnect to broker

    void delete_connect_to_broker(){
        if(m_worker){
            delete m_worker;
            m_worker = NULL;
        }
    }


    void connect_to_broker ()
    {
        if (!m_worker) {
            m_worker = new zmq::socket_t (*m_context, ZMQ_DEALER);
            int linger = 0;
            m_worker->setsockopt(ZMQ_IDENTITY, m_worker_name.c_str(), m_worker_name.size());
            m_worker->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
            m_worker->connect (m_broker.c_str());
        }
       if (m_verbose)
            s_console ("I: connecting to broker at %s...", m_broker.c_str());

        //  Register service with broker
        send_to_broker ((char*)MDPW_READY, m_service, "ServiceDoNotWaitResponse", NULL);

        //  If liveness hits zero, queue is considered disconnected
        m_heartbeat_at = s_clock () + m_heartbeat;
        m_liveness = s_clock () + 10 * m_heartbeat;
    }


    //  ---------------------------------------------------------------------
    //  Set heartbeat delay

    void
    set_heartbeat (int heartbeat)
    {
        m_heartbeat = heartbeat;
    }


    //  ---------------------------------------------------------------------
    //  Set reconnect delay

    void
    set_reconnect (int reconnect)
    {
        m_reconnect = reconnect;
    }

    void send(const char* data, size_t len){
        zmsg *reply = new zmsg(data,len);
        reply->send(*m_pair);
		if(reply){
			delete reply;
			reply = NULL;
		}
    }

    friend void * thread_recv_run (void *arg);

private:

    void __send(zmsg * & msg){
        assert (msg|| !m_expect_reply);
        if (msg) {
            assert (m_reply_to.size()!=0);
            msg->wrap (m_reply_to.c_str(), "");
            //m_reply_to = "";
            send_to_broker ((char*)MDPW_REPLY, "", "", msg);
			if(msg){
				delete msg;
				msg = NULL;
			}
        }
    }




    std::string m_broker;
    std::string m_service;
    zmq::context_t *m_context;
    zmq::socket_t  *m_worker;     //  Socket to broker
    zmq::socket_t  *m_pair;     //  Socket for two thread communicate
    int m_verbose;                //  Print activity to stdout

    //  Heartbeat management
    int64_t m_heartbeat_at;      //  When to send HEARTBEAT
    int64_t m_liveness;            //  How many attempts left
    int m_heartbeat;              //  Heartbeat delay, msecs
    int m_reconnect;              //  Reconnect delay, msecs

    //  Internal state
    bool m_expect_reply;           //  Zero only at start

    //  Return address, if any
    std::string m_reply_to;
    std::string m_worker_name;
    handler_of_worker m_handler;
};


#endif
