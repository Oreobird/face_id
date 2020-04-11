#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <dirent.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include "conf_mgt.h"
#include "timing.h"
#include "face_recognizer.h"

using namespace std;
using namespace cv;

int id_dir_depth = 1;
std::string img_file;
std::string img_dir;
std::string feature_dir;
std::string model_dir = "/../model/";
std::vector<double> get_feature_time;
extern StModelInfo g_modelInfo;

struct head_t
{
	int32_t rows;
	int32_t cols;
	int32_t step;
	int32_t dtype;
};

#define TEST_TIME 1

static void usage(char *bin_name)
{
    printf("usage: %s -d /data/crop_face/ -f \n", bin_name);
   	printf("	-i: one image file.\n");
   	printf("	-d: images in dir.\n");
   	printf("	-f: feature output save dir.\n");
   	printf("	-t: image dir depth.\n");
}

void print_feat_vec(std::vector<float> &feat_vec)
{
	dbg("feat vector:\n");
	for (auto feat: feat_vec)
	{
		dbg("%f ", feat);
	}
	dbg("\n");
}

void save_feature(const std::string &out_file, std::vector<float> &feature)
{
	struct head_t head;
	FILE *fp = NULL;

	head.rows = feature.size();
	head.cols = 1;
	head.step = 4;
	head.dtype = 5;

	fp = fopen(out_file.c_str(), "wb");
	if (fp == NULL)
	{
		dbg("fopen failed\n");
		return;
	}

	fwrite(&head, 4, 4, fp);
	fwrite(feature.data(), 4, feature.size(), fp);
	fclose(fp);
}

std::string get_dir_name_by_slash(std::string &file_path, int rnum)
{
	int pre_idx = -1;
	int post_idx = -1;
	std::string sub_path = file_path;
	std::string result;

	for (int i = 0; i < rnum; i++)
	{
		pre_idx = sub_path.find_last_of('/');
		sub_path = file_path.substr(0, pre_idx);
		result = file_path.substr(pre_idx + 1, post_idx - pre_idx - 1);
		post_idx = pre_idx;
	}

	return result;
}

void handle_img(std::unique_ptr<InsightFace_Recognizer> &handler, std::string file_path, int dir_depth=id_dir_depth)
{
	cv::Mat img = imread(file_path);
	if (img.empty())
	{
		dbg("imread failed\n");
		return;
	}

    face_box_t face;
    face.box.x1 = 0;
    face.box.y1 = 0;
    face.box.x2 = img.cols - 1;
    face.box.y2 = img.rows - 1;

	double start = now();

	handler->extract_feat_vec(img, face, true);

	double end = now();
	double elapsed = calc_elapsed(start, end);
	std::cout << "get_feature_time:" << elapsed << std::endl;
	get_feature_time.push_back(elapsed);

	std::string dir_name;

	for (int i = dir_depth; i > 1; --i)
	{
		dir_name = dir_name + "/" + get_dir_name_by_slash(file_path, i);
		mkdir((feature_dir + dir_name).c_str(), 0755);
	}

	std::string out_dir = feature_dir + dir_name;
	std::cout << "out_dir:     " << out_dir << std::endl;


	std::string fname = get_dir_name_by_slash(file_path, 1) + "_tf-insightface.bin";
	std::cout << "fname: " << fname << std::endl;

	std::string out_file = out_dir + "/" + fname;
	std::cout << "out_file:  " << out_file << std::endl;

	//print_feat_vec(face.feat_vec);

	save_feature(out_file, face.feat_vec);
}

void walk_dir(std::unique_ptr<InsightFace_Recognizer> &handler, std::string dir)
{
	struct stat fs;
	lstat(dir.c_str(), &fs);

	if (!S_ISDIR(fs.st_mode))
	{
		cout << "dir is not a valid dir"<<endl;
		return;
	}

	struct dirent *filename;
	DIR *pdir = NULL;

	pdir = opendir(dir.c_str());
	if (!pdir)
	{
		cout << "can not open dir" << dir << endl;
		return;
	}

	cout << "Open dir:"<<dir<<endl;

	char tmp_file[512] = {0};

	while ((filename = readdir(pdir)) != NULL)
	{
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0)
		{
			continue;
		}

		snprintf(tmp_file, sizeof(tmp_file), "%s/%s", dir.c_str(), filename->d_name);
		struct stat fs;
		lstat(tmp_file, &fs);

		if (S_ISDIR(fs.st_mode))
		{
 			cout << filename->d_name << endl;
			walk_dir(handler, tmp_file);
		}
 		else
 		{
 			printf("%s\n", tmp_file);
 			handle_img(handler, tmp_file);
 		}
 	}

	closedir(pdir);
}

void calc_time(void)
{
	double dt_sum = 0.0;
	double dt_avg = 0.0;
	double at_avg = 0.0;
	double total_avg = 0.0;
	int sample_num = get_feature_time.size();

	for (int i = 0; i < sample_num; ++i)
	{
		dt_sum += get_feature_time[i];
	}

	dt_avg = dt_sum / sample_num;
	total_avg = dt_sum / sample_num;
	printf("detect avg time: %f\n total avg time: %f\n",
			dt_avg, total_avg);
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char ch;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);
	model_dir = prog_path + model_dir;

	while ((ch = getopt(argc, argv, "hi:d:f:t:")) != EOF)
	{
		switch (ch)
		{
			case 'i':
				img_file = optarg;
				break;
			case 'd':
				img_dir = optarg;
				break;
			case 'f':
				feature_dir = optarg;
				break;
			case 't':
				id_dir_depth = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_file.empty() && img_dir.empty())
	{
		usage(argv[0]);
		return -1;
	}

	if (feature_dir.empty())
	{
		feature_dir = "./features/";
	}

	g_modelInfo = PSGT_Conf_Mgt->get_modelinfo();

	cout << feature_dir << endl;
	auto face_handler(std::make_unique<InsightFace_Recognizer>(model_dir, feature_dir));
	if (face_handler == NULL)
	{
		std::cout << "face handler init fail" << std::endl;
		return -1;
	}

	dbg("111\n");
	if (!img_dir.empty())
	{
		walk_dir(face_handler, img_dir);
	}
	else if (!img_file.empty())
	{
		handle_img(face_handler, img_file);
	}

	calc_time();

    return 0;
}

