//
//  Majordomo Protocol worker example
//  Uses the mdwrk API to hide all MDP aspects
//
//  Lets us 'build mdworker' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdwrkapi.hpp"

int main (int argc, char *argv [])
{
     if (argc < 5) {
         printf ("syntax: worker worker_name broker_address server_type -v\n");
         return 0;
     }
     printf("%d\n" ,argc);
     char * identity = argv [1];
     char * broker_address = argv [2];
     char * server_type = argv [3];

    int verbose = (argc > 1 && strcmp (argv [4], "-v") == 0);
    //mdwrk session (identity, "tcp://localhost:5556", "echo", verbose);
    mdwrk session (identity, broker_address, "echo", verbose);

    zmsg *reply = 0;
    while (1) {
        zmsg *request = session.recv (reply);
        if (request == 0) {
            break;              //  Worker was interrupted
        }
        request->body_set("hehe\0hehe",9);
        reply = request;        //  Echo is complex... :-)
    }
    return 0;
}
