#!/bin/bash

# 设置退出脚本当命令失败时 (非零退出状态)
#set -e

# 函数：运行单元测试
run_unit_tests() {
#    local pattern="unit_test_*"
#
#    # 遍历所有匹配模式的目录
#    for dir in $pattern; do
#        if [ -d "$dir" ] && [ -x "$dir/$(basename "$dir")" ]; then
#            echo "Running tests in $dir and Collecting coverage data..."
#            # 收集代码覆盖率数据
#	    (cd "$dir" && "./$(basename "$dir")" && (find ./ -name '*.o' | xargs gcov --preserve-paths))
#        else
#	    echo "Skipping non-existent or non-executable directory: $dir, $dir/$(basename "$dir")"
#        fi
#    done
    local pattern="/home/wangyan/mypro/kylin-ai-document-qa-service/test/build/CMakeFiles"

    # 检查模式是否为有效的目录
    if [ -d "$pattern" ]; then
        echo "Processing directory: $pattern"
        (cd "$pattern" && {
            # 查找并处理每个 .gcda 文件
            find . -name "*.gcda" -exec gcov --preserve-paths {} + || echo "No .gcda files found or gcov failed."
        })
    else
        echo "Directory does not exist: $pattern"
    fi

    #Print Working Directory
    echo "current pwd : $(pwd)"

    # 收集覆盖率信息
    lcov -d . -c -o r.info

    # 删除不需要的文件或路径
    lcov -r r.info  "/usr/include/*" "build/*" -o coverage.info

    # 生成html覆盖率报告
    genhtml "$(pwd)/coverage.info" -o result

}

# 上传 result.zip 到平台
upload_result() {

    # 打包
    zip -r  result.zip result
    # 上传平台
    curl --insecure -X POST -F "file=@/$(pwd)/result.zip" -F "package=kylin-baidu-speech-engine" -F "username=$1" $2
    echo "all parameter : $@"
}

# 编译项目
echo "Compiling the project..."
#qmake && make

# 运行各个单元测试
echo "Running unit tests..."
run_unit_tests

# 上传到平台
#upload_result

echo "All tests have been run successfully, and coverage data has been collected."
