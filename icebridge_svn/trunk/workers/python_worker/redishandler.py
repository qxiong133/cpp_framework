#!/bin/env python

import os
import ConfigParser

import redis


class RedisHandler():
    def __init__(self,config_file_path,section="master_redis"):
        self.init(config_file_path, section)
        pass


    def init(self,config_file_path,section):
        (host,port,db) = self.readRedisConfig(config_file_path,section)
        self.redis = redis.StrictRedis(host, int(port), int(db))

    def readRedisConfig(self,config_file_path,section):
        cf = ConfigParser.ConfigParser()
        if not os.path.exists(config_file_path):
            print '''>>sys.stderr,'''"%s not exist!"%config_file_path
            os._exit(-1)
        cf.read(config_file_path)
        return cf.get(section,"host"), \
            cf.get(section,"port"), \
            cf.get(section,"db")

    def getRedisHandler(self):
        if self.redis:
            return self.redis
        return None

    def test(self):
        print self.redis.zrange('test_sorted_set', 0, -1, withscores=True)

    def userInGroup(self, uid):
        ret = self.redis.hmget('user#%s'%uid, ['groupId', 'group#position'])
        if len(ret) > 1 and (ret[0] == None or ret[0] == '0'):
            return False
        return True

    def addUserIntoGroup(self, uid, group_id, group_position):
        '''update user#uid
           add user into group#gid
            if group_position is 2 add user to group#gid#leader
        '''
        ret = self._updateUserGroup(uid, group_id, group_position)
        ret = self._addUserIntoGroupMembers(uid, group_id)
        if group_position == 2:
            ret = self._addUserIntoGroupLeaders(uid, group_id)
        return True

    def deleteUserFromGroup(self, uid, group_id):
        '''update user#uid
        delete user from group#gid
        if group_position is 2 delete user from group#gid#leader
        '''
        position = self.getGroupPosition(uid)
        self._updateUserGroup(uid, 0, 0)
        self._deleteUserFromGroupMembers(uid, group_id)
        self._deleteUserFromGroupLeaders(uid, group_id)
        if position == '3':
            self._chooseGroupOwner(group_id)
        return self.getMembersNum(group_id)

    def _chooseGroupOwner(self, group_id):
        leader_list = self._getUserListFromGroupLeaders(group_id)
        if leader_list:
            self._updateUserGroup(leader_list[0], group_id, 3)
            print "group %s 's owner is %s"%(group_id, leader_list[0])
            return
        uid_list = self.getGroupMembersSorted(group_id)
        if uid_list:
            self._updateUserGroup(uid_list[0], group_id, 3)
            print "group %s 's owner is %s"%(group_id, uid_list[0])

    def _addUserIntoGroupLeaders(self, uid, group_id):
        return self.redis.sadd("group#%s#leaders"%group_id, uid)

    def _deleteUserFromGroupLeaders(self, uid, group_id):
        return self.redis.srem("group#%s#leaders"%group_id, uid)

    def _getUserListFromGroupLeaders(self, group_id):
        return self.redis.smembers("group#%s#leaders"%group_id)

    def _updateUserGroup(self, uid, group_id, group_position):
        return self.redis.hmset("user#%s"%uid,{"groupId":group_id, "group#position":group_position})

    def _addUserIntoGroupMembers(self, uid, group_id):
        point = self.redis.hget("user#%s"%uid, "abt#point")
        return self.redis.zadd("group#%s"%group_id, point, uid)

    def _deleteUserFromGroupMembers(self, uid, group_id):
        return self.redis.zrem("group#%s"%group_id, uid)

    def getMembersNum(self, group_id):
        ret = self.redis.zcard("group#%s"%group_id)
        if ret:
            return int(ret)
        return 0

    def sendJoinRequest(self, uid, group_id):
        return self.redis.sadd("group#%s#request"%group_id, uid)

    def getJoinRequest(self, group_id):
        return self.redis.smembers("group#%s#request"%group_id)

    def getGroupId(self, uid):
        return self.redis.hget("user#%s"%uid, "groupId")

    def getUserNameById(self, uid):
        return self.redis.hget("user#%s"%uid, "nickname")

    def getGroupPosition(self, uid):
        return self.redis.hget("user#%s"%uid, "group#position")

    def updateGroupMessage(self, uid, group_id, message):
        position = self.getGroupPosition(uid)
        if position != '3':
            return False
        return self.redis.set("group#%s#message"%group_id, message)

    def getGroupMessage(self, group_id):
        return self.redis.get("group#%s#message"%group_id)

    def getGroupUsersInfo(self, group_id):
        users_info = []
        for uid in self.getGroupMembersSorted(group_id):
            ret = self.redis.hmget('user#%s'%uid, ['race', 'nickname', 'abt#point', 'group#position'])
            users_info.append([int(ret[0]), int(uid), ret[1], int(ret[2]), int(ret[3])])
        return users_info

    def getGroupMembersSorted(self, group_id):
        return self.redis.zrevrange("group#%s"%group_id, 0 , -1)

    def getTopMembersSorted(self):
        return self.redis.zrevrange("group#ranking", 0 , 50)

    def getTopUsersInfo(self):
        ''' [race,uid,name,integral,gid, group_name, group_icon, battle_success, defence_success] '''
        return self.getTopMembersSorted()

    def getUserGroupInfo(self, uid):
        '''
	int  race;
	int  uid;
	string  name;
	int  integral;
	int  position;
	int  gid;
	string  group_name;
	int  group_icon;
	int  battle_success;
	int  defence_success;
        '''
        ret = []
        count = 0
        for value in self.redis.hmget('user#%s'%uid, ['race', 'nickname', 'abt#point', 'group#position','groupId','battle#success', 'defence#success']):
            count += 1
            if count == 2:
                ret.append(value)
                continue
            if value:
                ret.append(int(value))
                continue
            ret.append(0)
        return ret

    def getSimilarUser(self, uid):
        point = self.redis.hget("user#%s"%uid, "abt#point")
        if point:
            point = int(point)
        min_point = point - 30
        max_point = point + 30
        return self.redis.zrangebyscore("group#ranking", min_point, max_point)

