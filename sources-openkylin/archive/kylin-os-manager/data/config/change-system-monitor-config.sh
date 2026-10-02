#!/bin/dash
config_CpuThreshold=80
config_MemoryThreshold=80
config_Timer=120

configPath="/root/.kylin-os-manager"
configFile=${configPath}"/kylin-os-manager-plugin.ini"

key_headerStr=SystemMonitor
key_header="[${key_headerStr}]"
key_CpuThreshold="CpuThreshold="
key_MemoryThreshold="MemoryThreshold="
key_Timer="Timer="

if [ ! -d ${configPath} ]; then
    mkdir ${configPath}
fi

if [ ! -f "${configFile} " ];then
    touch ${configFile}
fi

if [ `grep -c ${key_header} ${configFile}` -eq '0' ];then
    echo ${key_header} >> ${configFile}
    echo ${key_CpuThreshold}${config_CpuThreshold} >> ${configFile}
    echo ${key_MemoryThreshold}${config_MemoryThreshold} >> ${configFile}
    echo ${key_Timer}${config_Timer} >> ${configFile}
else
  line=$(sed -n "/${key_headerStr}/=" ${configFile})
  if [ `grep -c ${key_CpuThreshold} ${configFile}` -eq '0' ];then
    sed -i "${line} a\\${key_CpuThreshold}${config_CpuThreshold}" ${configFile}
  else
    sed -i "/^${key_CpuThreshold}/s/=.*/=${config_CpuThreshold}/" ${configFile}
  fi
  if [ `grep -c ${key_MemoryThreshold} ${configFile}` -eq '0' ];then
    sed -i "${line} a\\${key_MemoryThreshold}${config_MemoryThreshold}" ${configFile}
  else
    sed -i "/^${key_MemoryThreshold}/s/=.*/=${config_MemoryThreshold}/" ${configFile}
  fi
    if [ `grep -c ${key_Timer} ${configFile}` -eq '0' ];then
    sed -i "${line} a\\${key_Timer}${config_Timer}" ${configFile}
  else
    sed -i "/^${key_Timer}/s/=.*/=${config_Timer}/" ${configFile}
  fi
fi

systemctl restart com.kylin-os-manager.service
