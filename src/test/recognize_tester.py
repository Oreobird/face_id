#coding=utf-8
#__author__ = 'zgs'

import os
import socket
import time
import cv2
import base64
from tqdm import tqdm
import argparse
import threading
import struct
import sys
from common.base import *
from dataset_config import *

def init_path():
    cur_path = os.path.abspath('.')
    comm_path = cur_path + "/common"
    sys.path.append(comm_path)
    sys.path.append(".")
    sys.path.append("..")

init_path()

import common.client as client


class AccRecognizeTester(client.Client):
	def __init__(self, ip="", port=1000, resolution=(160, 120), img_dir='./', registed_file="", detect_in_client=False):
		client.Client.__init__(self, conn=None, ip=ip, port=port)
		self.img_param = []
		self.resolution = resolution
		self.correct = 0
		self.total = 0
		self.img_dir = img_dir
		self.registed_file = registed_file
		self.sample_dict = {}
		self.detect_in_client = detect_in_client
		self.start_time = 0
		self.end_time = 0
		
	def pre_run(self):
		self.img_param = [int(cv2.IMWRITE_JPEG_QUALITY), 100]  # 设置传送图像格式、帧数
		
		registed_dict = {}
		with open(self.registed_file, 'r') as fr:
			lines = fr.readlines()
			for line in lines:
				file_name, face_id = line.replace('\n',"").split(',')
				registed_dict[face_id] = file_name
				
		for root, dirs, files in os.walk(self.img_dir):
			for subfolder in dirs:
				if registed_dict.get(subfolder) is not None:  # registed id
					subfiles = os.listdir(os.path.join(self.img_dir, subfolder))
					if len(subfiles) > 1:
						self.sample_dict[subfolder] = os.path.join(self.img_dir + subfolder, subfiles[-1])
		
		print("id num:{}".format(len(self.sample_dict)))
		
		time.sleep(0.5)
		self.start_time = time.time()
		
	def do_task(self):
		for key, val in self.sample_dict.items():
			face_id = key
			file_path = val
				
			# print("face_id: {}, file_path: {}".format(face_id, file_path))
			img = cv2.imread(file_path)
			# img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
			
			_, img_encode = cv2.imencode('.jpg', img, self.img_param)
			img_str = base64.b64encode(img_encode).decode('utf-8')

			json_data = [{'id': os.path.basename(file_path).split('.')[0], 'name': str(face_id), 'type': 0,
						  'detect_in_client': True, 'mode': 1,
						  'img_data': img_str
						  }]
			self.send_jn_data(json_data)
			
			ret, recv_data = self.recv_jn_data()
			if ret:
				if recv_data['err_code'] == 0:
					self.total += 1
					user_name = recv_data['user_name']
					result = "[Fail]"
					if user_name == face_id:
						self.correct += 1
						result = '[Pass]'
					print("Test file:{}".format(file_path))
					print("{}--> Label_Name:{}, Test_Name:{}".format(result, face_id, user_name))
					
			if self.total % 100 == 0:
				print("\n======> total:{}, correct:{}, accuracy:{}".format(self.total, self.correct, self.correct / self.total))
				end_time = time.time()
				
				elapsed = end_time - self.start_time
				avg_time = elapsed / self.total
				print("======> total time: {}, sample_num:{}, avg time: {}\n".format(elapsed, self.total, avg_time))
		return False

		
	def post_run(self):
		end_time = time.time()
		
		accuracy = self.correct / self.total
		print("\ntotal:{}, correct:{}, accuracy:{}".format(self.total, self.correct, accuracy))
		
		elapsed = end_time - self.start_time
		avg_time = elapsed / self.total
		print("======> total time: {}, sample_num:{}, avg time: {}\n".format(elapsed, self.total, avg_time))
		
		
