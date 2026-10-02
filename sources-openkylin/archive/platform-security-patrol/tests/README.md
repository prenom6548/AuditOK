# 单元测试指南

## 📋 概述

本项目使用 **CUnit** 框架进行 C 语言单元测试。测试覆盖核心模块和工具函数。

**当前测试统计**：**104 个测试用例**，覆盖 5 个模块


| 模块       | 文件                       | 用例数 | 主要测试点                                                                  |
| ---------- | -------------------------- | ------ | --------------------------------------------------------------------------- |
| 字符串工具 | `test_string_utils.c`      | 36     | trim、concat、starts_with、contains、split、strdup、atoi、snprintf、strncpy |
| 文件工具   | `test_file_utils.c`        | 29     | 读写、存在性、目录操作、扫描目录、文件大小                                  |
| 能力解析器 | `test_capability_parser.c` | 16     | JSON 解析、条件组、依赖管理、条件类型识别                                   |
| 判定模块   | `test_judge.c`             | 14     | 多层判定、版本比较（≥/=/< 通过公共接口）、状态转换、环境模拟               |
| 扩展模块   | `test_extension.c`         | 9      | 插件管理器生命周期、插件加载/卸载边界条件                                   |

## 📁 测试文件结构

```
tests/
├── CMakeLists.txt              # 测试构建配置（CUnit 集成）
├── run_tests.sh                # Linux/macOS 测试运行脚本
├── README.md                   # 本文件
├── test_string_utils.c         # 字符串工具函数测试 (36 个用例)
├── test_file_utils.c           # 文件工具函数测试 (29 个用例)
├── test_capability_parser.c    # 能力解析器测试 (16 个用例)
├── test_judge.c                # 判定模块测试 (14 个用例)
└── test_extension.c            # 扩展模块测试 (9 个用例)
```

## 🚀 快速开始

### 第一步：安装依赖

**Ubuntu/Debian/OpenKylin:**

```bash
sudo apt-get install libcunit1 libcunit1-dev
```

**CentOS/RHEL:**

```bash
sudo yum install CUnit-devel
```

### 第二步：运行测试

#### 方式一：使用脚本（推荐）

```bash
cd tests/
./run_tests.sh
```
**脚本功能**：

1. 自动定位项目根目录
2. 创建 build 目录
3. 配置项目（启用测试）
4. 编译项目和测试
5. 运行所有测试
6. 显示测试结果摘要

#### 方式二：手动构建

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
make
ctest --verbose
```
### 第三步：查看结果

测试完成后会显示：

- ✅ 通过的测试用例
- ❌ 失败的测试用例及原因
- 📊 总体通过率统计

## 🛠️ 构建和运行测试

### 1. 配置构建（启用测试）

```bash
cd build
cmake .. -DBUILD_TESTING=ON
make
```
### 2. 运行所有测试

```bash
ctest --verbose
```
或者单独运行某个测试：

```bash
./tests/test_string_utils
./tests/test_file_utils
./tests/test_capability_parser
./tests/test_judge
./tests/test_extension
```
### 3. 查看详细输出

```bash
ctest --verbose --output-on-failure
```
## 📊 测试覆盖范围

### 已完成测试

- ✅ **字符串工具函数** (`test_string_utils.c`，36 个用例)

  - `string_utils_trim()` — 去除空白（6 个用例：基本、无空白、纯空白、空串、NULL、tab/换行）
  - `string_utils_concat()` — 字符串连接（3 个用例）
  - `string_utils_starts_with()` — 前缀检查（5 个用例）
  - `string_utils_contains()` — 子串检查（5 个用例）
  - `string_utils_split()` — 字符串分割（4 个用例）
  - `string_utils_strdup()` — 字符串复制（3 个用例）
  - `string_utils_atoi()` — 字符串转整数（5 个用例）
  - `string_utils_snprintf()` — 格式化字符串（2 个用例）
  - `string_utils_strncpy()` — 安全字符串拷贝（3 个用例）
- ✅ **文件工具函数** (`test_file_utils.c`，29 个用例)

  - `file_utils_file_exists()` — 文件存在性检查（3 个用例）
  - `file_utils_is_directory()` — 目录判断（4 个用例）
  - `file_utils_is_regular_file()` — 普通文件判断（3 个用例）
  - `file_utils_read_file()` — 读取文件（3 个用例）
  - `file_utils_write_file()` — 写入文件（3 个用例）
  - `file_utils_get_file_size()` — 获取文件大小（3 个用例）
  - `file_utils_create_directory()` — 创建目录（3 个用例，含嵌套目录）
  - `file_utils_scan_directory()` — 扫描目录（4 个用例，含扩展名过滤）
- ✅ **能力解析器** (`test_capability_parser.c`，16 个用例)

  - 能力列表创建和管理
  - 条件组创建（AND/OR）
  - 条件创建和验证（单值、多值）
  - 层规则初始化
  - JSON 配置文件解析（有效文件、不存在文件、无效 JSON）
  - 条件类型识别（firmware、kernel_config、cpu_model）
- ✅ **判定模块** (`test_judge.c`，14 个用例)
  - 判定结果结构创建
  - 全支持判定（CPU + Firmware + OS 三层）
  - CPU 不支持判定
  - Firmware 禁用判定
  - OS 配置缺失判定
  - 指定能力查找（存在/不存在）
  - 版本比较（通过 `judge_firmware_layer()` / `judge_os_layer()` 公共接口间接覆盖）：
    - 固件版本不满足 `>=` 条件（`"1.1.0"` < 要求 `"1.2.0"`）→ UNSUPPORTED
    - 固件版本恰好等于要求值（`"1.2.0"` == 要求 `"1.2.0"`）→ SUPPORTED
    - OS 内核版本满足 `>=`（`"5.10.0"` > 要求 `"5.4.0"`）→ SUPPORTED
    - OS 内核版本不满足 `>=`（`"5.10.0"` < 要求 `"5.15.0"`）→ UNSUPPORTED
  - 状态转换函数（`cap_status_to_string`、`layer_status_to_string`）

- ✅ **扩展模块** (`test_extension.c`，9 个用例)
  - `extension_create_manager()` — 有效目录、NULL 目录、空目录
  - `extension_free_manager()` — NULL 安全、正常释放
  - `extension_scan_plugins()` — NULL 目录、不存在目录
  - `extension_load_plugins()` — NULL 管理器、不存在目录
  - `extension_unload_plugins()` — NULL 管理器、空管理器
  - `extension_invoke_collectors()` — NULL 管理器、NULL 输出、空管理器
  - `extension_load_capabilities()` — NULL 管理器、空管理器
  - `extension_get_plugin_type()` — NULL 安全

### 待添加测试

- ⏳ Executor 模块完整测试（使能/禁用操作流程、前置校验、配置备份恢复）
- ⏳ Base Collector 模块（环境采集）
- ⏳ CLI 参数解析
- ⏳ `cmd_runner` 工具函数
- ⏳ 能力解析器 `disable_info` 字段解析、多 CPU 型号匹配
- ⏳ 判定模块 OR 条件组（firmware alternatives）测试

## 📝 编写新测试

### 基本模板

```c
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

