
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
 

#ifndef _BASE_OS_H
#define _BASE_OS_H

#include "base_common.h"
#include <sys/resource.h>

NS_BASE_BEGIN


bool isBigEndian();

pid_t get_pid();

pid_t get_ppid();

unsigned long get_thread_id();

bool is_pid_exist(pid_t pid);

int get_rlimit(int resource, struct rlimit *rlim);

int get_cpu_number_proc(unsigned short &num);

NS_BASE_END


#endif


