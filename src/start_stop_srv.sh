#!/bin/sh

BASE_DIR="$(dirname "$0")"
cd $BASE_DIR

CONF_FILE="../conf/face_id.xml"
NAME=fid_srv

. /lib/init/vars.sh
. /lib/lsb/init-functions

start_srv() 
{
	if [ ! -e ../log ];then
		mkdir log
	fi
	
	nohup ./$NAME -s $CONF_FILE > /dev/null 2>&1 &
	echo "$NAME started"
}

stop_srv() 
{
	if [ `ls | grep ".pid"` ];then
		PID=$(cat *.pid | awk -F',' '{print $1}')
		kill -9 $PID > /dev/null
	else
		killall -9 $NAME > /dev/null
	fi
	sleep 1
	echo "$NAME stop"
}

case "$1" in
	start)
		start_srv
		;;
		
	stop)
		stop_srv
		;;
		
	restart)
		stop_srv
		start_srv
		;;

	status)
		ps aux | grep -v "grep" | grep -v "status" | grep "$NAME" > /dev/null && echo "$NAME is running" || echo "$NAME is not running"
		;;
	*)
		echo "Usage: $NAME {start|stop|restart|status}" >&2
		exit 3
		;;
esac
