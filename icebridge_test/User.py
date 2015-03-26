#!/bin/env python
#coding:utf8

import time
import msgpack

from socket import AF_INET,SOCK_STREAM,socket
from thread import start_new
import struct,json
import uuid

from EncryptionBox import EncryptionBox
import select
from cpp2python import request_response_mapping
import random


class User:
    def __init__(self, host, port, username, passwd, is_register):
        self.conn = socket(AF_INET,SOCK_STREAM)
        ADDR = (host, port)
        self.conn.connect(ADDR)
        self.conn.setblocking(0)
        self.encrypt = EncryptionBox()
        self.account = username
        self.passwd = passwd
        self.recv_buf = ""
        self.send_buf = ""
        self.encrypt_status = False
        if is_register:
            self.registerOrLogin = self.register_epoll
        else:
            self.registerOrLogin = self.login_epoll

    def fileno(self):
        return self.conn.fileno()

    def recv(self):
        try:
            self.recv_buf += self.conn.recv(1024)
        except:
            return

    def process(self):
        while True:
            buf_len = len(self.recv_buf)
            msg_len = 0
            if buf_len >=2 :
                head = struct.unpack('<h',self.recv_buf[:2])
                msg_len = head[0]
            if msg_len != 0 and buf_len-2 >= msg_len:
                fmt = '<hh%ss'%(msg_len-2)
                data = struct.unpack(fmt,self.recv_buf[:msg_len+2])
                self.recv_buf = self.recv_buf[msg_len+2:]
                self.__process(data)
            else:
                break
        return len(self.send_buf)

    def setEpoll(self, epoll):
        self.epoll = epoll

    def openPollOut(self):
        self.epoll.modify(self.fileno(), select.EPOLLIN + select.EPOLLOUT)

    def __process(self, data):
        if self.encrypt_status == False:
            self.changeEncryptBox(data)
            self.encrypt_status = True
            self.registerOrLogin()
        else:
            self.command_process(data)

    def changeEncryptBox(self, data):
        command_id = data[1]
        msg_len = len(data[2]) - 128
        msg_data = data[2][:msg_len]
        table_data = data[2][msg_len:]
        self.encrypt.SwithTable(table_data)
        msg = self.unpack(msg_data)
        msg[0] += 1
        self.produceData(request_response_mapping.getRequestIdByname('ServerChallengeRequest_ID'), msg)


    def unpack(self, msg_data):
        unpacker = msgpack.Unpacker()
        unpacker.feed(msg_data)
        return unpacker.unpack()

    def command_process(self, data):
        print data
        if len(data) == 3:
            ''' map command id to process '''
            msg = self.unpack(self.encrypt.EDcrpt(data[2],len(data[2])))
            print request_response_mapping.getNameByResponseId(data[1])
            print msg


    def produceData(self, request_id, msg):
        packer = msgpack.Packer()
        pmsg = packer.pack(msg)
        data = struct.pack("<hh",len(pmsg)+2, request_id)
        data += self.encrypt.EDcrpt(pmsg, len(pmsg))
        self.send_buf += data

    def send(self):
        byteswritten = self.conn.send(self.send_buf)
        self.send_buf = self.send_buf[byteswritten:]
        return len(self.send_buf)

    def close(self):
        self.conn.close()

    def register_recv(self):
        msg_len = 0
        while True:
            try:
                self.recv_buf += self.conn.recv(1024)
            except :
                #if len(buf) != 0 and len(buf)-2 >= buf_len:
                buf_len = len(self.recv_buf)
                if buf_len >=2 :
                    head = struct.unpack('<h',self.recv_buf[:2])
                    msg_len = head[0]
                if msg_len != 0 and buf_len-2 >= msg_len:
                    fmt = '<hh%ss'%(msg_len-2)
                    data = struct.unpack(fmt,self.recv_buf[:msg_len+2])
                    self.recv_buf = self.recv_buf[msg_len+2:]
                    return data
                else:
                    continue

    def register_send(self, request_id, msg):
        packer = msgpack.Packer()
        pmsg = packer.pack(msg)
        data = struct.pack("<hh",len(pmsg)+2, request_id)
        data += self.encrypt.EDcrpt(pmsg, len(pmsg))
        self.conn.send(data)

    def register_unpack(self, msg_data):
        unpacker = msgpack.Unpacker()
        unpacker.feed(msg_data)
        return unpacker.unpack()

    def register_request(self, request_id, msg):
        self.register_send(request_id, msg)
        data = self.register_recv()
        print data
        msg = self.register_unpack(self.encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        return msg

    def register_epoll(self):
        request_id = request_response_mapping.getRequestIdByname('RegisterRequest_ID')
        #self.produceData(request_id, [1, str(uuid.uuid1())])
        self.produceData(request_id, [1, self.account])
        request_id = request_response_mapping.getRequestIdByname('BindEmailRequest_ID')
        self.produceData(request_id, [self.account, self.passwd])
        self.login_epoll()
        pass

    def login_epoll(self):
        #request_id = request_response_mapping.getRequestIdByname('LoginRequest_ID')
        #self.produceData(request_id, [self.account, self.passwd])
        request_id = request_response_mapping.getRequestIdByname('RandomNewMapRequest_ID')
        self.produceData(request_id, [1])
        request_id = request_response_mapping.getRequestIdByname('SoldierInfoRequest_ID')
        self.produceData(request_id, [random.randint(1,15),1,10,'camp1',1])
        #request_id = request_response_mapping.getRequestIdByname('SearchGroupRequest_ID')
        #self.produceData(request_id, ['123'])
        '''
        short   soldier_type;^M
        short   soldier_level;^M
        short   soldier_num;^M
        string  which_camp;^M
        short   operation;^M
        '''

    def register(self, race, username, passwd):
        print "register begin"
        data = self.register_recv()
        print "received data"
        if data:
            command_id = data[1]
            msg_len = len(data[2]) - 128
            msg_data = data[2][:msg_len]
            table_data = data[2][msg_len:]
            self.encrypt.SwithTable(table_data)
            msg = self.register_unpack(msg_data)
            msg[0] += 1
            self.register_request(1000, msg) # exchange encrypt box
            self.register_request(1004, [race, str(uuid.uuid1())]) #register user
            self.register_request(1006, [username, passwd]) #bind user

    def login(self, username, passwd):
        print "login begin"
        data = self.register_recv()
        if data:
            command_id = data[1]
            msg_len = len(data[2]) - 128
            msg_data = data[2][:msg_len]
            table_data = data[2][msg_len:]
            self.encrypt.SwithTable(table_data)
            msg = self.register_unpack(msg_data)
            msg[0] += 1
            self.register_request(1000, msg) # exchange encrypt box
            self.register_request(1012, [username, passwd]) #login user










