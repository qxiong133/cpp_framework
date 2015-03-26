#!/bin/sh


root=`dirname $0`/..

#ps -ef | grep redis-server | grep -v 'grep' |awk '{print $2;}' | xargs -i -t kill -9 {}

#cd /home/worker/ori_redis/bin && ./redis-server ../redis.conf &

cd $root/conf;
for file in `find . -name "*lua"`
do

        echo $file;
        /home/worker/ori_redis/bin/redis-cli  SCRIPT LOAD "$(cat $file)";

done

#./redis-cli SCRIPT LOAD "$(cat finishCreateSoldier.lua)"


