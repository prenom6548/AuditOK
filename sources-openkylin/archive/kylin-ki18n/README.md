## CMake 构建

请在**单独目录**里配置，勿在仓库根执行 `cmake .`，否则会在根目录留下 `CMakeFiles/` 等垃圾：

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

打包清理：`./debian/rules clean` 会删除 `build/`、所有 `build-*` 试编译目录，以及根目录误生成的 CMake 缓存。

---

## 测试与代码覆盖率

### 运行测试

```bash
cd kylin-ki18n
cargo test
```

### 运行代码覆盖率（使用 cargo-tarpaulin）

在 Linux 上可以使用 `cargo-tarpaulin` 生成覆盖率报告：

```bash
cd kylin-ki18n

# 安装 tarpaulin（全局一次即可）
cargo install cargo-tarpaulin

# 运行覆盖率并在终端输出摘要
cargo tarpaulin

# 生成 HTML 报告（推荐）
cargo tarpaulin --out Html
```

生成的 HTML 报告通常位于 `kylin-ki18n/tarpaulin-report.html` 或 `kylin-ki18n/tarpaulin-report/` 目录下，可用浏览器打开查看各模块的覆盖率情况。

