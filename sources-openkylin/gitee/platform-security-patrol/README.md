# Platform Security Patrol Tool

平台安全能力检测与使能工具 - 用于检测和使能国产 CPU（海光等）及操作系统内置的硬件级安全能力。

## 功能特性

- **统一能力建模**: 将不同层级的安全能力抽象为统一的逻辑模型
- **智能决策引擎**: 基于推理判断当前安全状态是否合规
- **自动检测与评估**: 自动扫描并评估系统中各项安全能力的启用状态
- **插件化架构**: 支持动态扩展，适配不同硬件、固件和操作系统环境
- **分层判定**: CPU 层、固件层、OS 层三层判定机制

## 技术栈

- **语言**: C11
- **构建系统**: CMake &gt;= 3.10
- **依赖库**:
  - libdl (动态库加载)
  - pthread (多线程)
  - libm (数学库)
  - cJSON (内置于 `common/cJSON.c`，MIT 许可)
  - 标准 C 库

## 目录结构

```
plat-sec-patrol/
├── CMakeLists.txt           # 主构建文件
├── README.md                # 本文件
├── include/                 # 公共头文件
│   ├── plat_sec_patrol.h    # 主头文件（版本号、错误码、全局类型）
│   ├── core/                # 核心模块内部头文件
│   │   ├── cli.h
│   │   ├── base_collector.h
│   │   ├── capability_parser.h
│   │   ├── judge.h
│   │   ├── executor.h
│   │   └── extension.h
│   ├── models/              # 数据模型定义
│   │   ├── environment.h
│   │   ├── capability.h
│   │   └── result.h
│   ├── plugins/             # 插件接口头文件
│   │   ├── collector_plugin.h
│   │   └── capability_plugin.h
│   └── utils/               # 工具函数头文件
│       ├── string_utils.h
│       ├── file_utils.h
│       ├── cmd_runner.h
│       └── system_paths.h   # 系统路径和常量定义
├── src/                     # 源代码
│   ├── main.c               # 程序入口
│   ├── core/                # 核心模块实现
│   │   ├── cli.c            # CLI 解析与命令分发
│   │   ├── base_collector.c # 基础信息采集
│   │   ├── capability_parser.c # 能力库 JSON 解析
│   │   ├── judge.c          # 能力状态判定引擎
│   │   ├── executor.c       # 能力使能/禁用执行器
│   │   └── extension.c      # 插件加载与管理
│   ├── models/              # 数据模型实现
│   │   ├── environment.c
│   │   ├── capability.c
│   │   └── result.c
│   └── utils/               # 工具函数
│       ├── string_utils.c
│       ├── file_utils.c
│       └── cmd_runner.c     # 外部命令执行（fork/exec）
├── common/                  # 公共库（内置第三方和共享工具）
│   ├── cJSON.c              # cJSON 库（内置，MIT 许可）
│   ├── logger.c             # 日志模块实现
│   ├── plugin_env_utils.c   # 插件环境信息工具函数
│   └── include/
│       ├── cJSON.h
│       ├── logger.h
│       └── plugin_env_utils.h
├── plugins/                 # 插件源码
│   ├── environment/         # 环境感知插件
│   │   ├── cpu_collector/   # CPU 信息采集（/proc/cpuinfo）
│   │   ├── firmware_collector/ # 固件信息采集（DMI/hag）
│   │   └── os_collector/    # OS 信息采集（内核配置/驱动/软件包）
│   └── capability/          # 能力库插件
│       └── hygon/           # 海光平台能力定义（JSON）
├── tests/                   # 单元测试（CUnit）
├── scripts/                 # 构建与安装脚本
│   ├── build-deb.sh         # DEB 包构建
│   ├── clean-debian.sh      # Debian 构建产物清理
│   └── hygon/
│       └── install_hag.sh   # 海光 HAG 工具安装
└── debian/                  # Debian 打包配置
    ├── control
    ├── changelog
    ├── copyright
    ├── rules
    ├── postinst
    ├── postrm
    ├── prerm
    └── compat
```

## 构建说明

### 安装依赖

```bash
sudo apt-get install build-essential cmake pkg-config
```

### 编译项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake ..

# 编译
make -j$(nproc)

