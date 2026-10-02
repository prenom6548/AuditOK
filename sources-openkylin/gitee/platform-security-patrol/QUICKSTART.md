# 快速开始指南

## 1. 环境准备

### 安装依赖（以 Ubuntu/OpenKylin 为例）

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config
```

### 验证编译器版本

```bash
gcc --version  # 应支持 C11 标准
cmake --version  # 应 >= 3.10
```

## 2. 构建项目

```bash
# 进入项目目录
cd platform-security-patrol

# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake ..

# 编译
make -j$(nproc)
```

### 预期输出

成功编译后，build 目录中的关键产物：

```
build/
├── plat-sec-patrol                       # 主程序
├── libplat_common.so                     # 公共共享库（日志、工具函数、cJSON）
└── plugins/
    ├── environment/
    │   ├── cpu_collector/
    │   │   └── libcpu_hygon_collector.so    # 海光 CPU 信息采集插件
    │   ├── firmware_collector/
    │   │   └── libfirmware_collector.so     # 固件信息采集插件
    │   └── os_collector/
    │       └── libos_collector.so           # OS 信息采集插件
    └── capability/
        └── hygon/
            └── libhygongenuine_capability.so  # 海光平台安全能力库插件
```

> **注意：** 构建时各插件位于对应的子目录中。运行时需将所有插件集中到同一扁平目录，或使用 `make install` 安装后运行。

## 3. 安装

由于插件在构建目录中分散于各子目录，而运行时要求所有 `.so` 文件位于同一扁平目录中，建议先安装再运行：

```bash
# 安装到系统目录（默认 /usr/local）
sudo make install

# 验证安装
which plat-sec-patrol
plat-sec-patrol --version
```

安装后的文件位置：

| 类型 | 路径 |
|------|------|
| 可执行文件 | `/usr/local/bin/plat-sec-patrol` |
| 插件（扁平目录） | `/usr/local/lib/plat-sec-patrol/plugins/` |
| 公共共享库 | `/usr/local/lib/libplat_common.so` |
| 能力定义 JSON | `/usr/local/share/plat-sec-patrol/capabilities/hygon_capabilities.json` |
| 主头文件 | `/usr/local/include/plat-sec-patrol/plat_sec_patrol.h` |
| 数据模型头文件 | `/usr/local/include/plat-sec-patrol/models/` |
| 插件接口头文件 | `/usr/local/include/plat-sec-patrol/plugins/` |

## 4. 运行

### 基本用法

`check`、`enable`、`disable` 命令需要 root 权限。`list` 命令不需要。

```bash
# 查看所有已加载的安全能力（默认文本表格格式，需要 root）
sudo plat-sec-patrol check

# 查看某个具体能力的详细状态
sudo plat-sec-patrol check csv1

# 列出所有可用的能力定义（需要 root）
sudo plat-sec-patrol list

# 使能某个能力
sudo plat-sec-patrol enable csv1

# 禁用某个能力
sudo plat-sec-patrol disable csv1
```

### 输出格式

```bash
# JSON 格式输出
sudo plat-sec-patrol check --format json

# 详细日志模式（输出 DEBUG 级别日志到 /var/log/plat-sec-patrol/）
sudo plat-sec-patrol check --verbose

# 指定插件目录（用于开发调试）
sudo plat-sec-patrol -p /path/to/plugins check --verbose
```

### 命令行参数一览

```
用法: plat-sec-patrol [OPTIONS] COMMAND

命令:
  check              检测所有安全能力并报告状态（表格格式）
  check <CAP_ID>     查看指定能力的详细状态
  enable <CAP_ID>    使能指定能力
  disable <CAP_ID>   禁用指定能力
  list               列出所有可用的能力定义

选项:
  -f, --format FORMAT    输出格式: text（默认）或 json
  -v, --verbose          启用详细输出（DEBUG 级别日志）
  -p, --plugin-dir DIR   指定插件目录
  -h, --help             显示帮助信息
  -V, --version          显示版本信息
```

## 5. 常见问题排查

### 问题 1: PkgConfig 未找到

**错误信息:** `Could NOT find PkgConfig`

**解决方案：**
```bash
sudo apt-get install pkg-config
```

### 问题 2: 插件加载失败

**检查步骤：**
```bash
# 1. 确认插件文件存在
ls -la /usr/local/lib/plat-sec-patrol/plugins/

