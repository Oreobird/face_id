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

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

#include "timing.h"
#include "face_detector.h"

using namespace std;
using namespace cv;

std::string img_file;
std::string img_dir;
std::string crop_face_dir;
std::string model_dir = "/../model/";
std::vector<double> detect_time;
std::vector<double> align_time;


#define TEST_TIME 1

static void usage(char *bin_name)
{
    printf("usage: %s -i image_file -c /data/crop_face/\n", bin_name);
   	printf("	-i: crop one image file.\n");
   	printf("	-d: crop images in dir.\n");
   	printf("	-c: crop face save dir.\n");
   	printf("	-t: detect time.\n");
}


void handle_img(std::unique_ptr<Mtcnn_Detector> &handler, std::string file_path)
{
	cv::Mat img = imread(file_path);
	if (img.empty())
	{
		dbg("imread failed\n");
		return;
	}

	std::vector<face_box_t> faces;

	double start = now();
	handler->detect(img, faces);

	if (faces.size() > 0)
	{
		double end = now();
		double elapsed = calc_elapsed(start, end);
		cout << "detect_time:" << elapsed << endl;
		detect_time.push_back(elapsed);

		cv::Mat img_copy = img.clone();

		start = now();
		std::vector<cv::Mat> faces_mat = handler->align(img_copy, faces);
		end = now();
		elapsed = calc_elapsed(start, end);
		align_time.push_back(elapsed);

		#ifndef TEST_TIME
		cv::Mat face_mat;
		cv::resize(faces_mat[0], face_mat, cv::Size(160, 160));
		std::string face_path = crop_face_dir + basename((char *)file_path.c_str());
		cv::imwrite(face_path, face_mat);
		#endif
	}
}

void walk_dir(std::unique_ptr<Mtcnn_Detector> &handler, std::string dir)
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

	char tmp_file[256] = {0};

	while ((filename = readdir(pdir)) != NULL)
	{
		#ifdef TEST_TIME
		if (detect_time.size() > 1000)
		{
			return;
		}
		#endif

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
	double at_sum = 0.0;
	double dt_avg = 0.0;
	double at_avg = 0.0;
	double total_avg = 0.0;
	int sample_num = detect_time.size();

	for (int i = 0; i < sample_num; ++i)
	{
		dt_sum += detect_time[i];
		at_avg += align_time[i];
	}

	printf("align_sum:%f\n", at_sum);
	dt_avg = dt_sum / sample_num;
	at_avg = at_sum / sample_num;
	total_avg = (dt_sum + at_sum) / sample_num;
	printf("detect avg time: %f\nalign avg time: %f\ntotal avg time: %f\n",
			dt_avg, at_avg, total_avg);
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char ch;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);
	model_dir = prog_path + model_dir;

	while ((ch = getopt(argc, argv, "hi:d:c:")) != EOF)
	{
		switch (ch)
		{
			case 'i':
				img_file = optarg;
				break;
			case 'd':
				img_dir = optarg;
				break;
			case 'c':
				crop_face_dir = optarg;
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

	if (crop_face_dir.empty())
	{
		crop_face_dir = "./crop_face/";
	}

	auto face_handler(std::make_unique<Mtcnn_Detector>(model_dir, crop_face_dir));
	if (face_handler == NULL)
	{
		std::cout << "face handler init fail" << std::endl;
		return -1;
	}

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

