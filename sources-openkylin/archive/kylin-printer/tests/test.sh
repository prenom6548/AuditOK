#!/bin/bash



single_test() {
    dir=$1

    if [ -d "$dir" ] && [ -x "$dir/$(basename "$dir")" ]; then
        echo "Running tests in $dir..."
        # 执行单元测试
        (cd "$dir" && "./$(basename "$dir")")
    else
        echo "Skipping non-existent or non-executable directory: $dir, $dir/$(basename "$dir")"
    fi
}

collect() {
    dir=$1
    if [ -d "$dir" ]; then
        echo "Running tests in $dir..."
        # 收集代码覆盖率数据
        echo "Collecting coverage data..."
        (cd "$dir" && (find ./ -name '*.o' | xargs gcov --preserve-paths))
    else
        echo "Skipping non-existent or non-executable directory: $dir, $dir/$(basename "$dir")"
    fi
}


run_unit_tests() {
    local pattern="unit_test_*"
    
    # 遍历所有匹配模式的目录
    for dir in $pattern; do
        single_test $dir
    done

    for dir in $pattern; do
        collect $dir
    done

    collect "lib-kylin-printer"
}


generate_result() {
    echo "current pwd : $(pwd)"
    # 收集覆盖率信息
    lcov -d ./build -c -o r.info

    # 删除不需要的文件或路径
    lcov -r r.info  "*/3rdparty/*" "*/tests/*" "/usr/include/*" "/opt/*" -o coverage.info

    # 生成html覆盖率报告
    genhtml "$(pwd)/coverage.info" -o result
}

do_pre_task() {
    lpadmin -p unit_test_printer -E -v cups-pdf:/ -m drv:///cupsfilters.drv/textonly.ppd
}

do_after_task() {
    killall kylin-user-guide
}

# sudo apt install lcov  libgtest-dev

echo "Compiling the project..."
mkdir build
cd build
cmake .. && make -j8

do_pre_task

# 运行各个单元测试
echo "Running unit tests..."
run_unit_tests

do_after_task

cd ..
generate_result
