#!/bin/bash

set -e

cd fuzz

mkdir -p build

cd build

qmake ..

export AFL_USE_UBSAN=1 #启用 UndefinedBehaviorSanitizer(UBSan),用于检测未定义行为。
export AFL_USE_ASAN=1 #启用 AddressSanitizer(ASan),用于检测内存错误(如越界访问、使用已释放的内存等)。
export AFL_USE_LSAN=1 #启用 LeakSanitizer(LSan),用于检测内存泄漏。

make CC="afl-cc" CXX="afl-c++" LINK="afl-c++" #编译

# 准备fuzz测试
cp fuzzy_tests ../

cd ..

# CI 机器可能复用上次 out/；AFL 发现超过 25 分钟的旧结果会直接 abort。
rm -rf out

afl-system-config  #配置系统

# -t: per-exec timeout(ms). AFL 下走轻量 harness，保持足够超时以兼容慢速/ARM CI。
afl-fuzz -i in/ -o out/ -V 10800 -t 120000 -- ./fuzzy_tests   #启动fuzz测试

# 如果需要测试多个二进制程序则可以使用如下命令
# afl-fuzz -M master01 -i in/master1 -o out -- ./harness_A @@ &
# afl-fuzz -S slave02 -i in/master2 -o out -- ./harness_B @@ &
#afl-fuzz -S slave03 -i in/master2 -o out -- ./harness_C @@ &