# 2. 检查插件依赖是否满足
ldd /usr/local/lib/plat-sec-patrol/plugins/libcpu_hygon_collector.so

# 3. 确认 libplat_common.so 可被找到
ldconfig -p | grep plat_common

# 4. 使用 -p 指定插件目录调试
sudo plat-sec-patrol -p /usr/local/lib/plat-sec-patrol/plugins check --verbose
```

### 问题 3: 权限不足

`check`、`enable`、`disable` 命令需要 root 权限：

```bash
# 使用 sudo 运行
sudo plat-sec-patrol check

# 确保日志目录可写
sudo mkdir -p /var/log/plat-sec-patrol
sudo chmod 777 /var/log/plat-sec-patrol
```

### 问题 4: 海光平台才能看到能力

能力库插件 `libhygongenuine_capability.so` 仅在检测到海光 CPU（`vendor_id` 包含 `HygonGenuine`）时加载。非海光平台不会加载该插件。

```bash
# 检查 CPU 厂商
cat /proc/cpuinfo | grep vendor_id
```

如需支持其他平台，需开发对应的能力库插件（参见 PLUGIN_MATCHING.md 的开发指南）。

### 问题 5: 未构建测试

测试默认关闭，需手动开启：

```bash
cd build
cmake -DBUILD_TESTING=ON ..
make -j$(nproc)
ctest --output-on-failure
```

## 6. 开发调试

### 使用 GDB 调试

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# 启动 GDB（需要 root 权限才能运行 check）
sudo gdb ./plat-sec-patrol
(gdb) run check --verbose
(gdb) bt  # 如果崩溃，查看堆栈
```

### 内存泄漏检查

```bash
sudo valgrind --leak-check=full ./plat-sec-patrol check

sudo valgrind --leak-check=full --show-leak-kinds=all ./plat-sec-patrol check
```

### 性能分析

```bash
sudo perf record ./plat-sec-patrol check
sudo perf report
```

## 7. 扩展开发

### 添加新的环境感知插件

1. 在 `plugins/environment/` 下创建插件目录：
   ```bash
   mkdir plugins/environment/my_collector
   ```

2. 编写插件代码（参考 `plugins/environment/cpu_collector/cpu_collector.c`），确保导出：
   ```c
   const char* plugin_type = "collector";
   plugin_env_list_t* collect(const char *plugin_name) { ... }
   ```

3. 创建 `CMakeLists.txt`（参考其他插件），链接 `plat_common`：
   ```cmake
   add_library(my_collector SHARED my_collector.c)
   target_include_directories(my_collector PRIVATE
       ${CMAKE_SOURCE_DIR}/../../../include
       ${CMAKE_SOURCE_DIR}/../../../common
       ${CMAKE_SOURCE_DIR}/../../../common/include
   )
   target_link_libraries(my_collector plat_common ${CMAKE_DL_LIBS})
   ```

4. 在顶层 `CMakeLists.txt` 中添加子目录和安装规则：
   ```cmake
   add_subdirectory(plugins/environment/my_collector)
   # 在 install 行追加 my_collector 目标
   ```

5. 重新编译并安装：
   ```bash
   cd build && cmake .. && make -j$(nproc) && sudo make install
   ```

### 添加新的能力库插件

1. 在 `plugins/capability/` 下创建厂商目录：
   ```bash
   mkdir plugins/capability/<vendor>
   ```

2. 编写能力库插件（参考 `plugins/capability/hygon/hygon_capability.c`），确保导出：
   ```c
   const char* plugin_type = "capability";
   char* load_capabilities_json(const environment_t *env) { ... }
   ```
   返回值必须是调用方可释放的 JSON 字符串。

3. 准备能力定义 JSON 文件（参考 `plugins/capability/hygon/hygon_capabilities.json`）

4. 创建 `CMakeLists.txt` 并添加到顶层构建

5. 插件文件名必须包含 `capability` 和对应的 CPU vendor 字符串，否则不会被匹配加载（详见 PLUGIN_MATCHING.md）

## 8. 下一步

- 阅读 `src/core/` 下的源码了解核心业务逻辑（CLI、采集、解析、判定、执行）
- 查看 `include/` 目录中的头文件了解公共 API 和数据模型
- 查看 `common/` 目录了解公共工具（日志、cJSON、插件环境列表）
- 阅读 PLUGIN_MATCHING.md 了解插件匹配机制
- 开发自己的插件并贡献代码到项目
