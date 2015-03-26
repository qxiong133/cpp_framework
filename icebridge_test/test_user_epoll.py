#!/bin/env python

from User import  User
from mutil_users_online import MutiUserOnline
import sys

HOST='115.29.187.198'
#HOST='192.168.1.181'
#HOST="192.168.1.115"
PORT=20000



def main():
    m = MutiUserOnline()
    for line in open(sys.argv[1]):
        print line
        username = line.strip()
        #u = User(HOST, PORT, username, "123456", False);
        if username:
            u = User(HOST, PORT, username, "123456", True);
            m.addUser(u)
    m.run()
    pass



if __name__ == "__main__":
    main()
