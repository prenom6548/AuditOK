
#!/bin/bash
# 脚本示例

# set -e

# 生成fifo文件 与脚本自身同一目录
selfPath=`dirname $0`

# 添加公共函数
. $selfPath/../functions

export fifo_name=
export log_path="/var/log/plugin-manager/plugin.log"
export log_level=0
export mod_name="default"

for x in $* ; do
    case $x in
    fifoname=*)
        fifo_name=${x#fifoname=}
        ;;
    loglevel=*)
        log_level=${x#loglevel=}
        set_logLevel $log_level
        ;;
    logpath=*)
        log_path=${x#logpath=}
        ;;
    modename=*)
        mod_name=${x#modename=}
        ;;
    esac
done

# 模块日志路径
if [ -n "$log_path" ]; then
    exec >$log_path 2>&1
fi

# 模块日志函数
M2_LOG_DBG()
{
    LOG_DBG "["$mod_name"]:""$*"
}

# 输出开始标志
M2_LOG_DBG "Start！ cmd is $*"

# 进度FIFO文件
fifoPath=$selfPath"/"$fifo_name
if [ ! -e ${fifoPath} ]; then
    mkfifo -m 666 ${fifoPath}
fi
exec 6<>${fifoPath}




# 输出进度示例
i=0
while :
do
    M2_LOG_DBG $i
    echo $i >&6     # 输出进度

    let i++
    sleep 1

    if [ $i -gt 5 ];then

        exec 6<&-
        rm -r ${fifoPath}
        # 输出结束标志
        M2_LOG_DBG "$mod_name end."
        exit 0
    fi
done

# 输出结束标志
M2_LOG_DBG "$mod_name end!"