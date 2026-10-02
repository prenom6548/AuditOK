# 插件平台匹配规则

## 概述

插件系统采用智能匹配机制，根据当前平台的 CPU 厂商信息，动态加载合适的插件。匹配逻辑集中在 `src/core/extension.c` 的 `should_load_plugin()` 函数中，在扫描插件目录时逐一判定每个 `.so` 文件是否应当加载。

## 匹配流程

```
1. 扫描插件目录中的所有 .so 文件（扁平目录，无子目录递归）
2. 从文件名提取关键信息（厂商、类型等）
3. 与当前环境信息对比（主要依据 CPU vendor_id）
4. 只加载匹配的插件
5. 跳过不匹配的插件
```

## 插件分类

项目包含两类插件：

| 类型 | plugin_type 值 | 导出函数 | 说明 |
|------|---------------|----------|------|
| 环境感知插件 | `"collector"` | `collect()` | 采集 CPU、固件、OS 等环境信息 |
| 能力库插件 | `"capability"` | `load_capabilities_json()` | 返回 JSON 格式的能力定义 |

每个插件必须导出全局符号 `plugin_type`（值为 `"collector"` 或 `"capability"`），否则加载失败。可选导出 `plugin_init()` 和 `plugin_fini()` 用于初始化和清理。

## 匹配规则详解

### 1. CPU 相关插件

CPU 插件的文件名必须严格遵循 `libcpu_<vendor>_collector.so` 格式。

#### 匹配逻辑（`extract_vendor_from_filename()` + `match_cpu_info()`）

1. 检查文件名是否以 `libcpu_` 开头且以 `_collector.so` 结尾
2. 提取中间的厂商字段（如 `hygon`）
3. 使用 `strcasestr()`（不区分大小写）将提取的厂商名与 `env->cpu.vendor` 进行模糊匹配

#### 当前插件

| 文件名 | 匹配平台 | vendor 字段 |
|--------|---------|------------|
| `libcpu_hygon_collector.so` | 海光 (HygonGenuine) | `hygon` |

**示例（海光平台 `HygonGenuine`）：**

```
# 会加载
libcpu_hygon_collector.so   ✓  (vendor="hygon" 匹配 "HygonGenuine"，不区分大小写)

# 不会加载（假设未来存在其他厂商的 CPU 插件）
libcpu_intel_collector.so   ✗  (vendor="intel" 不匹配 "HygonGenuine")
libcpu_amd_collector.so     ✗  (vendor="amd" 不匹配 "HygonGenuine")
```

**注意：** 如果 CPU vendor 信息未知（`env->cpu.vendor == NULL`），所有 CPU 插件均不加载。

### 2. 固件采集插件

匹配条件：文件名包含 `firmware_collector`。

**当前插件：**

| 文件名 | 匹配平台 |
|--------|---------|
| `libfirmware_collector.so` | 所有平台（无条件加载） |

### 3. OS 采集插件

匹配条件：文件名包含 `os`。

**当前插件：**

| 文件名 | 匹配平台 |
|--------|---------|
| `libos_collector.so` | 所有平台（无条件加载） |

### 4. 能力库插件

匹配条件：文件名包含 `capability` **且** 文件名中包含当前 CPU vendor 字符串（不区分大小写）。

**当前插件：**

| 文件名 | 匹配平台 | 匹配依据 |
|--------|---------|---------|
| `libhygongenuine_capability.so` | 海光 (HygonGenuine) | `"hygongenuine"` 包含 `"HygonGenuine"`（不区分大小写） |

**示例（海光平台 `HygonGenuine`）：**

```
# 会加载
libhygongenuine_capability.so   ✓  (包含 "capability" 且包含 "HygonGenuine")

# 不会加载（假设未来存在其他厂商的能力库插件）
libintel_capability.so          ✗  (包含 "capability" 但不包含 "HygonGenuine")
```

## 匹配优先级

`should_load_plugin()` 的判定顺序：

```
1. search_environment_plugin()   ← 先尝试匹配环境感知插件
   ├── 文件名含 "cpu"              → 提取 vendor 并匹配 CPU 厂商
   ├── 文件名含 "firmware_collector" → 直接加载
   └── 文件名含 "os"               → 直接加载
2. 能力库匹配                    ← 环境插件未匹配时尝试
   └── 文件名含 "capability" 且含 CPU vendor → 加载
3. 其他情况 → 跳过
```

## 插件命名规范

### 推荐命名格式

```
lib<platform>_<feature>.so
```

**当前项目中的实际命名：**

| 文件名 | 类型 | 说明 |
|--------|------|------|
| `libcpu_hygon_collector.so` | collector | 海光 CPU 信息采集 |
| `libfirmware_collector.so` | collector | 固件信息采集 |
| `libos_collector.so` | collector | 操作系统信息采集 |
| `libhygongenuine_capability.so` | capability | 海光平台安全能力库 |

### 命名约束

- **CPU 插件**：必须严格使用 `libcpu_<vendor>_collector.so` 格式，否则 `extract_vendor_from_filename()` 无法解析
- **固件/OS 插件**：文件名中需包含 `firmware_collector` 或 `os` 关键字
- **能力库插件**：文件名中需包含 `capability` 关键字和对应的 CPU vendor 字符串

## 公共依赖库

所有插件均链接 `libplat_common.so` 共享库（由顶层 CMakeLists.txt 构建），提供日志、字符串工具、文件工具、cJSON 和插件环境列表等公共功能。

