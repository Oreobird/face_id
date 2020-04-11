
#!/bin/bash

user=AIPlayer
password=123456
mysql_exec="/usr/bin/mysql -h127.0.0.1 -u$user -p$password"


##########################
#【用户人脸特征数据库】
##########################
db=face_db


$mysql_exec -e "drop database $db"
$mysql_exec -e "create database $db"

#-------------------------
# 用户表
#-------------------------
echo "begin create ${db}.tbl_user table";	
$mysql_exec $db -e "CREATE TABLE ${db}.tbl_user(
		id varchar(18) COMMENT '唯一用户ID编号',
		name varchar(100) COMMENT '用户名称',
		feat_vec varchar(10000) COMMENT '人脸特征',		
		created_at bigint unsigned COMMENT '创建时间戳',
		updated_at bigint unsigned COMMENT '上次更新时间戳',		
		PRIMARY KEY(id)
	) ENGINE=InnoDB DEFAULT  CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci";
