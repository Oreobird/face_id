1. 工程目录
	/data/zgs/face_id/
	|-- src							源码目录
		|-- test 
			|-- register.py			注册人脸
			|-- realtime_tester.py  实时人脸识别测试
			|-- recognize_tester.py 性能测试
	|-- user_db.sh					数据库建表脚本				

2. 编译C++源码
	./build.sh
	
3. 运行服务：
 cd ./build
（1）前台运行：
	./fid_srv -s ../conf/face_id.xml
（2）后台运行：
	./start_stop_srv.sh start
	
4. 注册人脸
（1）批量注册人脸：
	cd /data/AIPlayer/face_id/src/test/
	python register.py
（2）注册单一人脸：
	cd ./build
    ./cli -i img.jpg -t 1 -u user_name -d user_id
（3）识别单一人脸：
	cd ./build
    ./cli -i img.jpg -t 0
5. 通过率与误识率测试
（1）通过率
	cd /data/AIPlayer/face_id/src/test/
	python recognize_tester.py --accuracy
（2）误识率
	cd /data/AIPlayer/face_id/src/test/
	python recognize_tester.py --far
	
6. 实时人脸识别测试
	python realtime_tester.py

其他：
目前数据库名为 face_db，用户:AIPlayer, 密码：123456
建立测试数据库步骤：
（1）./user_db.sh	重新建立数据库表
（2）启动fid_srv服务
（3）运行批量注册人脸脚本
（4）导出数据库 
	mysqldump -u AIPlayer -p hd_face_db > 数据库名.sql
	输入密码:123456

算法对比测试步骤：
（1）./user_db.sh	重新建立数据库表
（2）导入数据库
	mysql -u AIPlayer -p hd_face_db < 数据库名1.sql
（3）启动fid_srv服务
（4）运行识别人脸测试程序
（5）关闭fid_srv服务，重复步骤1~4，在步骤2中导入对比测试数据库



	

