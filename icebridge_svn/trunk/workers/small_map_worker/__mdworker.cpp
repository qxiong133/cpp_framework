//
//  Majordomo Protocol worker example
//  Uses the mdwrk API to hide all MDP aspects
//
//  Lets us 'build mdworker' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include <string>
#include <vector>
#include <list>
#include <msgpack.hpp>
#include "_small_map.h"
#include <mdwrkapi.hpp>
#include  <boost/bind.hpp>
#include   <boost/asio/io_service.hpp>

extern MysqlBase* mysql_main_thread ;

int main (int argc, char *argv [])
{

    mysql_main_thread = MysqlBase::get_instance(1);
    printf("start main\n");
    SmallMapBase& smallmap = SmallMapBase::GetInstance();
    smallmap.run();
    return 0;
}
