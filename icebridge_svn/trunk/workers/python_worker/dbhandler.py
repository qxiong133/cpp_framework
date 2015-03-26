#!/bin/env python
#coding:utf-8

import os
import ConfigParser

import database
import logging

class DbHandler():
    def __init__(self,db_config,db_section="master_mysql"):
        self.init(db_config,db_section)
        logging.basicConfig(format="%(asctime)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S", level=logging.INFO)
        pass


    def init(self,db_config,db_section="master_mysql"):
        (host,port,user,password,db) = self.readDbConfig(db_config,db_section)
        self.db = database.Connection("%s:%s"%(host,port),db,user,password)

    def readDbConfig(self,db_config,db_section):
        cf = ConfigParser.ConfigParser()
        if not os.path.exists(db_config):
            print '''>>sys.stderr,'''"%s not exist!"%db_config
            os._exit(-1)
        cf.read(db_config)
        return cf.get(db_section,"host"), \
            cf.get(db_section,"port"), \
            cf.get(db_section,"user"), \
            cf.get(db_section,"password"), \
            cf.get(db_section,"db")

    def getDb(self):
        if self.db:
            return self.db
        return None

    def test(self):
        for group in self.db.iter("select * from army_group limit 10"):
            print group

    def searchGroup(self, group_name):
        group_info = []
        for group in self.db.iter("select * from army_group where group_name=%s", group_name):
            group_info.append(group)
        return group_info

    def insertGroup(self, group_info):
        '''group_name group_icon group_type 1.任何人可以加入 2. 不可以加入 3需批准加入 min_integral'''
        if len(group_info) != 4:
            return False
        lastrowid = self.db.execute_lastrowid("insert into army_group(group_name, group_icon, group_type, min_integral, group_level, member_num, group_cup_num) values(%s, %s, %s, %s, %s, %s, %s)",group_info[0], group_info[1], group_info[2],group_info[3],1,50,0)
        return lastrowid

    def deleteGroup(self, group_id):
        logging.info("delete group %s", group_id)
        return self.db.execute("delete from army_group where id = %s", group_id)

    def updateGroup(self, group_icon, group_type, min_integral, group_id):
        logging.info("update group %s", group_id)
        return self.db.execute("update army_group set group_icon=%s,group_type=%s,min_integral=%s where id=%s", group_icon, group_type, min_integral, group_id)

    def getGroupType(self, group_id):
        ret = self.db.get("select group_type from army_group where id = %s", group_id)
        if ret:
            return ret['group_type']
        return None

    def getGroupById(self, group_id):
        return self.db.get("select * from army_group where id = %s", group_id)

    def getTop20Group(self):
        '''{'group_cup_num': 0L, 'group_level': 1, 'min_integral': 0, 'member_num': 50L, 'id': 6L, 'group_name': u'123', 'group_icon': 1, 'group_type': 1, 'ctime': datetime.datetime(2014, 1, 9, 11, 45, 42)}
       '''
        return self.db.iter("select * from army_group order by group_cup_num desc limit 20")

