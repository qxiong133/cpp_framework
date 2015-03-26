#!/bin/env python
#coding:utf-8

import heapq

class TimeHeap():
    '''push (int timevalue, user send a random message)  '''
    def __init__(self):
        self.n = 0
        self.list = []

    def buildHeap(self):
        self.list = [random.randint(1, 100) for i in range(30)]
        heapq.heapify(self.list) #建立最小堆
        for i in range(len(self.list)):
            print heapq.heappop(self.list) , #在堆中取出一个元素
        print '\n'

    def pop(self):
        if self.n > 0:
            self.n -= 1
            return heapq.heappop(self.list) , #在堆中取出一个元素
        return None

    def push(self, item):
        heapq.heappush(self.list, item)
        self.n += 1



def main():
    t = TimeHeap()
    t.push((4,"a"))
    t.push((2,"c"))
    t.push((1,"c"))
    while True:
        ret = t.pop()
        if ret == None:
            break
        print ret



if __name__ == "__main__":
    main()
