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
#include "_world_map.h"
#include <mdwrkapi.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <mysql_base.h>
#include <RandomUtil.h>

//MysqlBase* mysql_main_thread;

int main(int argc, char *argv [])
{
	RandomUtil::GenerateLotsOfRandom();
    WorldMapBase& worldmap = WorldMapBase::GetInstance();
    worldmap.run();
    return 0;
}
