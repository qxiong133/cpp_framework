#!/bin/bash

ps -ef | grep -E "router|simple_worker|gateway|small_map_worker" | awk '{print $2}' | xargs -i -t kill -9 {}
#ps -ef | grep -E "small_map_worker" | awk '{print $2}' | xargs -i -t kill -9 {}


