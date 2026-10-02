#!/bin/bash

client_user=$1
client_uuid=$2
server_ip=$3
server_user=$4
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ];then
	echo "1,Parameter input error."
	exit 1
fi
SUPPORT_ARCHS=( "arm64" "x86_64" "x64" "loongarch64" "aarch64" "loong64" "mips64el" "mips64");

arch=$(uname -m)

echo "${SUPPORT_ARCHS[@]}" | grep -wq "$arch" &&  support="Yes" || support="No"
if [ $support == "No" ];then
    echo "1,Current arch not support."
	exit 0
fi

echo $client_user $client_uuid $server_ip $server_user
uuidinfo=$client_user$client_uuid$server_user$server_ip
# echo $uuidinfo
hashkey=`echo $uuidinfo |md5sum |awk '{print $1}'`
# echo $hashkey
SESSION="${hashkey}"
SERVER=~/.kylin-ide-server
TOKENFILE=$SERVER/data/.$hashkey.token
# RUNNINGPID=$(ps -aux | grep $SESSION | grep  server-main.js | grep -v grep | awk '{print $2}')
RUNNINGPID=$(ps -aux | grep $SESSION | grep  server-main.js | grep -v grep | awk '{print $2}')
WEB_SERVER_PATH=$SERVER/server/node
# os=$(cat /etc/os-release | grep "PRETTY_NAME" | awk -F '=' '{print $2}')

# echo "runningpid:$RUNNINGPID arch:${arch}  os :${os}"

STATUS=("RUNNING" "TOSTART" "TODEPLOY"  "ERROR")


# echo "----------"
if [ -n "$RUNNINGPID" ];then
    # isrunning=1
	stat=${STATUS[0]}
	[ -e  $TOKENFILE ] && token=$(cat $TOKENFILE) || token=""
else
    # isrunning=0
	test -f $WEB_SERVER_PATH && stat=${STATUS[1]} || stat=${STATUS[2]}
fi
# echo $stat
#todeploy 中找软件包，找不到报错  或者脚本和软件包在一个目录下载解压，然后运行脚本 进行cp
error_token="1,Token not found ."
error_stat="1,status error."

run_webide_server(){
	SERVER_DATA=$SERVER/data
	[ -d $SERVER_DATA ] || mkdir $SERVER_DATA
	SERVER_BIN=$SERVER/server/bin/kylin-ide-server
	LOGFILE=$SERVER_DATA/${hashkey}.log
	TOKEN=$(cat /proc/sys/kernel/random/uuid)
	echo $TOKEN >$SERVER_DATA/.${hashkey}.token
	echo "token=$TOKEN"
	if [ token == "" ]
	then
		echo $error_token
	else
		[ -e $SERVER_DATA/.${hashkey}.sock ] && rm -rf $SERVER_DATA/.${hashkey}.sock
		$SERVER_BIN --socket-path=$SERVER_DATA/.${hashkey}.sock --remotename=${server_user}@${server_ip} --connection-token-file=$SERVER_DATA/.${hashkey}.token --server-data-dir=$SERVER_DATA --enable-remote-auto-shutdown  > $LOGFILE 2>&1 &
	fi
}
prepare_start_remote_server(){
	stat=$1
	case $stat in
		"RUNNING")
			# echo "start RUNNING"
			if [ token == "" ]
			then
				echo $error_token
			else
				# selfSock = `/tmp/${hashkey}.sock`   客户端sock
				serverSock="/home/${server_user}/.kylin-ide-server/data/.${hashkey}.sock"
				echo "0,${serverSock},${token}"
			fi
		;;
		"TOSTART")
			# echo "start TOSTART"
			run_webide_server
			
		;;
		"TODEPLOY")
			
			# echo "start deploy"
			if [ ! -d ~/.kylin-ide-server/server ];then
				mkdir -p ~/.kylin-ide-server/server

				
			fi
			# find -type
			codepath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
			# echo $codepath
			echo ${codepath}  
		
			echo ${arch}
			pkg="$(ls "${codepath}" | grep "^kylin-ide-server_\([0-9]*.\)\{2\}[0-9]*_${arch}"|sort -r | head -n 1)"

			echo $pkg
			if [ -z $pkg ];then
			
				echo "1,not find package in ${codepath}"
				exit 1
			fi
			
			tar -xf ${codepath}/${pkg} -C ~/.kylin-ide-server/server;
			

			run_webide_server
		;;
		*)
		# echo "===error"
		echo $error_stat
		exit 1
	esac
}

# echo $stat
prepare_start_remote_server $stat

exit 0
