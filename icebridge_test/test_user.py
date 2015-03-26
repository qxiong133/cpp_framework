#!/bin/env python

from User import  User

HOST='115.29.187.198'
#HOST='192.168.1.181'
#HOST="192.168.1.115"
PORT=20000



def main():
    for line in open("account.txt"):
        username = line.strip()
        print username
        u = User(HOST, PORT, username, "123456");
        #u.register(1, username, "123456")
        u.login(username, "123456")
    pass



if __name__ == "__main__":
    main()
