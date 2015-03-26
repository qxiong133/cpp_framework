//
//  Majordomo Protocol broker
//  A minimal implementation of http://rfc.zeromq.org/spec:7 and spec:8
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "zmsg.hpp"
#include "mdp.h"
#include "service.hpp"

#include <map>
#include <vector>
#include <boost/unordered_set.hpp>


//  We'd normally pull these from config data

#define HEARTBEAT_LIVENESS  3       //  3-5 is reasonable
#define HEARTBEAT_INTERVAL  2500      //  msecs
#define HEARTBEAT_EXPIRY    HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS

//  This defines a single broker
class broker {
public:

   //  ---------------------------------------------------------------------
   //  Constructor for broker object

   typedef boost::unordered_set<string> clients_set;


   broker (int verbose,const char *identity)
   {
       //  Initialize broker state
       m_context = new zmq::context_t(1);
       m_socket = new zmq::socket_t(*m_context, ZMQ_ROUTER);
       m_identity = identity;
       m_socket->setsockopt(ZMQ_IDENTITY, m_identity.c_str(), m_identity.size());
       // make zeromq don't save any reqeust
       // so if the worker reconnect the broker, it will not recv the request in the queue
       // m_socket->setsockopt(ZMP_HWM, 0, sizeof (uint64_t));
       m_verbose = verbose;
       m_heartbeat_at = s_clock () + HEARTBEAT_INTERVAL;
   }

   //  ---------------------------------------------------------------------
   //  Destructor for broker object

   virtual
   ~broker ()
   {
       // broker delete all the services
       for(std::map<std::string, Service*>::iterator p = m_services.begin(); \
       p != m_services.end(); ++p ){
           delete p->second;
       }
       m_services.clear();
       // broker delete all the workers
      for(std::map<std::string, BrokerWorker*>::iterator p = m_workers.begin(); \
       p != m_workers.end(); ++p ){
           delete p->second;
       }
       m_workers.clear();
   }

   //  ---------------------------------------------------------------------
   //  Bind broker to endpoint, can call this multiple times
   //  We use a single socket for both clients and workers.

   void
   bind (std::string endpoint)
   {
       m_endpoint = endpoint;
       m_socket->bind(m_endpoint.c_str());
       s_console ("I: MDP broker/0.1.1 is active at %s", endpoint.c_str());
   }

   //  ---------------------------------------------------------------------
   //  Delete any idle workers that haven't pinged us in a while.

   /*void
   purge_workers ()
   {
       BrokerWorker * wrk = m_waiting.size()>0 ? m_waiting.front() : 0;
       while (wrk) {
           if (!wrk->expired ()) {
               break;              //  BrokerWorker is alive, we're done here
           }
           if (m_verbose) {
               s_console ("I: deleting expired worker: %s",
                     wrk->m_identity.c_str());
           }
           worker_delete (wrk, 0);
           wrk = m_waiting.size()>0 ? m_waiting.front() : 0;
       }
   }*/

   void service_delete(Service* srv){
         m_services.erase(srv->m_name);
         delete srv;
   }

   Service * service_change(Service * old_srv, std::string service_type){
       Service * new_srv = ServiceFactory::CreateService(service_type, old_srv->m_name);
       if (! new_srv) return NULL;
       new_srv->copy(old_srv);
       service_delete(old_srv);
       m_services.insert(std::make_pair(new_srv->m_name, new_srv));
       return new_srv;  

   }

   //  ---------------------------------------------------------------------
   //  Locate or create new service entry

   Service *
   service_require (std::string service_name,std::string service_type = "ServiceDoNotWaitResponse")
   {
       assert (service_name.size()>0);
       if (m_services.count(service_name)) {
          Service * srv = m_services.at(service_name);
          if(srv->m_type == service_type)
            return m_services.at(service_name);
          else{
            return service_change(srv,service_type);
          }
       } else {
           Service * srv = ServiceFactory::CreateService(service_type, service_name);
           if(!srv) return NULL;
           m_services.insert(std::make_pair(service_name, srv));
           if (m_verbose) {
               s_console ("I: received message:");
           }
           return srv;
       }
   }



   //  ---------------------------------------------------------------------
   //  Dispatch requests to waiting workers as possible

   void
   service_dispatch (Service *srv, zmsg *msg)
   {
       assert (srv);
       if (msg) {                    //  Queue message if any
           srv->m_requests.push_back(msg);
       }

       //purge_workers ();
       while (srv->m_requests.size()>0 && srv->has_actived_worker())
       {
           BrokerWorker *wrk = srv->pop_worker(); 
           if ( wrk == NULL)
                break;
           zmsg *msg_ = srv->pop_request(); 
           worker_send (wrk, (char*)MDPW_REQUEST, "", msg_);

           delete msg_;
       }
   }

   //  ---------------------------------------------------------------------
   //  Handle internal service according to 8/MMI specification

