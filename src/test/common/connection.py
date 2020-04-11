#coding=utf-8
#__author__ = 'zgs'

import socket
import logging
import struct

import com_config

class Connection():
    def __init__(self, host='127.0.0.1', port=1000):
        if host == '127.0.0.1' and port == 1000:
            self.host = com_config.access_host
            self.port = com_config.access_port
        else:
            self.host = host
            self.port = port
        
        self.tcp_client = None
    
    def create_conn(self):
        print("Connection create_conn")
        if self.tcp_client is not None:
            return
        
        try:
            self.tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.tcp_client.setblocking(True)
            # self.tcp_client.settimeout(2)
            self.tcp_client.connect((self.host, self.port))
            return True
        
        except Exception as ex:
            
            return False
    
    def close(self):
        if self.tcp_client is not None:
            self.tcp_client.close()
            self.tcp_client = None
    
    def send_message(self, content):
        try:
            if self.tcp_client is None:
                if not self.create_conn():
                    return False
            
            # print(len(content))
            ret = self.tcp_client.sendall(struct.pack('I', socket.htonl(len(content))) + content.encode())
            if ret != None:
                raise Exception("send message error")
            
            return True
        except Exception as ex:
            logging.error("exception ex, ", ex)
            return False
    
    def read_n_message(self, read_len):
        data = b""
        while read_len:
            try:
                recv_buf = self.tcp_client.recv(read_len)
                read_len -= len(recv_buf)
                data += recv_buf

            except Exception as ex:
                return False, b""
        return True, data
    
    def decode_len(self, content):
        return socket.ntohl(struct.unpack('I', content)[0])
    
    def read_message(self):
        # read content len: integer type
        ret, data = self.read_n_message(4)
        if not ret:
            return False, ""
        content_len = self.decode_len(data)
        # read content
        ret, content = self.read_n_message(content_len)
        return ret, content.decode()[:-1]


if __name__ == '__main__':
    pass