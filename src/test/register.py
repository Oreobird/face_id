#coding=utf-8
#__author__ = 'zgs'

import os
from tqdm import tqdm
import socket
import time
import cv2
import base64
import argparse
import threading
import logging
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


class Register(client.Client):
	def __init__(self, ip="", port=1000, img_dir="", regist_file="",
				 sub_dir_flag=True, ext='.jpg', detect_in_client=False, append_regist=False):
		client.Client.__init__(self, conn=None, ip=ip, port=port)
		self.img_param = []
		self.img_dir = img_dir
		self.id_dict = {}
		self.regist_file = regist_file
		self.subdir_flg = sub_dir_flag
		self.ext = ext
		self.detect_in_client = detect_in_client
		self.append_regist = append_regist
		
	def pre_run(self):
		self.img_param = [int(cv2.IMWRITE_JPEG_QUALITY), 100]  # 设置传送图像格式、帧数
		print(self.img_dir)
		if self.subdir_flg:
			for root, dirs, files in os.walk(self.img_dir):
				for subfolder in dirs:
					# print(subfolder)
					if self.id_dict.get(subfolder) is None:
						subfiles = os.listdir(os.path.join(self.img_dir, subfolder))
						for f in subfiles:
							self.id_dict[subfolder] = f
							break
		else:
			files = os.listdir(self.img_dir)
			for f in files:
				if self.id_dict.get(f) is None:
					self.id_dict[f] = f
		
		print("id num:%d" % len(self.id_dict))
		time.sleep(0.5)
		
	def do_task(self):
		for face_id, file_name in tqdm(self.id_dict.items()):
			if self.subdir_flg:
				file_path = os.path.join(self.img_dir + face_id, file_name.split('.')[0] + self.ext)
			else:
				file_path = self.img_dir + file_name
			print(file_path)
			img = cv2.imread(file_path)
			
			if img is None:
				continue
				
			# img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
			print(img.shape[0])
			_, img_encode = cv2.imencode('.jpg', img, self.img_param)
			img_str = base64.b64encode(img_encode).decode('utf-8')

			json_data = [{'id': face_id, 'name': face_id, 'type': 1,
						  'detect_in_client': self.detect_in_client, 'mode': 1,
						  'img_data': img_str
						  }]
			self.send_jn_data(json_data)

			ret, recv_data = self.recv_jn_data()
			if ret:
				if recv_data['err_code'] == 0:
					logging.debug("Registe user:{}, id:{} success".format(face_id, face_id))
		return False
	
	def post_run(self):
		mode = 'a+' if self.append_regist else 'w+'
		with open(self.regist_file, mode) as fw:
			for face_id, file_name in self.id_dict.items():
				content = file_name + ',' + face_id + '\n'
				fw.write(content)


def parse_args():
	parser = argparse.ArgumentParser()
	parser.register("type", "bool", lambda v: v.lower() == "true")
	
	parser.add_argument(
		"--dataset",
		type=str,
		default="",
		help="dataset: lfw_crop, celeba_crop"
	)
	parser.add_argument(
		"--img_dir",
		type=str,
		default="/data/center_160/",
		help="Id img root dir"
	)
	parser.add_argument(
		"--registed_id_file",
		type=str,
		default="/data/CelebA/registed_id.txt",
		help="File to record registed id"
	)
	parser.add_argument(
		"--no_subdir",
		default=True,
		action='store_true',
		help="image in subdir"
	)
	parser.add_argument(
		"--append_regist",
		default=False,
		action='store_false',
		help="Append regist"
	)
	parser.add_argument(
		"--img_ext",
		type=str,
		default=".jpg",
		help="File format."
	)
	
	return parser.parse_known_args()

if __name__ == '__main__':
	FLAGS, unparsed = parse_args()
	img_dir = FLAGS.img_dir
	subdir_flag = FLAGS.no_subdir
	registed_id_file = FLAGS.registed_id_file
	img_ext = FLAGS.img_ext
	detect_in_client = False
	append_regist = FLAGS.append_regist
	print(subdir_flag)
	print(append_regist)
	if len(FLAGS.dataset) > 0:
		img_dir, registed_id_file, detect_in_client = parse_dataset(FLAGS.dataset)
		
	register = Register(ip='127.0.0.1', port=8080, img_dir=img_dir, regist_file=registed_id_file,
						sub_dir_flag=subdir_flag, ext=img_ext, detect_in_client=detect_in_client,
						append_regist=append_regist)
	
	register.setName('register')
	register.setDaemon(True)
	register.start()
	
	register.join()