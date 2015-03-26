
#ifndef __borker_worker__
#define __broker_worker__


class Service;

//  This defines one worker, idle or active
class BrokerWorker {
public:
   //  ---------------------------------------------------------------------
   //  Destroy worker object, called when worker is removed from
   //  broker->workers.

   friend class broker;
   friend class Service;

   virtual
   ~BrokerWorker ()
   {
   }

   //  ---------------------------------------------------------------------
   //  Return 1 if worker has expired and must be deleted

   bool
   expired ()
   {
       return m_expiry < s_clock ();
   }

private:
    //todo if the worker is die ,should we push all the requests to the service's m_requests
    //but if we resend the request , we may process the request twice or many
    //so we now don't implement this function
    std::vector<zmsg*> m_requests;   //  List of client requests that the worker is processing
    std::string m_identity;   //  Address of worker
    Service * m_service;      //  Owning service, if known
    int64_t m_expiry;         //  Expires at unless heartbeat

    //  ---------------------------------------------------------------------
    //  Constructor is private, only used from broker
    BrokerWorker(std::string identity, Service * service = 0, int64_t expiry = 0) {
       m_identity = identity;
       m_service = service;
       m_expiry = expiry;
    }
};

#endif


