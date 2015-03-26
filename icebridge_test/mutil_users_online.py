
import socket, select

class MutiUserOnline():
    def __init__(self):
        self.epoll = select.epoll()
        #epoll.register(onesocket.fileno(), select.EPOLLIN)
        self.connections = {}

    def addUser(self, user):
        ''' init user  '''
        self.connections[user.fileno()] = user
        user.setEpoll(self.epoll)
        self.epoll.register(user.fileno(), select.EPOLLIN)

    def run(self):
        try:
            while True:
                events = self.epoll.poll(1)
                for fileno, event in events:
                    if event & select.EPOLLIN:
                        conn = self.connections[fileno]
                        conn.recv()
                        if conn.process():
                            #self.epoll.modify(conn.fileno(), select.EPOLLIN + select.EPOLLOUT)
                            conn.openPollOut()
                    elif event & select.EPOLLOUT:
                        conn = self.connections[fileno]
                        if conn.send() == 0:
                            self.epoll.modify(fileno, select.EPOLLIN)
                            #connections[fileno].shutdown(socket.SHUT_RDWR)
                    elif event & select.EPOLLHUP:
                        self.epoll.unregister(fileno)
                        connections[fileno].close()
                        del connections[fileno]
        finally:
            #epoll.unregister(serversocket.fileno())
            self.epoll.close()
            for fileno in self.connections:
                self.connections[fileno].close()
                #del self.connections[fileno]
