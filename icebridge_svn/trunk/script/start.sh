#!/bin/sh


root=`dirname $0`/..
export LD_LIBRARY_PATH=$root/lib
export GameLog=$root/conf/game.conf

sh $root/script/stop.sh

sleep 1

$root/bin/gateway > /tmp/gateway.log 2>&1 &
$root/bin/router broker01 "tcp://*:5555" -v > /tmp/router.log 2>&1 &
$root/bin/simple_worker worker01 "tcp://localhost:5555" "echo" -v > /tmp/simple_worker.log 2>&1 &
$root/bin/small_map_worker > /tmp/small_map.log 2>&1 &
$root/bin/pvp_worker > /tmp/pvp_worker.log 2>&1 &


