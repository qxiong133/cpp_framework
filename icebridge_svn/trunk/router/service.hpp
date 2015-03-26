
#ifndef __broker_service__
#define __broker_service__

#include <string>
#include <vector>

#include "worker.hpp"

class ServiceFactory;

//  This defines a single service
class Service {
    public:
        friend class broker;
        friend class ServiceFactory;

        virtual BrokerWorker * pop_worker() = 0;


        //  ---------------------------------------------------------------------
        //  Destroy service object, called when service is removed from
        //  broker->services.
        virtual ~Service ()
        {
            for(size_t i = 0; i < m_requests.size(); i++) {
                delete m_requests[i];
            }
            m_requests.clear();
            m_waiting.clear();
        }

        Service * copy(Service * srv){
            m_requests = srv->m_requests;  
            m_waiting = srv->m_waiting;  
            for(std::vector<BrokerWorker*>::iterator it = m_waiting.begin();
                        it != m_waiting.end(); it++) {
                    (*it)->m_service = this;
            }
            srv->m_requests.clear();
            srv->m_waiting.clear();
            return this;
        }

        virtual void worker_waiting(BrokerWorker * worker){
            for(std::vector<BrokerWorker*>::iterator it = m_waiting.begin();
                        it != m_waiting.end(); it++) {
                if (*it == worker) {
                    return;    
                }
            }
            m_waiting.push_back(worker);
        }

        virtual void delete_worker(BrokerWorker *worker){
            for(std::vector<BrokerWorker*>::iterator it = m_waiting.begin();
                        it != m_waiting.end(); it++) {
                if (*it == worker) {
                    it = m_waiting.erase(it)-1;
                }
            }
        }


        virtual zmsg * pop_request(){
            zmsg * msg = NULL;
            if(m_requests.size()){
                msg =  m_requests.front();
                m_requests.erase(m_requests.begin());
            }
            return msg; 
        }


        virtual int has_actived_worker(){
            for(std::vector<BrokerWorker*>::iterator it = m_waiting.begin();
                        it != m_waiting.end(); it++) {
                if ((*it)->expired()){
                  it = m_waiting.erase(it)-1;
                }
            }
            return m_waiting.size();
        }

    protected:
        Service(std::string name)
        {
            m_name = name;
            m_type = "Service";
        }

        std::string m_name;             //  Service name
        std::string m_type;             //  Service type
        std::vector<BrokerWorker*> m_waiting;  //  List of actived workers
        std::vector<zmsg*> m_requests;   //  List of client requests


};


class ServiceWaitResponse : public Service{
    public:
        ServiceWaitResponse(std::string name)
            :Service(name)
        {
            m_type = "ServiceWaitResponse";
        }

        virtual ~ServiceWaitResponse(){
        }

        virtual BrokerWorker * pop_worker(){
            BrokerWorker *wrk = NULL;
            if(m_waiting.size()){
                wrk = m_waiting.front();
                m_waiting.erase(m_waiting.begin());
            }
            return wrk;
        }

};

class ServiceDoNotWaitResponse : public Service{
    public:
        ServiceDoNotWaitResponse(std::string name)
            :Service(name)
        {
            if_pos_init = false;
            m_type = "ServiceDoNotWaitResponse";
        }

        ~ServiceDoNotWaitResponse(){
        }


        virtual BrokerWorker * pop_worker(){
            if (m_waiting.begin() == m_waiting.end())
              return NULL;
            if( pos == m_waiting.end() || !if_pos_init)  {
                if_pos_init = true;
                pos = m_waiting.begin();
            }
            return *pos++;
        }

    private:
        std::vector<BrokerWorker*>::iterator pos;
        bool if_pos_init;

};

class ServiceFactory{
    public:
        static Service * CreateService(std::string service_type, std::string service_name){
            if (service_type == "ServiceWaitResponse")
              return new ServiceWaitResponse(service_name);
            else if (service_type == "ServiceDoNotWaitResponse")
              return new ServiceDoNotWaitResponse(service_name);
            else
              return NULL;
        }
};


#endif