   void
   service_internal (std::string service_name, zmsg *msg)
   {
       if (service_name.compare("mmi.service") == 0) {
           Service * srv = m_services.at(msg->body());
           if (srv && srv->m_waiting.size()) {
               msg->body_set("200");
           } else {
               msg->body_set("404");
           }
       } else {
           msg->body_set("501");
       }

       //  Remove & save client return envelope and insert the
       //  protocol header and service name, then rewrap envelope.
       //char * client = (char *)msg->pop_front().c_str();
       // pop a empty frame
       //msg->pop_front();


       // it's dangerous to use a temp object
       std::string client = msg->unwrap();
       msg->wrap(MDPC_CLIENT, service_name.c_str());
       msg->wrap((char *)client.c_str(), "");
       //delete client;
       msg->send (*m_socket);
       delete msg;
   }

   //  ---------------------------------------------------------------------
   //  Creates worker if necessary

   BrokerWorker *
   worker_require (std::string identity)
   {
       //assert (identity.length()!=0);

       //  self->workers is keyed off worker identity
       if (m_workers.count(identity)) {
          return m_workers.at(identity);
       } else {
          BrokerWorker *wrk = new BrokerWorker(identity);
          m_workers.insert(std::make_pair(identity, wrk));
          if (m_verbose) {
             s_console ("I: registering new worker: %s", identity.c_str());
          }
          return wrk;
       }
   }

   //  ---------------------------------------------------------------------
   //  Deletes worker from all data structures, and destroys worker

   void
   worker_delete (BrokerWorker *&wrk, int disconnect)
   {
       assert (wrk);
       if (disconnect) {
           worker_send (wrk, (char*)MDPW_DISCONNECT, "", NULL);
       }

      /* if (wrk->m_service) {
          wrk->m_service->delete_worker(wrk);
       }
      //  This implicitly calls the worker destructor
       m_workers.erase(wrk->m_identity);
       delete wrk;*/
   }



   //  ---------------------------------------------------------------------
   //  Process message sent to us by a worker

   void worker_process (std::string sender, zmsg *msg)
   {
       assert (msg && msg->parts() >= 1);     //  At least, command

       std::string command = (char *)msg->pop_front().c_str();
       bool worker_ready = m_workers.count(sender)>0;
       BrokerWorker *wrk = worker_require (sender);

       if (command.compare (MDPW_READY) == 0) {
           if (sender.size() >= 4  //  Reserved service name
                       &&  sender.find_first_of("mmi.") == 0) {
               worker_delete (wrk, 0);
           } else {
               //  Attach worker to service and mark as idle
               std::string service_name = (char*)msg->pop_front ().c_str();
               std::string service_type = (char*)msg->pop_front ().c_str();
               wrk->m_service = service_require (service_name, service_type.size()>0 ? service_type : "ServiceDoNotWaitResponse");
               worker_waiting (wrk);
           }
       } else if (command.compare (MDPW_REPLY) == 0) {
           //  Remove & save client return envelope and insert the
           //  protocol header and service name, then rewrap envelope.
           std::string client = msg->unwrap();
           msg->wrap (MDPC_CLIENT, wrk->m_service->m_name.c_str());
           msg->wrap (client.c_str(), "");
           msg->send (*m_socket);
           if (wrk->m_service)  {              //  Not first command in session
               worker_waiting (wrk);
           }
           else {
               worker_delete (wrk, 1);
           }
       } else if (command.compare (MDPW_HEARTBEAT) == 0) {
           if (wrk->m_service)  {              //  Not first command in session
               worker_waiting (wrk);
           }
           else {
               worker_delete (wrk, 1);
           }
      } else if (command.compare (MDPW_DISCONNECT) == 0) {
           worker_delete (wrk, 1);
       } else {
           s_console ("E: invalid input message (%d)", (int) *command.c_str());
           msg->dump ();
       }
       delete msg;
   }


   void client_send(std::string client,
       char *command, std::string option)
   {
       zmsg * msg = new zmsg();
       //  Stack protocol envelope to start of message
       if (option.size()>0) {                 //  Optional frame after command
           msg->push_front ((char*)option.c_str());
       }
       msg->push_front (command);
       msg->push_front ((char*)MDPC_CLIENT);
       //  Stack routing envelope to start of message
       msg->wrap(client.c_str(), "");

       if (m_verbose) {
           s_console ("I: sending %s to client",
               mdps_commands [(int) *command]);
           msg->dump ();
       }
       msg->send (*m_socket);
       delete msg;
    }

   //  ---------------------------------------------------------------------
   //  Send message to worker
   //  If pointer to message is provided, sends that message

   void
   worker_send (BrokerWorker *worker,
       char *command, std::string option, zmsg *_msg)
   {
       //msg = (msg ? new zmsg(*msg) : new zmsg ());
       zmsg * msg = (_msg ? _msg : new zmsg());
       //  Stack protocol envelope to start of message
       if (option.size()>0) {                 //  Optional frame after command
           msg->push_front ((char*)option.c_str());
       }
       msg->push_front (command);
       msg->push_front ((char*)MDPW_WORKER);
       //  Stack routing envelope to start of message
       msg->wrap(worker->m_identity.c_str(), "");

       if (m_verbose) {
           s_console ("I: sending %s to worker",
               mdps_commands [(int) *command]);
           msg->dump ();
       }
       msg->send (*m_socket);
       if(!_msg)
         delete msg;
   }

