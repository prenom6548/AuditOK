#!/bin/bash



# cd ./build/
# make -j6
# cd ../

# # 删除并创建 build 目录
# rm -rf ./build
# if [ $? -ne 0 ]; then
#     echo "Failed to remove build directory"
#     exit 1
# fi

mkdir -p build
if [ $? -ne 0 ]; then
    echo "Failed to create build directory"
    exit 1
fi

# 复制 result.py 到 build 目录
cp ./result.py ./build/
if [ $? -ne 0 ]; then
    echo "Failed to copy result.py"
    exit 1
fi

# 进入 build 目录
cd ./build
if [ $? -ne 0 ]; then
    echo "Failed to change directory to build"
    exit 1
fi

# 重新链接
find . -type f -name "UnitTest_*" ! -name "*.*"  |while read file; do
    # 检查文件是否为 ELF 文件
    if file "$file" | grep -q "ELF"; then
        echo "rm $file"
        # 赋予可执行权限
        rm "$file"
    fi
done


# 运行 qmake
if [ -f Makefile ]; then
    echo "Makefile already exists, skipping qmake"
else
    # 运行 qmake
    qmake ..
    if [ $? -ne 0 ]; then
        echo "qmake failed"
        exit 1
    fi
fi


# 运行 make
make -j6
if [ $? -ne 0 ]; then
    echo "make failed"
    exit 1
fi

# # 备份编译结果，使用tar的形式,时间戳
# sleep 1
# tar -czf build.tar.gz *
# cp ./build.tar.gz ../


runResult=0

# 查找带有 UnitTest_ 前缀且不带 .cpp 或 .h 后缀的文件
while IFS= read -r file; do
    # 检查文件是否为 ELF 文件
    if file "$file" | grep -q "ELF"; then
        echo "Running $file"
        # 赋予可执行权限
        chmod +x "$file"
        # 运行程序
        $file
        # if [ $? -ne 0 ]; then
        #     runResult=1
        #     echo "Error: $file"
        #     break
        # fi
    fi
done < <(find . -type f -name "UnitTest_*" ! -name "*.*" | sort)


# echo "runResult: $runResult"

# if [ "$runResult" -ne 0 ]; then
#     echo "Failed to run tests"
#     exit 1
# fi

# 生成覆盖率信息
lcov -d . -c -o coverage.info
if [ $? -ne 0 ]; then
    echo "lcov failed"
    exit 1
fi

# 运行 result.py 脚本
python3 ./result.py
if [ $? -ne 0 ]; then
    echo "result.py execution failed"
    exit 1
fi

echo "All steps completed successfully"
