# Peony AFL模糊测试框架

基于同步API的隔离层方案（方案1）

## 📁 目录结构

```
afl_fuzzing/
├── targets/           # 测试目标源码
│   ├── PeonyTestHarness.h       # 隔离层头文件
│   ├── PeonyTestHarness.cpp     # 隔离层实现
│   └── peony_sync_test.cpp      # AFL测试入口
├── scripts/           # 构建和测试脚本
│   ├── build.sh       # 构建脚本
│   └── test.sh        # 测试脚本
├── corpus/            # 测试语料库（种子输入）
└── results/           # AFL运行结果
```

## 🎯 设计原理

**隔离层方案**：
- 仅使用Peony中的**同步API**（带`Sync`后缀）
- 无需Qt事件循环支持
- 无需GLib事件循环支持
- 稳定可靠，适合持久化模式

**覆盖的同步API**：
- `FileInfo::fromUri()` - 从URI创建文件信息
- `FileInfo::fromPath()` - 从路径创建文件信息
- `FileInfoJob::querySync()` - 同步查询文件信息
- `FileEnumerator::enumerateSync()` - 同步枚举目录

**预期覆盖率**：约58%的项目代码（所有同步相关代码）

## 🚀 使用方法

### 1. 构建测试目标

```bash
cd afl_fuzzing
./scripts/build.sh
```

或

```bash
cd afl_fuzzing/scripts
./build.sh
```

### 2. 运行模糊测试

```bash
# 运行60分钟的模糊测试
./scripts/test.sh 60
```

或

```bash
# 运行指定时间（分钟）
./scripts/test.sh <duration>
```

### 3. 查看结果

AFL的运行结果保存在 `results/` 目录：
- `crashes/` - 导致崩溃的输入
- `hangs/` - 导致超时的输入
- `queue/` - 所有生成的测试用例
- `plot_data` - 覆盖率随时间的变化

## 📊 测试内容

`PeonyTestHarness` 包装的测试函数：

1. **testFileInfoSync()** - 测试FileInfo的同步创建和属性访问
2. **testFileInfoJobSync()** - 测试FileInfoJob的同步查询
3. **testFileEnumeratorSync()** - 测试FileEnumerator的同步目录枚举

## 🔧 配置

### 修改AFL参数

编辑 `scripts/test.sh`：
```bash
afl-fuzz -i ... -o ... \
         -t 1000      # 单个输入的超时时间（毫秒）
         -V "$DURATION" \  # 模糊测试总时长（分钟）
         ...
```

### 添加新的语料库

将新的种子输入文件添加到 `corpus/` 目录，然后重新运行测试。

## 📝 输入格式

AFL使用文本输入，每个测试用例是一个文件，内容为：
- 文件URI或路径（如 `file:///home/user/test.txt`）
- 目录URI或路径（如 `file:///home/user/`)

## ⚠️ 注意事项

1. 确保Peony库已编译：`libpeony.so` 应该存在于 `libpeony-qt/` 目录
2. 需要安装 `afl++` 和 `Qt5` 开发库
3. 建议在虚拟机或测试环境中运行，避免影响实际系统
4. 大型目录的枚举可能需要较长时间，AFL的超时设置需要相应调整


## 当前版本实现的测试模块
● ✅ 测试模块：                                                                                                                         
  
  1. FileInfo - 文件信息查询                                                                                                            
     - FileInfo::fromUri()                                                                                                            
     - FileInfo::fromPath()
     - 属性访问测试

  2. FileInfoJob - 同步文件信息查询
     - querySync()

  3. FileEnumerator - 目录枚举
     - enumerateSync()
     - 获取子文件列表

  覆盖率约 34%，主要是Peony中的同步API部分（不含异步/事件循环相关代码）。


## 快捷查看结果
    grep "bitmap_cvg\|edges_found\|saved_crashes" results/default/fuzzer_stats
