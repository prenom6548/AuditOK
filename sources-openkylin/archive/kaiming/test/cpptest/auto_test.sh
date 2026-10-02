#!/bin/bash

# 设置退出脚本当命令失败时 (非零退出状态)
#set -e

# 检查 lcov 命令是否存在
if ! command -v lcov >/dev/null 2>&1; then
  echo "Error: lcov is not installed."
  exit 1
fi

export_report=$(pwd)/test/.report

# 函数：运行单元测试
run_unit_tests() {
  local pattern="*cpptest*"

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

# 上传xml到平台
upload_result() {
  # 查找.o并生成.gco
  #find ./ -name '*.o' | xargs gcov --preserve-paths

  sonar-scanner \
    -Dsonar.projectKey=desktop_kaiming \
    -Dsonar.sources=./app \
    -Dsonar.host.url=https://codescan.kylinos.cn \
    -Dsonar.coverageReportPaths=./test/.report/sonarqube-coverage.xml \
    -Dsonar.token=sqp_d314927effd1b28392941c1b6c17f533b7805fad \
    -Dsonar.branch.name=yhkylin/v11-2503
  #\
  #-Dsonar.coverage.exclusions="**"
}

gen_local_lcov() {
  # 切换到项目根目录
  echo "current pwd : $(pwd)"

  # 运行单元测试
  find . -name 'kmtest' -type f -exec ./{} \;

  # 收集覆盖率信息
  mkdir -p ${export_report}
  lcov -d . -c -o ${export_report}/r.info --ignore-errors mismatch

  # 删除不需要的文件或路径
  lcov -r ${export_report}/r.info "$(pwd)/test/cpptest/*" "/usr/include/*" -o ${export_report}/coverage.info
  # 需要根据跑单元测试的项目目录修改
  sed -i "s|/home/test/kaiming/||g" ${export_report}/coverage.info

  # 生成html覆盖率报告
  genhtml "${export_report}/coverage.info" -o ${export_report}/result

  # 生成xml文件
  cp -r $(pwd)/test/cpptest/result.py ${export_report}/result.py
  python3 ${export_report}/result.py --lcov ${export_report}/coverage.info --output ${export_report}/../../debian/kaiming-coverage.xml
 

  # 打开html报告
  xdg-open ${export_report}/result/index.html
}

# 运行各个单元测试
echo "Running unit tests..."
# run_unit_tests

# 生成本地覆盖率报告
echo "Generating local coverage report..."
gen_local_lcov

# 上传到平台,新版本已不需要手动上传
#upload_result

echo "All tests have been run successfully, and coverage data has been collected."
