#coding:utf8

import time
import msgpack
import uuid

from socket import AF_INET,SOCK_STREAM,socket
from thread import start_new
import struct,json

from EncryptionBox import EncryptionBox



#HOST='192.168.1.115'
HOST='115.29.187.198'
PORT=20000
#HOST='192.168.1.181'
ADDR=(HOST , PORT)
client = socket(AF_INET,SOCK_STREAM)
client.connect(ADDR)
client.setblocking(0)
buf = ""
encrypt = EncryptionBox()
#account = "4e1bf8116b8211e3a82100163e021694"
#passwd = "3a8210016"
account = "hahago"
passwd= "123456"

def recv():
    global buf
    msg_len = 0
    while True:
        try:
            buf += client.recv(1024)
        except :
            #if len(buf) != 0 and len(buf)-2 >= buf_len:
            buf_len = len(buf)
            if buf_len >=2 :
                head = struct.unpack('<h',buf[:2])
                msg_len = head[0]
            if msg_len != 0 and buf_len-2 >= msg_len:
                fmt = '<hh%ss'%(msg_len-2)
                data = struct.unpack(fmt,buf[:msg_len+2])
                buf = buf[msg_len+2:]
                return data
            else:
                continue

def send(request_id, msg):
    packer = msgpack.Packer()
    pmsg = packer.pack(msg)
    data = struct.pack("<hh",len(pmsg)+2, request_id)
    data += encrypt.EDcrpt(pmsg, len(pmsg))
    client.sendall(data)

def unpack(msg_data):
    unpacker = msgpack.Unpacker()
    unpacker.feed(msg_data)
    return unpacker.unpack()


def request(request_id, msg):
    send(request_id, msg)
    data = recv()
    print data
    msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
    print msg
    return msg




def serverchallengerequest():
    data = recv()
    print data
    if data:
        command_id = data[1]
        msg_len = len(data[2]) - 128
        msg_data = data[2][:msg_len]
        table_data = data[2][msg_len:]
        encrypt.SwithTable(table_data)
        msg = unpack(msg_data)
        msg[0] += 1
        #request(1000, msg) # exchange encrypt box
        request(1004, [1, 'xiongxqq']) #register user
        request(1006, ['hahago', '123456']) #bind user
        #request(1012, ['xxiongxx', '100163e02']) #login user
        #request(1012, [account, passwd]) #login user
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        data = recv()
        print data
        msg = unpack(encrypt.EDcrpt(data[2],len(data[2])))
        print msg
        print "send create group"
        request(1072,['123',1,1,1])

def register(username,passwd):
    request(1004, [1, str(uuid.uuid1())]) #register user
    request(1006, [username, '123456']) #bind user


def login(username, passwd):
    request(1012, [username, passwd]) #login user


serverchallengerequest()


