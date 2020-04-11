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
import pickle
import numpy as np

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


class Evaluater(client.Client):
	def __init__(self, ip="", port=1000, img_dir="", bin_path="lfw.bin",
				 ext='.jpg', detect_in_client=False, test_mode=1):
		client.Client.__init__(self, conn=None, ip=ip, port=port)
		self.img_param = []
		self.img_dir = img_dir
		self.pair_dict = {}
		self.ext = ext
		self.detect_in_client = detect_in_client
		self.bin_path = bin_path
		self.evalute_result = [0, 0, 0, 0, 0]
		self.test_mode = test_mode
	
	def regist(self, file_path, user):
		img = cv2.imread(file_path)
		if img is None:
			return
		_, img_encode = cv2.imencode('.jpg', img, self.img_param)
		img_str = base64.b64encode(img_encode).decode('utf-8')
		
		json_data = [{'id': str(user), 'name': str(user), 'type': 1,
					  'detect_in_client': self.detect_in_client, 'mode': 1,
					  'img_data': img_str
					  }]
		self.send_jn_data(json_data)
		ret, recv_data = self.recv_jn_data()
		if ret:
			if recv_data['err_code'] == 0:
				logging.debug("Registe user:{} Success".format(user))
			else:
				logging.debug("Registe user:{} Failed".format(user))
		return ret
		
		
	def withdraw(self, file_path, user):
		img = cv2.imread(file_path)
		_, img_encode = cv2.imencode('.jpg', img, self.img_param)
		img_str = base64.b64encode(img_encode).decode('utf-8')
		json_data = [{'id': str(user), 'name': str(user), 'type': 2,
					  'detect_in_client': self.detect_in_client, 'mode': 1,
					  'img_data': img_str
					  }]
		self.send_jn_data(json_data)
		ret, recv_data = self.recv_jn_data()
		if ret:
			if recv_data['err_code'] == 0:
				logging.debug("Withdraw user:{} Success".format(user))
			else:
				logging.debug("Withdraw user:{} Failed".format(user))
		return ret
		
		
	def recognize(self, file_path, true_user, threshhold=1.0):
		img = cv2.imread(file_path)
		if img is None:
			return
		_, img_encode = cv2.imencode('.jpg', img, self.img_param)
		img_str = base64.b64encode(img_encode).decode('utf-8')
		
		json_data = [{'id': str(true_user), 'name': str(true_user), 'type': 0,
					  'detect_in_client': self.detect_in_client, 'mode': 1,
					  'threshhold': threshhold, 'img_data': img_str
					  }]
		self.send_jn_data(json_data)
		ret, recv_data = self.recv_jn_data()
		if ret:
			if recv_data['err_code'] == 0:
				user_name = recv_data['user_name']
				if user_name == "unknown":
					return False
				else:
					return True
			
					
	def calculate_accuracy(self, predict_issame, actual_issame):
		tp = np.sum(np.logical_and(predict_issame, actual_issame))
		fp = np.sum(np.logical_and(predict_issame, np.logical_not(actual_issame)))
		tn = np.sum(np.logical_and(np.logical_not(predict_issame), np.logical_not(actual_issame)))
		fn = np.sum(np.logical_and(np.logical_not(predict_issame), actual_issame))
		
		tpr = 0 if (tp + fn == 0) else float(tp) / float(tp + fn)
		fpr = 0 if (fp + tn == 0) else float(fp) / float(fp + tn)
		acc = float(tp + tn) / len(actual_issame)
		return [tpr, fpr, acc]
	
	def calculate_tar_far(self, predict_issame, actual_issame):
		true_accept = np.sum(np.logical_and(predict_issame, actual_issame))
		false_accept = np.sum(np.logical_and(predict_issame, np.logical_not(actual_issame)))
		n_same = np.sum(actual_issame)
		n_diff = np.sum(np.logical_not(actual_issame))
		tar = float(true_accept) / float(n_same)
		far = float(false_accept) / float(n_diff)
		return [tar, far]
	
	def pre_run(self):
		self.img_param = [int(cv2.IMWRITE_JPEG_QUALITY), 100]
		_, issame_list = pickle.load(open(self.bin_path, 'rb'), encoding='bytes')
		
		files = os.listdir(self.img_dir)
		files.sort(key=lambda x:int(x[:-4]))
		img_idx = 0
		for idx in range(len(issame_list)):
			user = os.path.basename(files[idx]).split('.')[0]
			self.pair_dict[user] = [os.path.join(self.img_dir, files[img_idx]),
									os.path.join(self.img_dir, files[img_idx + 1]),
									issame_list[idx]]
			img_idx += 2
			print("{}, {}".format(user, self.pair_dict[user]))
			
		print("pairs num:%d" % len(self.pair_dict))
		time.sleep(0.5)
		
	def evaluate_1_1(self):
		predict_issame = []
		actual_issame = []
		for regist_user, item in tqdm(self.pair_dict.items()):
			regist_file_path, test_file_path, issame = item
			
			ret = self.regist(regist_file_path, regist_user)
			if not ret:
				continue
			
			predict_result = self.recognize(test_file_path, regist_user, threshhold=1.0)
			predict_issame.append(predict_result)
			actual_issame.append(issame)
			# time.sleep(0.1)
			ret = self.withdraw(regist_file_path, regist_user)
			if not ret:
				raise Exception("Withdraw failed")
			# time.sleep(0.1)
		self.evalute_result = self.calculate_accuracy(predict_issame, actual_issame) \
							  + self.calculate_tar_far(predict_issame, actual_issame)
		
	def evaluate_1_N(self):
		pass
		
		
	def do_task(self):
		if self.test_mode == 1:
			self.evaluate_1_1()
		else:
			self.evaluate_1_N()
			
		return False
	
	def post_run(self):
		tpr, fpr, acc, tar, far = self.evalute_result
		acc_mean = np.mean(acc)
		acc_std = np.std(acc)
		tar_mean = np.mean(tar)
		tar_std = np.std(tar)
		far_mean = np.mean(far)
		far_std = np.std(far)
		print('eval on %s: acc--%1.5f+-%1.5f, tar--%1.5f+-%1.5f, far--%1.5f+-%1.5f' %
			  (self.bin_path, acc_mean, acc_std, tar_mean, tar_std, far_mean, far_std))


def parse_args():
	parser = argparse.ArgumentParser()
	parser.register("type", "bool", lambda v: v.lower() == "true")
	
	parser.add_argument(
		"--dataset",
		type=str,
		default="lfw.bin",
		help="dataset: lfw.bin"
	)
	parser.add_argument(
		"--img_dir",
		type=str,
		default="E:/ai/aivideo/InsightFace-tensorflow-master/lfw/",
		help="Img root dir"
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
	
	img_ext = FLAGS.img_ext
	detect_in_client = False

	
	if len(FLAGS.dataset) > 0:
		img_dir, bin_path, detect_in_client = parse_dataset(FLAGS.dataset)
		
	evaluater = Evaluater(ip='10.101.76.2', port=8080, img_dir=img_dir, bin_path=bin_path,
						  ext=img_ext, detect_in_client=detect_in_client, test_mode=1)
	
	evaluater.setName('evaluater')
	evaluater.setDaemon(True)
	evaluater.start()
	
	evaluater.join()