   //  ---------------------------------------------------------------------
   //  This worker is now waiting for work

   void
   worker_waiting (BrokerWorker *worker)
   {
       assert (worker);
       //  Queue to broker and service waiting lists
       worker->m_expiry = s_clock () + (HEARTBEAT_EXPIRY * 10);
       worker->m_service->worker_waiting(worker);
       service_dispatch (worker->m_service, 0);
   }



   //  ---------------------------------------------------------------------
   //  Process a request coming from a client

   void
   client_process (std::string sender, zmsg *&msg)
   {
       assert (msg && msg->parts () >= 2);     //  Service name + body

       std::string service_name = (char *)msg->pop_front().c_str();

       // if the msg is heartbeat from clients,we add it to the m_clients
       // so we can set heartbeat to the clients
       if(service_name.compare (MDPW_HEARTBEAT) == 0){
           clients_set::iterator cli_iterator = m_clients.find(sender);
           if(cli_iterator == m_clients.end()) {
               m_clients.insert(sender);
               s_console("add the client %s to the list",sender.c_str());
           }
           s_console("m_clients size is %d",m_clients.size());
           delete msg;
           msg = NULL;
           return;
       }

       Service *srv = service_require (service_name);
       //  Set reply return address to client sender
       msg->wrap (sender.c_str(), "");
       if (service_name.length() >= 4
       &&  service_name.find_first_of("mmi.") == 0) {
           service_internal (service_name, msg);
       } else {
           service_dispatch (srv, msg);
       }
   }

   //  Get and process messages forever or until interrupted
   void
   start_brokering() {
      while (!s_interrupted) {

          zmq::pollitem_t items [] = {
              { *m_socket,  0, ZMQ_POLLIN, 0 } };
          zmq::poll (items, 1, HEARTBEAT_INTERVAL * 3);

          //  Process next input message, if any
          if (items [0].revents & ZMQ_POLLIN) {
              printf("recv a msg\n");
              zmsg *msg = new zmsg(*m_socket);
              if (m_verbose) {
                  s_console ("I: received message:");
                  msg->dump ();
              }
              std::string sender = (char*)msg->pop_front ().c_str();
              msg->pop_front (); //empty message
              std::string header = (char*)msg->pop_front ().c_str();

             if (header.compare(MDPC_CLIENT) == 0) {
                  client_process (sender, msg);
              }
              else if (header.compare(MDPW_WORKER) == 0) {
                  worker_process (sender, msg);
              }
              else {
                  s_console ("E: invalid message:");
                  msg->dump ();
                  delete msg;
              }
          }
          //send broker heartbeat to all the workers
          //send broker heartbeat to all the clients
          if (s_clock () > m_heartbeat_at) {
              //send broker heartbeat to all the workers;
              for(std::map<std::string, BrokerWorker*>::iterator p = m_workers.begin(); \
                          p != m_workers.end(); ++p ){
                  worker_send (p->second, (char*)MDPW_HEARTBEAT, "", NULL);
              }

              // send heartbeat to all the clients
              s_console("send hearbeat to the client out of for"); 
              for (clients_set::iterator p = m_clients.begin(); p != m_clients.end(); ++p){
                  s_console("send hearbeat to the client"); 
                  client_send(*p,(char*)MDPW_HEARTBEAT, "");
              }
              m_heartbeat_at = s_clock () + HEARTBEAT_INTERVAL;
          }
      }
   }

private:
    zmq::context_t * m_context;                  //  0MQ context
    zmq::socket_t * m_socket;                    //  Socket for clients & workers
    int m_verbose;                               //  Print activity to stdout
    std::string m_endpoint;                      //  Broker binds to this endpoint
    std::string m_identity;                      //  Broker name
    std::map<std::string, Service*> m_services;  //  Hash of known services
    std::map<std::string, BrokerWorker*> m_workers;    //  Hash of known workers
    clients_set m_clients;
    int64_t m_heartbeat_at;                     //  When to send HEARTBEAT
};


//  ---------------------------------------------------------------------
//  Main broker work happens here

int main (int argc, char *argv [])
{
    if (argc < 4) {
        printf ("syntax: broker broker01 endpoint -v\n");
        return 0;
    }
    char * identity = argv [1];
    char * endpoint = argv [2];

    int verbose = (argc > 1 && strcmp (argv [3], "-v") == 0);

    s_version_assert (2, 1);
    s_catch_signals ();
    broker brk(verbose, identity);
    brk.bind (endpoint);

    brk.start_brokering();

    if (s_interrupted)
        printf ("W: interrupt received, shutting down...\n");

    return 0;
}


