"""Majordomo Protocol worker example.

Uses the mdwrk API to hide all MDP aspects

Author: Min RK <benjaminrk@gmail.com>
"""

import sys
from mdwrkapi import MajorDomoWorker
from group_manager import GroupManager


def main():
    verbose = '-v' in sys.argv
    worker = MajorDomoWorker("tcp://localhost:5555", "python_service", True)
    reply = None
    import os
    worker_root = os.path.dirname(os.path.abspath(__file__))
    print os.path.join(worker_root,"python_worker.conf")
    group_instace = GroupManager(config_file_path=os.path.join(worker_root,"python_worker.conf"))
    while True:
        request = worker.recv(reply)
        if request is None:
            break # Worker was interrupted
        ''' reply is a list, the messge that we want to return is [msg]
            the request we receive is [msg]
        '''
        reply = group_instace.process(request)


if __name__ == '__main__':
    main()
