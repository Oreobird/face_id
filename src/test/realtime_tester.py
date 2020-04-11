#coding=utf-8
#__author__ = 'zgs'

import os
import socket
import time
import cv2
import base64

import threading
import struct
import sys
from common.base import *

def init_path():
    cur_path = os.path.abspath('.')
    comm_path = cur_path + "/common"
    sys.path.append(comm_path)
    sys.path.append(".")
    sys.path.append("..")

init_path()

import common.client as client


class RealTimeTester(client.Client):
	def __init__(self, ip="", port=1000, resolution=(160, 120), test_user="tester"):
		client.Client.__init__(self, conn=None, ip=ip, port=port)
		self.img_param = []
		self.camera = None
		self.resolution = resolution
		self.test_user = test_user
		self.correct = 0
		self.total = 0
		
	def add_overlays(self, frame, name, box):
		width, height, _ = frame.shape
		x1 = int(height * box[0])
		y1 = int(width * box[1])
		x2 = int(height * box[2])
		y2 = int(width * box[3])
		cv2.rectangle(frame, (x1, y1), (x2, y2),
					  (0, 255, 0), 2)
		cv2.putText(frame, name, (x1, y1),
					cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255),
					thickness=2, lineType=2)
		
	def do_task(self):
		ret, img = self.camera.read()
		
		if ret:
			cpy_img = img.copy()
			
			# cpy_img = cv2.cvtColor(cpy_img, cv2.COLOR_BGR2RGB)
			
			cpy_img = cv2.resize(cpy_img, self.resolution)
			_, img_encode = cv2.imencode('.jpg', cpy_img, self.img_param)
			img_str = base64.b64encode(img_encode).decode('utf-8')

			json_data = [{'id': 'xxx', 'name': 'xxx', 'type': 0,
						  'detect_in_client': False, 'mode': 0,
						  'img_data': img_str
						  }]
			self.send_jn_data(json_data)
			
			ret, recv_data = self.recv_jn_data()
			if ret:
				if recv_data['err_code'] == 0:
					self.total += 1
					user_name = recv_data['user_name']
					box = recv_data['box']
					print("name: {}, box: {}".format(user_name, box))
					if user_name == self.test_user:
						self.correct += 1

					self.add_overlays(img, user_name, box)
			
			cv2.imshow("video", img)
			if cv2.waitKey(1) == ord('q') or self.total == 10000:
				return False
		return True
		
	def pre_run(self):
		self.camera = cv2.VideoCapture(0)
		self.img_param = [int(cv2.IMWRITE_JPEG_QUALITY), 100]  # 设置传送图像格式、帧数
		time.sleep(0.5)
		
	def post_run(self):
		accuracy = self.correct / self.total
		print("\ntotal:{}, correct:{}, accuracy:{}".format(self.total, self.correct, accuracy))
		
if __name__ == '__main__':
	tester = RealTimeTester(ip = '127.0.0.1', port = 8080, test_user='AIPlayer')
	tester.setName('realtime_tester')
	tester.setDaemon(True)
	tester.start()
	
	tester.join()