#!/bin/bash
echo "exec \"pip3 install pylint  -i  https://pypi.tuna.tsinghua.edu.cn/simple\""
pip3 install --help | grep -q break-system-packages
if [ $? -eq 0 ]; then
    pip3 install pylint  --break-system-packages  -i  https://pypi.tuna.tsinghua.edu.cn/simple
else
    pip3 install pylint  -i  https://pypi.tuna.tsinghua.edu.cn/simple
fi