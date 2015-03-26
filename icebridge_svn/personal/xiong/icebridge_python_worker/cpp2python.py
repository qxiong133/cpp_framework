#!/bin/env python

import re


class Transform():
    def __init__(self, id_path , struct_path):
        self.id_path = id_path
        self.struct_path = struct_path
        self.request_id2name_struct = {}
        self.response_id2name_struct = {}
        self.request_name2id= {}
        self.response_name2id= {}
        self.name2struct= {}
        self.re_start = re.compile(r'\s*([^\s]*?)\s*=.*?(\d+).*')
        self.re_normal = re.compile(r'\s*(\S*)\s*,//(\d+).*')

    def process_id(self):
        in_brace = False
        brace_num = 0
        start_num = 0
        for line in open(self.id_path):
            if '{' in line:
                in_brace = True
                continue
            if '}' in line:
                in_brace = False
                brace_num += 1
                continue
            if brace_num == 0 and in_brace:
                m = self.re_start.match(line)
                if m:
                    g = m.groups()
                    self.request_name2id[g[0]] = int(g[1])
                    self.request_id2name_struct[int(g[1])] = [g[0]]
                    start_num = int(g[1])
                m = self.re_normal.match(line)
                if m:
                    g = m.groups()
                    self.request_name2id[g[0]] = int(g[1]) + start_num - 1
                    self.request_id2name_struct[int(g[1]) + start_num - 1] = [g[0]]
            if brace_num == 1 and in_brace:
                m = self.re_start.match(line)
                if m:
                    g = m.groups()
                    self.response_name2id[g[0]] = int(g[1])
                    self.response_id2name_struct[int(g[1])] = [g[0]]
                    start_num = int(g[1])
                m = self.re_normal.match(line)
                if m:
                    g = m.groups()
                    self.response_name2id[g[0]] = int(g[1]) + start_num - 1
                    self.response_id2name_struct[int(g[1]) + start_num - 1] = [g[0]]

    def getNameByRequestId(self, msg_id):
        if msg_id in self.request_id2name_struct:
            return self.request_id2name_struct[msg_id][0]
        return None

    def getResponseIdByName(self, msg_name):
        if msg_name in self.response_name2id:
            return self.response_name2id[msg_name]
        return None

    def process_struct(self):
        pass



def main():
    t = Transform('msgpack_id.h','msgpack_struct.h')
    t.process_id()
    print t.getResponseIdByName("BuildStatusListRequest_ID")
    print t.getNameByRequestId(1000)
    pass





if __name__ == "__main__":
    main()

