#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"


#apt install -y gcc
#apt install -y afl++

export AFL_USE_UBSAN=1
export AFL_USE_ASAN=1
export AFL_USE_LSAN=1

qmake kylin-nm.pro
make -j4 CC="afl-cc -D_GNU_SOURCE" CXX="afl-c++ -D_GNU_SOURCE" LD="afl-c++" LINK="afl-c++"

if [ $? -ne 0 ]; then
    echo "Error: build failed."
    exit -1
fi


./fuzz/testSrc/build_fuzz.sh

if [ $? -ne 0 ]; then
    echo "Error: build fuzz test failed."
    exit -1
fi

afl-system-config

echo "start run"

afl-fuzz -t 4000 -i fuzz/in/ -o fuzz/out/ -M master_nm -V 10800 -- ./fuzz/testSrc/libfuzz-test fuzz_kylin_networkmanager @@ &