// 测试函数
static void test_example(void)
{
    CU_ASSERT_TRUE(1 == 1);
    CU_ASSERT_STRING_EQUAL("hello", "hello");
    CU_ASSERT_PTR_NOT_NULL(some_pointer);
}

// 主函数
int main(void)
{
    CU_pSuite pSuite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    pSuite = CU_add_suite("Example_Test", NULL, NULL);
    if (!pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_add_test(pSuite, "test_example", test_example);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();
    return CU_get_error();
}
```
### 常用断言宏


| 宏                                  | 用途       |
| ----------------------------------- | ---------- |
| `CU_ASSERT(condition)`              | 通用断言   |
| `CU_ASSERT_TRUE(value)`             | 断言为真   |
| `CU_ASSERT_FALSE(value)`            | 断言为假   |
| `CU_ASSERT_EQUAL(actual, expected)` | 相等断言   |
| `CU_ASSERT_STRING_EQUAL(s1, s2)`    | 字符串相等 |
| `CU_ASSERT_PTR_NULL(ptr)`           | 指针为空   |
| `CU_ASSERT_PTR_NOT_NULL(ptr)`       | 指针非空   |
| `CU_FAIL(message)`                  | 强制失败   |

### 扩展步骤

如需添加新测试：

1. **创建测试文件**: `tests/test_<module>.c`
2. **编写测试函数**: 使用 CUnit 断言宏
3. **注册测试**: 在 `main()` 中添加 `CU_add_test()`
4. **更新 tests/CMakeLists.txt**: 添加新的 `add_executable()` 和 `add_test()`
5. **更新本文档**: 在"测试覆盖范围"部分记录新测试

## 🔍 调试测试

### 启用详细输出

```bash
ctest --verbose --output-on-failure
```
### 使用 GDB 调试

```bash
gdb ./tests/test_extension
(gdb) run
(gdb) bt  # 如果崩溃，查看回溯
```
## ⚙️ 持续集成

在 CI/CD 流程中添加测试步骤：

```yaml
test:
  script:
    - mkdir -p build && cd build
    - cmake .. -DBUILD_TESTING=ON
    - make
    - ctest --output-on-failure
```
## 💡 最佳实践

1. **独立性**: 每个测试用例应该独立，不依赖其他测试的执行顺序
2. **清理资源**: 测试完成后清理临时文件和分配的内存
3. **边界条件**: 测试正常情况、边界情况和错误情况
4. **命名规范**: 测试函数命名为 `test_<function>_<scenario>`
5. **注释清晰**: 每个测试用例说明其测试目的

## ⚠️ 注意事项

1. 测试需要安装 CUnit 开发包
2. 必须使用 `-DBUILD_TESTING=ON` 启用测试
3. 测试会创建临时文件，会自动清理
4. 某些文件操作可能需要写权限

## ❓ 常见问题

### Q: 找不到 CUnit 头文件？

**A**: 确保已安装开发包（libcunit1-dev 或 CUnit-devel），并检查 CMake 输出中是否找到 CUnit。

### Q: 测试链接失败？

**A**: 检查 `tests/CMakeLists.txt` 中的库链接是否正确包含 `${CUNIT_LIBRARIES}`。

### Q: 如何只运行某个测试？

**A**: 直接执行对应的可执行文件，如 `./tests/test_string_utils`

### Q: 测试运行时崩溃？

**A**: 使用 gdb 调试，检查是否有空指针解引用或内存访问越界。

### Q: 测试失败如何调试？

**A**: 使用 `ctest --verbose --output-on-failure` 查看详细输出，或用 gdb 调试。

## 🤝 贡献指南

欢迎贡献新的测试用例！请遵循以下原则：

✅ 每个测试独立，不依赖执行顺序
✅ 测试完成后清理资源（内存、文件）
✅ 覆盖正常、边界、错误三种情况
✅ 使用清晰的命名和注释
✅ 更新文档中的测试覆盖范围

添加新测试时：

1. 在对应的测试文件中添加测试函数
2. 在 `main()` 函数中注册测试用例
3. 更新本文档的"测试覆盖范围"部分
4. 确保所有测试通过后再提交
