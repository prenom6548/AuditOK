# 麒麟刻录器单元测试说明文档

## 项目概述

本文档介绍了麒麟刻录器项目的单元测试框架结构和使用方法。该测试框架基于Qt Test框架构建，用于对项目的核心模块进行白盒测试。

## 测试框架结构

```
tests/
├── CMakeLists.txt                # 主CMake配置文件
├── test_common/                  # 通用测试工具和辅助函数
│   ├── CMakeLists.txt
│   ├── test_helpers.h/cpp        # 测试辅助工具类
├── test_module_common/           # Common模块测试
│   ├── CMakeLists.txt
│   ├── utils_test.cpp           # Utils类测试
│   ├── dbcaller_test.cpp        # DBCaller类测试
│   └── trackingmanager_test.cpp # TrackingManager类测试
├── test_module_data/             # Data模块测试
│   ├── CMakeLists.txt
│   ├── datacdrom_test.cpp       # DataCDROM类测试
│   ├── datadirectory_test.cpp   # DataDirectory类测试
│   └── datafile_test.cpp        # DataFile类测试
├── test_module_model/            # Model模块测试
│   ├── CMakeLists.txt
│   ├── logicmain_test.cpp       # LogicMain类测试
│   ├── logicburndata_test.cpp   # LogicBurnData类测试
│   ├── logicburnimage_test.cpp  # LogicBurnImage类测试
│   └── logiccopydisc_test.cpp   # LogicCopyDisc类测试
├── test_module_frame/            # Frame模块测试
│   ├── CMakeLists.txt
│   ├── configuration_test.cpp   # Configuration类测试
│   ├── logger_test.cpp          # Logger类测试
│   └── signals_test.cpp         # Signals类测试
├── runUnitTest.sh               # 测试运行脚本
└── auto_test.sh                 # 自动化测试脚本
```

## 模块测试说明

### Common模块测试
包含对通用工具类的测试：
- Utils类：系统信息获取工具
- DBCaller类：DBus调用相关功能
- TrackingManager类：用户行为跟踪管理

### Data模块测试
包含对数据管理层的测试：
- DataCDROM类：光盘数据管理核心类
- DataDirectory类：目录数据管理
- DataFile类：文件数据管理

### Model模块测试
包含对业务逻辑层的测试：
- LogicMain类：主逻辑控制类
- LogicBurnData类：数据刻录逻辑
- LogicBurnImage类：镜像刻录逻辑
- LogicCopyDisc类：光盘复制逻辑

### Frame模块测试
包含对框架层的测试：
- Configuration类：配置管理
- Logger类：日志记录
- Signals类：信号处理

## 编译和运行测试

### 编译测试
```bash
cd build
cmake ..
make
```

### 运行测试
```bash
cd tests
./runUnitTest.sh
```

或者使用ctest命令：
```bash
ctest
```

## 添加新的测试用例

1. 在相应的模块测试目录中创建新的测试文件
2. 在对应模块的CMakeLists.txt中添加新的测试可执行文件
3. 更新主CMakeLists.txt以包含新的测试目录

## 测试覆盖率

在Debug模式下编译时，测试框架会自动启用覆盖率分析功能。可以通过以下命令生成覆盖率报告：

```bash
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## 注意事项

1. 所有测试文件应遵循Qt Test框架的命名规范
2. 测试用例应尽可能覆盖所有代码分支
3. 对于涉及硬件操作的测试，应使用模拟对象(Mock)进行替代
4. 测试代码应保持简洁明了，易于维护