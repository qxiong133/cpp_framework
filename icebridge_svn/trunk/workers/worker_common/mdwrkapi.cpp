#include "mdwrkapi.hpp"


void * thread_recv_run(void * arg){
    // the socket recv message from main thread and send to the broker
    mdwrk * worker =  static_cast<mdwrk*>(arg);  
    zmq::socket_t* socket_sender= new zmq::socket_t (*(worker->m_context), ZMQ_PULL);
    socket_sender->connect("inproc://pair");

    //  Format and send the reply if we were provided one
    worker->m_expect_reply = true;

    while (!s_interrupted) {
        zmq::pollitem_t items [] = {
            { *(worker->m_worker),  0, ZMQ_POLLIN, 0 } ,
            { *socket_sender,  0, ZMQ_POLLIN, 0 } 
        };
        zmq::poll (items, 2, worker->m_heartbeat);

        if (items [0].revents & ZMQ_POLLIN) {
            zmsg *msg = new zmsg(*(worker->m_worker));
            worker->m_liveness = s_clock () + 10 * (worker->m_heartbeat);

            //  Don't try to handle errors, just assert noisily
            assert (msg->parts () >= 3);

            std::basic_string<unsigned char> empty = msg->pop_front ();
            assert (empty.compare((unsigned char *)"") == 0);
            //assert (strcmp (empty, "") == 0);
            //free (empty);

            std::basic_string<unsigned char> header = msg->pop_front ();
            assert (header.compare((unsigned char *)MDPW_WORKER) == 0);
            //free (header);

            std::string command = (char*) msg->pop_front ().c_str();
            if (command.compare (MDPW_REQUEST) == 0) {
                //  We should pop and save as many addresses as there are
                //  up to a null part, but for now, just save one...
                if (worker->m_verbose) {
                    s_console ("I: received REQUEST from broker:");
                    msg->dump ();
                }
                worker->m_reply_to = msg->unwrap ();
                size_t len = 0 ;
                const char * data= (const char *)(msg->body_data(len));
                worker->m_handler(data, len);
           }
            else if (command.compare (MDPW_HEARTBEAT) == 0) {
                //  Do nothing for heartbeats
            }
            else if (command.compare (MDPW_DISCONNECT) == 0) {
                // add to reconnect to broker
                worker->delete_connect_to_broker();
                worker->connect_to_broker ();
            }
            else {
                s_console ("E: invalid input message (%d)",
                            (int) *(command.c_str()));
                msg->dump ();
            }
            if(msg){
                delete msg;
                msg = NULL;
            }
        }else if(items [1].revents & ZMQ_POLLIN){
            zmsg *msg = new zmsg(*socket_sender);
            worker->__send(msg);
        }
        else
          if (s_clock () > worker->m_liveness) {
              if (worker->m_verbose) {
                  s_console ("W: disconnected from broker - retrying...");
              }
              s_sleep (worker->m_reconnect);
                // add to reconnect to broker
              worker->delete_connect_to_broker();
              worker->connect_to_broker ();
          }
        //  Send HEARTBEAT if it's time
        if (s_clock () > worker->m_heartbeat_at) {
            worker->send_to_broker ((char*)MDPW_HEARTBEAT, "", "", NULL);
            worker->m_heartbeat_at = s_clock () + worker->m_heartbeat;
        }
    }
    if (socket_sender){
        delete socket_sender;
        socket_sender = NULL;
    }
    if (s_interrupted)
      printf ("W: interrupt received, killing worker...\n");


}


