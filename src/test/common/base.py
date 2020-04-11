#coding=utf-8
#__author__ = 'peterzhong'

import traceback
import json
import logging
import os

class JsonUtil:
    @staticmethod
    def Dumps(jn_data):
        try:
            data = json.dumps(jn_data)
            return data
        except Exception as ex:
            return  ""

    @staticmethod
    def Loads(data):
        try:
            jn_data = json.loads(data)
            return jn_data
        except Exception as ex:
            #print(ex)
            logging.error("json loads str fail, ",  traceback.format_stack())
            return {}


class JsonCompareUtil:
    @staticmethod
    def ContainCompare(jn_large_data, jn_small_data, prefix_key = ""):
        for k, v in jn_small_data.items():
            if k not in jn_large_data:
                error_info = "recv data not contain key %s which is expected "%( k)
                logging.error(error_info)
                return False, error_info

            large_v = jn_large_data[k]
            if not isinstance(large_v, (dict, list)):
                if v != large_v:
                    error_info = "key: {" + prefix_key + k + "} value not equal, expected: "+ str(v) + ", real:" + str(large_v)
                    logging.error(error_info)
                    return False, error_info
            elif isinstance(large_v, (dict)):
                prefix = prefix_key + k + "."
                ret, error_info = JsonCompareUtil.ContainCompare(large_v, v, prefix)
                if not ret:
                    return False, error_info

        return True, ""

class DirUtil:
    @staticmethod
    def CreateDir(path):
        path = path.strip()
        path = path.rstrip("\\")

        isExists = os.path.exists(path)
        if not isExists:
            os.makedirs(path)

        return True

class FileUtil:
    @staticmethod
    def CreateFile(file_name, mode="wt"):
        try:
            file = open(file_name, mode)
            return file
        except Exception as ex:
            logging.error("open file fail, ",str(ex))
            return None


class PrintUtil:
    @staticmethod
    def print_error(content):
        print("\033[1;31;40m%s\033[0m"%(content))
        # print(content)