# 安装（默认安装到 /usr/local）
sudo make install
```

### 清理构建

```bash
cd build
make clean
# 或完全清除
cd .. && rm -rf build
```

## 打包分发

### DEB 打包

项目使用 `debian/` 目录提供标准的 Debian 打包配置。

#### 构建 DEB 包

```bash
chmod +x scripts/build-deb.sh
./scripts/build-deb.sh
```

生成的包位于 `packages/` 目录：

```
packages/platform-security-patrol_0.1.0_amd64.deb
```

#### 安装 DEB 包

```bash
sudo dpkg -i packages/platform-security-patrol_0.1.0_amd64.deb
# 或
sudo apt install ./packages/platform-security-patrol_0.1.0_amd64.deb
```

## 前置条件

### 海光平台需要安装 HAG 工具

海光平台的固件层能力检测依赖海光 **HAG (Hygon Authentication Gateway)** 命令行工具。执行 `check` 检测时，`firmware_collector` 插件会调用 `hag general check`，并从其输出中提取以下三项环境信息：

| 环境信息 | hag 输出中的来源字段 | 用途 |
| --- | --- | --- |
| `firmware_version` | `firmware version:` | 判定安全固件版本是否满足能力要求（如 3G ≥ 1837、4G ≥ 2089） |
| `tkm_status` | `TKM:` | 判定 TKM 是否已使能 |
| `tdm_status` | `TDM:` | 判定 TDM 是否已使能 |

这三项信息**仅由 HAG 提供**，系统内没有其他备用采集来源。

未安装 HAG 时，工具不会报错退出，`firmware_collector` 插件仅打印警告日志并继续检测：

```
hag command not found, skipping HAG info collection
```

`check` 命令检测到 HAG（或能力库声明的其他外部工具）缺失时，会在检测结果下方输出警告与安装指导：

```
WARNING: 平台能力库声明的外部工具缺失，相关能力检测结果可能不准确:
  [hag]
    安装方法: sudo wget -c https://download.sourcefind.cn:65024/directlink/8/c86-devkit/previous-release/bin/hag -O /usr/local/bin/hag && sudo chmod +x /usr/local/bin/hag
    安装验证: sudo hag general check
```

> 说明：`--format json` 模式下，检测结果输出到 stdout（纯净 JSON），警告块输出到 stderr，不影响管道或重定向方式的自动化消费。

#### 安装 HAG

**方式一：使用源码目录中的安装脚本**

```bash
# 从海光开发者社区（sourcefind）下载并安装 HAG
# 将安装到 /usr/bin/hag，需要网络访问和 wget
sudo bash scripts/hygon/install_hag.sh
```

**方式二：wget 直接安装**（适用于通过 DEB 包安装、无安装脚本的环境）

```bash
sudo wget -c https://download.sourcefind.cn:65024/directlink/8/c86-devkit/previous-release/bin/hag -O /usr/local/bin/hag && sudo chmod +x /usr/local/bin/hag
```

安装完成后，验证 HAG 是否可用：

```bash
# 确认 hag 位于 PATH 中（采集器通过 PATH 查找 hag 命令）
which hag

# 运行 general check，输出应包含 firmware version: / TKM: / TDM: 等字段
sudo hag general check
```

若能正常输出上述字段，说明 HAG 工作正常，检测结果可信。

## 使用方法

### 基本命令

`check`、`enable`、`disable` 命令需要 root 权限。

```bash
# 查看所有可用命令
plat-sec-patrol --help

# 查看版本
plat-sec-patrol --version

# 检测所有安全能力（默认文本表格格式，需要 root）
sudo plat-sec-patrol check

# 检测指定能力
sudo plat-sec-patrol check csv1

# 以 JSON 格式输出检测结果
sudo plat-sec-patrol check --format json

# 启用指定能力
sudo plat-sec-patrol enable tcm2.0

# 禁用指定能力
sudo plat-sec-patrol disable csv1

# 列出所有可用能力
plat-sec-patrol list

# 详细输出（启用 DEBUG 级别日志）
sudo plat-sec-patrol check --verbose
```

### 命令行选项

- `-f, --format FORMAT`: 输出格式（`text` 或 `json`；默认 `text`）
- `-v, --verbose`: 启用详细输出（DEBUG 级别日志）
- `-p, --plugin-dir DIR`: 指定插件目录（覆盖默认路径）
- `-h, --help`: 显示帮助信息
- `-V, --version`: 显示版本信息

### 子命令

| 命令 | 说明 | 需要参数 | 需要 root |
| --- | --- | --- | --- |
| `check [CAP_ID]` | 检测安全能力状态，可选指定单个能力 ID | 可选 | 是 |
| `enable <CAP_ID>` | 使能指定安全能力 | 必须 | 是 |
| `disable <CAP_ID>` | 禁用指定安全能力 | 必须 | 是 |
| `list` | 列出所有可用安全能力 | 无 | 否 |

### 输出示例

#### 表格格式输出（默认）

```
Name                    Overall Status     CPU Layer    Firmware Layer   OS Layer     Description
---------------------------------------------------------------------------------------------------------
csv1                    available          supported    supported        available    N/A
tpm2.0                  disabled           supported    supported        unavailable  Kernel config not enabled

Total: 2 capabilities
  Available: 1, Disabled: 1, Unsupported: 0, Unavailable: 0, Unknown: 0
```

#### JSON 格式输出

```json
[
    {
        "capability_id": "csv1",
        "overall_status": "available",
        "cpu_status": "supported",
        "firmware_status": "supported",
        "os_status": "available"
    },
    {
        "capability_id": "tpm2.0",
        "overall_status": "disabled",
        "cpu_status": "supported",
        "firmware_status": "supported",
        "os_status": "unavailable",
        "evidence": "CONFIG_TCG_TPM=n"
    }
]
```

#### 指定单个能力输出

使用 `check <CAP_ID>` 查看某个能力的详细分层状态：

```
=== Capability csv1 Status Report ===
Capability: csv1
Status:     available
  -- CPU Layer:      supported
  -- Firmware Layer: supported
  -- OS Layer:       available
