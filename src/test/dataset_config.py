#coding=utf-8
#__author__ = 'zgs'

def parse_dataset(dataset):
	dataset_dict = {'lfw_crop': ['/data/datasets/LFW/lfw_crop_faces/',
								 '/data/datasets/LFW/registed_id.txt',
								 True],
					'lfw.bin':['E:/ai/aivideo/InsightFace-tensorflow-master/lfw/',
							   'E:/ai/aivideo/InsightFace-tensorflow-master/lfw.bin',
							   True],
					'lfw': ['/data/datasets/LFW/lfw/',
							'/data/datasets/LFW/registed_id.txt',
							False],
					'facescrub': ['/data/datasets/megaface_testpack/facescrub_images/',
								  '/data/datasets/LFW/registed_id.txt',
								  True],
					'celeba_crop': ['/data/datasets/CelebA/Img/celeba_crop_faces/',
									'/data/datasets/CelebA/registed_id.txt',
									True],
					'celeba': ['/data/datasets/CelebA/Img/img_align_celeba/',
							   '/data/datasets/CelebA/registed_id.txt',
							   False],
					'center_160': ['/data/datasets/center_160/',
								   '/data/datasets/center_160/registed_id.txt',
								   True],
					'far': ['/data/zgs/far_faces/', None, True]}
	return dataset_dict[dataset]
