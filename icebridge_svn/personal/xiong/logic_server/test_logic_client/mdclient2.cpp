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
    int verbose = (argc > 1 && strcmp (argv [1], "-v") == 0);
    mdcli session ("tcp://localhost:5555", verbose);

    int count;
    for (count = 0; count < 10000; count++) {
        zmsg * request = new zmsg("Hello\0w",7);
        /*size_t len = 0;
        cout << request->body_data(len)[6] << endl;
        cout << len << endl;*/
        session.send ("echo", request);
        delete request;
    }
    for (count = 0; count < 10000; count++) {
        zmsg *reply = session.recv ();
        /*size_t len = 0;
        cout << reply->body_data(len) << endl;
        cout << len << endl;*/
        if (reply) {
            delete reply;
        } else {
            break;              //  Interrupted by Ctrl-C
        }
    }
    std::cout << count << " replies received" << std::endl;
    return 0;
}
