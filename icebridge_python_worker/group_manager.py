#!/bin/env python
#coding:utf-8

from redishandler import RedisHandler
from dbhandler import DbHandler
from cpp2python import  Transform


class GroupManager():
    def __init__(self, config_file_path):
        '''init mysql and redis client'''
        self.redis = RedisHandler(config_file_path)
        self.db = DbHandler(config_file_path)
        self.transform = Transform('msgpack_id.h','msgpack_struct.h')
        #print t.getResponseIdByName("BuildStatusListRequest_ID")
        #print t.getNameByRequestId(1000)
        #self.redis.test()
        #self.db.test()

    def process(self, request):
        '''
        SearchGroupRequest_ID,//63
    JoinGroupRequest_ID,//64
    GetJoinRequest_ID,//65
    AcceptJoinRequest_ID,//66
    ExitGroupRequest_ID,//67
    CreateGroupRequest_ID,//68
    PublicGroupMessageRequest_ID,//69
    GetGroupInfoRequest_ID//70
        '''
        print request
        return request

    def searchGroup(self, group_name):
        '''return gid group_icon group_name group_type 3/50 min_integral total_integral'''
        group_infos = self.db.searchGroup(group_name)
        if len(group_infos) == 0:
            return [0 , "" , 0, 0, 0, 0, 0 ,0]
        info = group_infos[0]
        member_num = self.redis.getMembersNum(info['id'])
        return [info['id'], info['group_name'], info['group_icon'], info['group_type'], member_num, info['member_num'], info['min_integral'], info['group_cup_num']]

    def joinGroup(self, uid, group_id):
        '''
           return true false
        '''

        if self.redis.userInGroup(uid):
            return [False, 0, "the user has belong to a group"]
        group_type = self.db.getGroupType(group_id)
        if not group_type:
            return [False, 0, "group is not exited"]
        if group_type == 1:
            self._joinDirectly(uid, group_id)
            return [True, 0, ""]
        if group_type == 2:
            return [False, 1, "the group is not allowed to join"]
        if group_type == 3:
            self._sendJoinRequest(uid, group_id)
            return [True, 0, "apply for joining success"]

    def _joinDirectly(self, uid, group_id):
        return self.redis.addUserIntoGroup(uid, group_id, 0)

    def _sendJoinRequest(self, uid, group_id):
        return self.redis.sendJoinRequest(uid, group_id)

    def getJoinRequest(self, uid):
        '''
        message jiangling  juntuanzhang  request_join_uid name and gid
        '''
        ret = []
        group_id = self.redis.getGroupId(uid)
        if not group_id:
            return ret
        for req_uid in self.redis.getJoinRequest(group_id):
            name = self.redis.getUserNameById(uid)
            ret.append([req_uid, name, group_id])
        return [ret]

    def acceptJoin(self, uid, join_uid, group_id):
        '''return true false'''
        self._joinDirectly(join_uid, group_id)
        return [True, 0, ""]

    def exitGroup(self, uid, group_id):
        '''  return true false '''
        self.redis.deleteUserFromGroup(uid, group_id)
        return [True, 0, ""]

    def createGroup(self, uid, group_info):
        ''' init group level 1 and max group member 50

            recv group_icon group_name group_type min_integral

            return true/false
                   err_number 1. database fail 2. name duplicate
        '''
        if self.redis.userInGroup(uid):
            return [False, 0, "the user has belong to a group"]
        ret = self.db.searchGroup(group_info[0])
        if len(ret) != 0:
            return [False, 1, "the group has existed"]
        group_id = self.db.insertGroup(group_info)
        if group_id:
            if self._addUserIntoGroup(uid, group_id, 3):
                return [True, 0, ""]
            else:
                self.db.deleteGroup(group_id)
                return [False, 3, "redis operation failed"]
        else:
            return [False, 2, "db operation failed"]

    def _addUserIntoGroup(self, uid, group_id, group_position):
        ''' groupId group#position 1普通成员 2将领 3军团长 '''
        return self.redis.addUserIntoGroup(uid, group_id, group_position)

    def _deleteUserFromGroup(self, uid, group_id):
        ''' groupId group#position 1普通成员 2将领 3军团长 '''
        return self.redis.deleteUserFromGroup(uid, group_id)

    def publicGroupMessage(self, uid, group_id, message):
        ''' recv string
            return true false
                   err_number 1. database fail
        '''
        if self.redis.updateGroupMessage(uid, group_id, message):
            return [True, 0, ""]
        else:
            return [False, 0, "update message failed"]

    def deleteUserFromGroup(self, uid, delete_uid):
        ''' #send message by mail to delete_uid
            return true false
                    err_number 1. net err
        '''
        group_id = self.redis.getGroupId(uid)
        self.redis.deleteUserFromGroup(delete_uid, group_id)
        return [True, 0, ""]

    def getGroupInfo(self, uid, group_id):
        ''' gid group_icon group_name group_type 3/50 total_integral min_integral
            user_list
            [rece,uid,name,integral,position]
        '''
        group_infos = self.db.getGroupById(group_id)
        if len(group_infos) == 0:
            return [0 , "" , 0, 0, 0, 0, 0 ,0, "", []]
        info = group_infos
        member_num = self.redis.getMembersNum(info['id'])
        group_message = self.redis.getGroupMessage(group_id)
        users_list = self.redis.getGroupUsersInfo(group_id)
        return [info['id'], info['group_name'], info['group_icon'], info['group_type'], member_num, info['member_num'], info['min_integral'], info['group_cup_num'], group_message, users_list]


    def getTop20Group(self, uid):
        ''' list top 20 group if the group_id in the top 20
            if the gourp_id not in the top 20, list the top 19 and add the group_id

            user get on line send the messge
            {[gid group_icon group_name group_type 3/50 total_integral]

            }
	int  gid;
	string  group_name;
	int  group_icon;
	int   group_type;
	int  recent_member_num;
	int  max_num;
	int  min_integral;
	int  total_integral;
        return [info['id'], info['group_name'], info['group_icon'], info['group_type'], member_num, info['member_num'], info['min_integral'], info['group_cup_num'], group_message, user_list]
        '''
        group_infos = self.db.getTop20Group()
        ret = []
        for info in group_infos:
            member_num = self.redis.getMembersNum(info['id'])
            group = [info['id'], info['group_name'], info['group_icon'], info['group_type'], member_num, info['member_num'], info['min_integral'], info['group_cup_num']]
            ret.append(group)
        return [ret]

    def getTop50User(self, uid):
        ''' list the top 50 User if the user in the top50
            if the user not in the top50, list the top 49 and add himself


            online send the message
            [race,uid,name,integral,gid, group_name, group_icon, battle_success, defence_success]
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

        return self.redis.hmget('user#%s'%uid, ['race', 'nickname', 'abt#point', 'group#position','groupId','battle#success', 'defence#success'])

        '''
        users_infos = []
        for uid in self.redis.getTopUsersInfo():
            u_infos = self.redis.getUserGroupInfo(uid)
            group_id = u_infos[4]
            g_infos = self.db.getGroupById(group_id)
            users_infos.append([u_infos[0],uid,u_infos[1],u_infos[2],u_infos[3],u_infos[4],g_infos['group_name'], g_infos['group_icon'], u_infos[5], u_infos[6]])
        return [users_infos]

    def caculateGroupIntegral(self, group_id):
        group_integral = 0;
        pos = 1
        ratio = {0:0.5,1:0.25,2:0.12,3:0.1,4:0.03}
        for user_info in self.redis.getGroupUsersInfo(group_id):
            integral = int(user_info[3])
            factor = 0
            tmp = pos
            while True:
                tmp -= 10
                if tmp <= 0:
                    break
                factor += 1
            if factor > 4:
                continue
            group_integral += integral * ratio[factor]
            pos += 1
        group_integral = int(group_integral)
        return group_integral




def main():
    import os
    worker_root = os.path.dirname(os.path.abspath(__file__))
    print os.path.join(worker_root,"python_worker.conf")
    g = GroupManager(config_file_path=os.path.join(worker_root,"python_worker.conf"))
    ''' user 3061'''
    '''group_name group_icon group_type 1.任何人可以加入 2. 不可以加入 3需批准加入 min_integral'''
    #print g.createGroup(3061, [u"123",1,1,0])
    #print g.searchGroup("123")
    #print g.joinGroup(402, 6)
    #print g.joinGroup(3816, 6)
    #print g.getJoinRequest(3061)
    #print g.getGroupInfo(3061, 6)
    #print g.getTop20Group(3061)
    #print g.getTop50User(3061)
    #print g.caculateGroupIntegral(6)
    pass



if __name__ == "__main__":
    main()
