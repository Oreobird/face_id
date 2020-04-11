#coding=utf-8
#__author__ = 'zgs'

import socket
import time
import threading
import json
import logging
import connection
from base import *


class Client(threading.Thread):
    def __init__(self, conn=None, ip="", port=1000):
        threading.Thread.__init__(self)
        self.connection = conn
        if self.connection is None:
            if ip != "":
                self.connection = connection.Connection(host=ip, port=port)
            else:
                self.connection = connection.Connection()
    
    def connect(self):
        self.connection.create_conn()
    
    def send_jn_data(self, jn_data):
        send_data = JsonUtil.Dumps(jn_data)[1:-1]
        # print(send_data)
        return self.connection.send_message(send_data)

    def recv_jn_data(self):
        ret, recv_msg = self.connection.read_message()
        if ret:
            recv_msg = JsonUtil.Loads(recv_msg)
        return ret, recv_msg
        
    def pre_run(self):
        pass
    
    def post_run(self):
        pass
    
    def do_task(self):
        pass
    
    def run(self):
        self.connect()
        self.pre_run()
        while True:
            ret = self.do_task()
            if not ret:
                break
        self.post_run()
        self.connection.close()


if __name__ == '__main__':
    pass





