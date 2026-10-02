#!/bin/bash

hw_arch=$(uname -m)

echo $hw_arch|logger
if [ $hw_arch = "loongarch64" ]
then
	echo "gdb loongarch64"|logger
	gdb /usr/bin/miracle-wifid -batch -ex "set args --use-dev --log-level info --lazy-managed --config-methods display --wpa-loglevel trace" -ex "set logging file /home/execstart.gdb" -ex "set logging on" -ex "run" -ex "thread apply all backtrace" -ex "quit"
	echo "gdb loongarch64 success"|logger
else
	echo "other arch"|logger
	/usr/bin/miracle-wifid \
	--use-dev \
	--log-level info \
	--lazy-managed \
	--config-methods display 
	echo "other arch success"|logger
fi

