
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
 

#ifndef _BASE_CLOSE_HELPER_H
#define _BASE_CLOSE_HELPER_H

NS_BASE_BEGIN

//针对有close()  成员函数
template <class T>
class CloseHelper
{
public:

    CloseHelper(T& obj):m_obj(obj)
    {
    }
    
    ~CloseHelper()
    {
        m_obj.close();
    }
 
private:
    T &m_obj;
};




//模板特化---  针对整数文件类型，使用全局close() API
template <>
class CloseHelper<int>
{
public:
    CloseHelper<int>(int& fd) :m_fd(fd)
    {
    }
    
    ~CloseHelper<int>()
    {
        if (m_fd != -1)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }
 
private:
    int &m_fd;
	
};
 



//模板特化 --- 针对FILE 的关闭fclose
template <>
class CloseHelper<FILE*>
{
public:
    CloseHelper<FILE*>(FILE* fp):m_fp(fp)
    {
    }
    
    ~CloseHelper<FILE*>()
    {
        if (m_fp != NULL)
        {
            fclose(m_fp);
            m_fp = NULL;
        }
    }
 
private:
    FILE *m_fp;
	
};


NS_BASE_END


#endif

