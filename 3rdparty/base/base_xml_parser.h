
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
 * Author: cuipingxu918@qq.com
 */
 

#ifndef _BASE_XML_PARSER_H
#define _BASE_XML_PARSER_H

#include "base_common.h"
#include "tinyxml.h"


NS_BASE_BEGIN


class XML_Node
{
public:

	std::string get_text();

	std::string get_attr_str(std::string key);

	template <typename T>
	T get_attr(std::string key);

	std::string to_string();
	
public:
	std::string m_text;
	std::map<std::string, std::string> m_mapAttr;
};




/*
【注意】: tinyxml解析的xml文件标签名称只允许全英文字符
*/
class XML_Parser
{
public:

	int parse_file(std::string xmlFile);

	int parse_str(std::string xmlStr);

	int get_nodes(std::string key, std::vector<XML_Node> &vecNode);
	
	int get_node(std::string key, XML_Node &node); 

private:
	int get_node(TiXmlElement *father_element, std::list<std::string> &lstKey, 
		std::vector<XML_Node> &vecNode);

	void create_node(TiXmlElement *element, std::vector<XML_Node> &vecNode);
		
private:
	TiXmlDocument m_document;

};


NS_BASE_END


#include "base_xml_parser.inl"


#endif


