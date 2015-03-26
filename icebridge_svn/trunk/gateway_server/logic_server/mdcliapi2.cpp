
#include "mdcliapi2.hpp"

extern void handler_in_gateway(const char * data, size_t size);

void * thread_recv_run_in_gateway(void * arg){
    // the socket recv message from main thread and send to the broker
    mdcli* client=  static_cast<mdcli*>(arg);  
    zmq::socket_t* socket_sender= new zmq::socket_t (*(client->m_context), ZMQ_PULL);
    socket_sender->connect("inproc://pair");
    size_t len = 0;

    client->m_items[client->m_brokers.size()] = {*socket_sender, 0, ZMQ_POLLIN, 0 };

      //  Poll socket for a reply, with timeout

    client->send_requests();
    while (!s_interrupted) {
       zmq::poll (client->m_items, client->m_brokers.size()+1, 2500);
  
       if (client->m_items[client->m_brokers.size()].revents & ZMQ_POLLIN) {
           zmsg *msg = new zmsg (*socket_sender);
           delete msg;
           client->send_requests();
       }
       //  If we got a reply, process it
       for(mdcli::brokers_map::iterator p = client->m_brokers.begin();p != client->m_brokers.end(); ++p){
           if (client->m_items[p->first].revents & ZMQ_POLLIN) {
               zmsg *msg = new zmsg (*(p->second.get_socket()));
               /*if (client->m_verbose) {
                   s_console ("I: received reply");
                   msg->dump ();
               }*/
               //  Don't try to handle errors, just assert noisily
               assert (msg->parts () >= 3);

               p->second.set_heartbeat_at(s_clock() + HEARTBEAT_INTERVAL * 1000);
               p->second.set_status(1);
               //if(client->m_verbose) s_console("m_requests size is %d",client->m_requests.size());
               if(!(client->m_requests.empty())) client->send_requests();

               assert (msg->pop_front ().length() == 0);  // empty message

               std::basic_string<unsigned char> header = msg->pop_front();
               assert (header.compare((unsigned char *)MDPC_CLIENT) == 0);

               std::basic_string<unsigned char> service = msg->pop_front();
               assert (service.compare((unsigned char *)service.c_str()) == 0);

               if(service.compare ((unsigned char *)MDPW_HEARTBEAT) == 0)
                {
                 s_console("received heartbeat");
                 if(msg){
                     delete msg;
                     msg = NULL;
                 }
                }else{
					if (client->m_verbose) {
						s_console ("I: received message");
						msg->dump ();
					}
                  const char * data = (const char *)(msg->body_data(len));
                  handler_in_gateway(data,len);
                }
           }
       }
    }

       if (s_interrupted)
           std::cout << "W: interrupt received, killing client..." << std::endl;

       if(socket_sender){
           delete socket_sender;
           socket_sender = NULL;
       }
       return 0;
   }
