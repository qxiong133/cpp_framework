

import sys
from mdwrkapi import MajorDomoWorker
from group_manager import GroupManager


def main():
    if len(sys.argv) != 2:
        print "argv is less than 2"
        print "python mdworker python_worker.conf"
        return 0
    worker = MajorDomoWorker("tcp://localhost:5555", "python_service", True)
    reply = None
    import os
    worker_root = os.path.dirname(os.path.abspath(__file__))
    print os.path.join(worker_root, sys.argv[1])
    group_instace = GroupManager(config_file_path=os.path.join(worker_root, sys.argv[1]))
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