class FarRecognizeTester(client.Client):
	def __init__(self, ip="", port=1000, resolution=(160, 160), img_dir='./', detect_in_client=False):
		client.Client.__init__(self, conn=None, ip=ip, port=port)
		self.img_param = []
		self.resolution = resolution
		self.total = 0
		self.img_dir = img_dir
		self.sample_dict = {}
		self.unknown = 0
		self.start_time = 0
		self.end_time = 0
		self.detect_in_client = detect_in_client
		
	def pre_run(self):
		files = os.listdir(self.img_dir)
		for f in files:
			self.sample_dict[f.split('.')[0]] = os.path.join(self.img_dir, f)
		self.start_time = time.time()
			
	def do_task(self):
		for key, val in self.sample_dict.items():
			face_id = key
			file_path = val
			
			# print("face_id: {}, file_path: {}".format(face_id, file_path))
			
			img = cv2.imread(file_path)
			
			if img is None:
				continue
				
			img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
			img = cv2.resize(img, self.resolution)
			
			_, img_encode = cv2.imencode('.jpg', img, self.img_param)
			img_str = base64.b64encode(img_encode).decode('utf-8')
			
			json_data = [{'id': os.path.basename(file_path).split('.')[0], 'name': str(face_id), 'type': 0,
						  'detect_in_client': self.detect_in_client, 'mode': 1,
						  'img_data': img_str
						  }]
			self.send_jn_data(json_data)
			
			ret, recv_data = self.recv_jn_data()
			if ret:
				if recv_data['err_code'] == 0:
					self.total += 1
					user_name = recv_data['user_name']
					
					result = "Fail"
					if user_name == "unknown":
						self.unknown += 1
						result = "Pass"

					print("Test file:{}".format(file_path))
					print("{}--> True_Name:{}, Test_Name:{}".format(result, face_id, user_name))
					
			if self.total % 100 == 0:
				print("\n======> total:{}, unknown:{}, far:{}".format(self.total, self.unknown, (1.0 - self.unknown / self.total)))
				end_time = time.time()
				
				elapsed = end_time - self.start_time
				avg_time = elapsed / self.total
				print("======> total time: {}, sample_num:{}, avg time: {}\n".format(elapsed, self.total, avg_time))
		return False
		
	def post_run(self):
		self.end_time = time.time()
		elapsed_time = self.end_time - self.start_time
		avg_time = elapsed_time / len(self.sample_dict)
		far = 1 - self.unknown / self.total
		print("\ntotal:{}, unknown:{}, far:{}".format(self.total, self.unknown, far))
		print("\ntotal time: {}, sample_num:{}, avg time: {}".format(elapsed_time, self.total, avg_time))

def parse_args():
	parser = argparse.ArgumentParser()
	parser.register("type", "bool", lambda v: v.lower() == "true")
	parser.add_argument(
		"--acc",
		type=str,
		default="",
		help="Evaluate Accuracy"
	)
	parser.add_argument(
		"--far",
		type=str,
		default="",
		help="Evaluate FAR"
	)
	parser.add_argument(
		"--dataset",
		type=str,
		default="lfw",
		help="dataset: lfw_crop, celeba_crop"
	)
	parser.add_argument(
		"--registed_file",
		type=str,
		default="/data/LFW/registed_id.txt",
		help="Registed file"
	)
	return parser.parse_known_args()
		
if __name__ == '__main__':
	FLAGS, unparsed = parse_args()
	
	img_dir, registed_file, detect_in_client = parse_dataset(FLAGS.dataset)
	
	if len(FLAGS.acc) > 0:
		tester = AccRecognizeTester(ip = '127.0.0.1', port = 8080,
								 img_dir=img_dir,
								 registed_file=registed_file,
								 detect_in_client=detect_in_client
								 )
	elif len(FLAGS.far) > 0:
		tester = FarRecognizeTester(ip = '127.0.0.1', port = 8080,
									img_dir=img_dir, detect_in_client=detect_in_client)
	else:
		print("No test mode selected.")
		exit(-1)
	
	tester.setName('recognize_tester')
	tester.setDaemon(True)
	tester.start()
	
	tester.join()