```

## 插件开发

### 环境感知插件

环境感知插件需要导出以下符号：

```c
#include "plugins/collector_plugin.h"

/* 必须：插件类型标识 */
const char* plugin_type = "collector";

/* 必须：采集函数，返回键值对链表 */
plugin_env_list_t* collect(const char *plugin_name) {
    plugin_env_list_t *env = plugin_env_list_create();
    plugin_env_list_add(env, "vendor", "HygonGenuine");
    plugin_env_list_add(env, "model", "Hygon C86-3G");
    return env;
}

/* 可选：初始化 */
int plugin_init(void) {
    return 0;  /* 成功返回 0 */
}

/* 可选：清理 */
void plugin_fini(void) {
}
```

### 能力库插件

能力库插件需要导出以下符号：

```c
#include "plugins/capability_plugin.h"

/* 必须：插件类型标识 */
const char* plugin_type = "capability";

/* 必须：返回能力定义 JSON 字符串，由核心层负责解析 */
const char* load_capabilities_json(const environment_t *env) {
    /* 根据 env->cpu.vendor 判断是否匹配当前平台 */
    /* 从文件读取或直接构造 JSON 字符串 */
    return json_string;
}

/* 可选：清理 */
void plugin_fini(void) {
}
```

### 编译插件

每个插件是一个独立的共享库（.so 文件），需链接 `libplat_common.so`：

```bash
gcc -shared -fPIC -o my_plugin.so my_plugin.c \
    -I/usr/local/include/plat-sec-patrol -L/usr/local/lib -lplat_common
```

插件命名和匹配规则详见 [PLUGIN_MATCHING.md](PLUGIN_MATCHING.md)。

## 日志

日志文件位于 `/var/log/plat-sec-patrol/` 目录下，按时间戳命名：

```
/var/log/plat-sec-patrol/log-20240317-143022.txt
```

日志输出采用三级降级策略：文件日志 → syslog → stderr。

日志级别：

- ERROR: 错误信息
- WARN: 警告信息
- INFO: 一般信息（默认级别）
- DEBUG: 调试信息（需使用 `--verbose` 启用）

## 单元测试

项目使用 **CUnit** 框架进行单元测试，覆盖 5 个模块共 104 个用例。详见 tests/README.md。

```bash
cd build

# 开启测试构建（默认关闭）
cmake -DBUILD_TESTING=ON ..
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

> **注意：** 测试构建需要安装 CUnit 框架（`sudo apt-get install libcunit1-dev`）。

## 支持的硬件平台

当前支持：

- 海光 CPU 平台（HygonGenuine）
  - **C86-3G**: CSV1、CSV2、TPM 2.0、TCM 2.0、TKM、TDM
  - **C86-4G**: CSV1、CSV2、CSV3、TPM 2.0、TCM 2.0、TKM、TDM

能力定义文件：`plugins/capability/hygon/hygon_capabilities.json`

## 安装路径

安装后的文件布局：

| 文件 | 源码安装 (`make install`) | DEB 包安装 |
| --- | --- | --- |
| 主程序 | `/usr/local/bin/plat-sec-patrol` | `/usr/bin/plat-sec-patrol` |
| 插件目录 | `/usr/local/lib/plat-sec-patrol/plugins/` | `/usr/lib/plat-sec-patrol/plugins/` |
| 公共共享库 | `/usr/local/lib/libplat_common.so` | `/usr/lib/libplat_common.so` |
| 能力定义 JSON | `/usr/local/share/plat-sec-patrol/capabilities/` | `/usr/share/plat-sec-patrol/capabilities/` |
| 头文件 | `/usr/local/include/plat-sec-patrol/` | \- |
| 日志目录 | `/var/log/plat-sec-patrol/` | `/var/log/plat-sec-patrol/` |

## 故障排除

### 无法加载插件

确保插件目录存在且包含有效的 .so 文件：

```bash
# 源码安装路径
ls -la /usr/local/lib/plat-sec-patrol/plugins/

# DEB 包安装路径
ls -la /usr/lib/plat-sec-patrol/plugins/

# 或使用 -p 参数手动指定插件目录
sudo plat-sec-patrol check -p /path/to/plugins
```

### 权限不足

`check`、`enable`、`disable` 命令需要 root 权限：

```bash
sudo plat-sec-patrol enable tcm2.0
```

### 共享库找不到

```bash
# 更新动态链接库缓存
sudo ldconfig

# 验证 libplat_common.so 是否可被找到
ldconfig -p | grep plat_common
```

### 查看日志

```bash
# 查看最新日志
tail -f /var/log/plat-sec-patrol/log-*.txt

# 或使用 journalctl（如果使用 syslog 降级输出）
journalctl -t plat-sec-patrol
```

## 开发者信息

### 代码风格

- 遵循 C11 标准
- 使用 4 空格缩进
- 所有函数必须有注释说明
- 头文件必须使用 include guard

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 联系方式

- 项目主页：<https://gitee.com/openkylin/platform-security-patrol.git>