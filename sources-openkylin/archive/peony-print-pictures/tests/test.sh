#!/bin/bash


run_unit_tests() {
    local pattern="unit_test_*"
    
    # 遍历所有匹配模式的目录
    for dir in $pattern; do
        if [ -d "$dir" ] && [ -x "$dir/$(basename "$dir")" ]; then
            echo "Running tests in $dir..."
	    # 执行单元测试
	    (cd "$dir" && "./$(basename "$dir")")
            # 收集代码覆盖率数据
	    echo "Collecting coverage data..."
	    (cd "$dir" && (find ./ -name '*.o' | xargs gcov --preserve-paths))
        else
	    echo "Skipping non-existent or non-executable directory: $dir, $dir/$(basename "$dir")"
        fi
    done
}


generate_result() {
    echo "current pwd : $(pwd)"
    # 收集覆盖率信息
    lcov -d ./build -c -o r.info

    # 删除不需要的文件或路径
    lcov -r r.info "*/tests/*" "*/3rdparty/*" "/usr/include/*" "/opt/*" -o coverage.info

    # 生成html覆盖率报告
    genhtml "$(pwd)/coverage.info" -o result
}


# sudo apt install lcov  libgtest-dev

echo "Compiling the project..."
mkdir build
cd build
qmake .. && make

# 运行各个单元测试
echo "Running unit tests..."
run_unit_tests
cd ..
generate_result