插件通过 `INSTALL_RPATH` 设置运行时库搜索路径（`$ORIGIN/../../lib`），确保安装后能找到 `libplat_common.so`。

## 日志输出示例

### 成功匹配（海光平台）

```log
[INFO] Found 4 potential plugins in /usr/local/lib/plat-sec-patrol/plugins
[DEBUG] Checking plugin: libcpu_hygon_collector.so
[DEBUG] Plugin info extracted: vendor='hygon'
[DEBUG] Loading matched plugin: /usr/local/lib/plat-sec-patrol/plugins/libcpu_hygon_collector.so
[INFO] Loaded plugin: /usr/local/lib/plat-sec-patrol/plugins/libcpu_hygon_collector.so (type=collector)
[DEBUG] Checking plugin: libfirmware_collector.so
[DEBUG] Loading matched plugin: /usr/local/lib/plat-sec-patrol/plugins/libfirmware_collector.so
[INFO] Loaded plugin: /usr/local/lib/plat-sec-patrol/plugins/libfirmware_collector.so (type=collector)
[DEBUG] Checking plugin: libos_collector.so
[DEBUG] Loading matched plugin: /usr/local/lib/plat-sec-patrol/plugins/libos_collector.so
[INFO] Loaded plugin: /usr/local/lib/plat-sec-patrol/plugins/libos_collector.so (type=collector)
[DEBUG] Checking plugin: libhygongenuine_capability.so
[DEBUG] Loading matched plugin: /usr/local/lib/plat-sec-patrol/plugins/libhygongenuine_capability.so
[INFO] Loaded plugin: /usr/local/lib/plat-sec-patrol/plugins/libhygongenuine_capability.so (type=capability)
[INFO] Plugin loading completed: 4 loaded, 0 skipped
```

### 跳过不匹配插件（假设存在 Intel 插件）

```log
[DEBUG] Checking plugin: libcpu_intel_collector.so
[DEBUG] Plugin info extracted: vendor='intel'
[DEBUG] No matching CPU plugin found for: Hygon C86-4G (OPN:7490)
[DEBUG] Skipping plugin (platform mismatch): /usr/local/lib/plat-sec-patrol/plugins/libcpu_intel_collector.so
```

## 开发指南

### 为新厂商开发 CPU 采集插件

1. **确定平台标识**
   ```bash
   cat /proc/cpuinfo | grep vendor_id
   ```

2. **命名插件文件**（必须遵循格式）
   ```
   libcpu_<vendor小写>_collector.so
   ```

3. **在插件 CMakeLists.txt 中设置目标名**
   ```cmake
   add_library(cpu_<vendor小写>_collector SHARED cpu_collector.c)
   ```

4. **在插件源码中导出必要符号**
   ```c
   const char* plugin_type = "collector";

   plugin_env_list_t* collect(const char *plugin_name)
   {
       // 采集环境信息...
   }

   int plugin_init(void) { return 0; }    // 可选
   void plugin_fini(void) {}              // 可选
   ```

### 为新厂商开发能力库插件

1. **命名插件文件**，确保包含 `capability` 和 vendor 字符串
   ```
   lib<vendor>_capability.so
   ```

2. **导出必要符号**
   ```c
   const char* plugin_type = "capability";

   char* load_capabilities_json(const environment_t *env)
   {
       // 返回 JSON 字符串（调用方负责释放）
   }
   ```

3. **准备能力定义 JSON 文件**，按 CPU 型号分组定义安全能力

### 测试插件匹配

```bash
# 查看哪些插件会被加载（需要 root 权限）
sudo ./plat-sec-patrol check --verbose 2>&1 | grep -E "(Loading|Skipping|Loaded|Checking)"

# 查看当前加载的能力
sudo ./plat-sec-patrol list --verbose

# 指定插件目录测试
sudo ./plat-sec-patrol -p /path/to/plugins check --verbose
```

## 故障排查

### 问题：插件未被加载

**检查步骤：**

1. 确认插件文件名符合命名规范（特别是 CPU 插件必须为 `libcpu_<vendor>_collector.so` 格式）
2. 运行详细日志查看匹配过程
   ```bash
   sudo ./plat-sec-patrol check --verbose
   ```
3. 检查 CPU 厂商识别是否正确
   ```bash
   cat /proc/cpuinfo | grep vendor_id
   ```
4. 确认插件文件存在于插件目录中
   ```bash
   ls -la /usr/local/lib/plat-sec-patrol/plugins/
   ```
5. 确认 `libplat_common.so` 可被插件找到
   ```bash
   ldd /usr/local/lib/plat-sec-patrol/plugins/libcpu_hygon_collector.so
   ```

### 问题：插件加载但返回空数据

**检查步骤：**

1. 确认插件导出了 `plugin_type` 符号且值正确
   ```bash
   nm -D libcpu_hygon_collector.so | grep plugin_type
   ```
2. 确认 collector 插件导出了 `collect` 函数
   ```bash
   nm -D libcpu_hygon_collector.so | grep collect
   ```
3. 确认 capability 插件导出了 `load_capabilities_json` 函数
   ```bash
   nm -D libhygongenuine_capability.so | grep load_capabilities_json
   ```

## 未来扩展

计划支持的匹配维度：

- [ ] 根据操作系统名称和版本筛选 OS 插件（当前 `search_environment_plugin()` 中的 TODO）
- [ ] 固件版本匹配
- [ ] 内核版本匹配
- [ ] 特定硬件特性匹配（如 IOMMU、TPM）
- [ ] 插件依赖关系自动解析
- [ ] 插件签名验证和安全加载
