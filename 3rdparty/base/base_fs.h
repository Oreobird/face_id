
/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: 89617663@qq.com
 */
 

#ifndef _BASE_FS_H
#define _BASE_FS_H

#include "base_common.h"
#include <sys/types.h>
#include <sys/stat.h>

NS_BASE_BEGIN

int getFilesByDirectory_api(const std::string directory, std::list<std::string> &lstFiles);

int getFileSize_stat(const std::string file, unsigned long long &size);

std::string getCWD();

int changeDirectory(const std::string directory);

int moveFile(const std::string srcFile, const std::string dstFile);

//file仅是文件名
bool isFileExist(const std::string file, const std::string directory);

//#include <sys/stat.h>
int getFileStat(std::string file, struct stat &filestat);

bool isDirectory(const std::string directory);

bool isFile(const std::string file);

int chmod(const std::string file, const int mode);

int basename(std::string path, std::string &dname, std::string &bname);
		

NS_BASE_END

#endif


