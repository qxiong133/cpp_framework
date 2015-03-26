#!/bin/env python

from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm


env.hosts = ['worker@115.29.187.198']

from fabric.api import *

def host_type():
    run('uname -s')

def sync_mysql():
	local("mysqldump -h 127.0.0.1 -P8888 -u root -proot --add-drop-database --add-drop-table  --default-character-set=utf8 --database dev_game > game.sql")
	local("tar -jcf game.sql.tar.bz2 game.sql")
	local("scp game.sql.tar.bz2 %s:~/"%(env.hosts[0]))
	run("tar -jxf ~/game.sql.tar.bz2")
	run("mysql -h 127.0.0.1 -uroot -proot < ~/game.sql")

def start_redis():
	with cd("cd /home/worker/ori_redis/bin"):
		run("./redis-server ../redis.conf &")
	run("sh /home/worker/output/script/load_redis_script.sh")


def put_game_online():
	build_root = "/home/work/workspace/online/trunk"
	with lcd(build_root):
		local("svn up")
		with lcd("build"):
			local("rm -rf *")
			local("cmake ..")
			if(local("make -j 4").succeeded):
				local("make install")
				#local("echo 'work' | sudo -S supervisorctl restart all")
				local("tar -jcf output.tar.bz2 output")
				local("scp -r output.tar.bz2  %s:~/"%(env.hosts[0]))
				pass
	run("tar -jxf output.tar.bz2")
	run("chmod a+x /home/worker/output/script/start.sh")
	run("/home/worker/output/script/start.sh")
	run("ps -ef | grep gateway")
	run("ps -ef | grep simple_worker")
	run("ps -ef | grep small_map_worker")
