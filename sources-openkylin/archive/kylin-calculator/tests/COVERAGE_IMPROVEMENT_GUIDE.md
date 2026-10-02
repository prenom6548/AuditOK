# 单元测试覆盖率提升指南

## 当前覆盖率分析

| 指标 | 当前值 | 目标值 |
|------|---------|---------|
| 行覆盖率 | 51.4% | 70%+ |
| 函数覆盖率 | 48.8% | 65%+ |

---

## 一、测试优先级策略

### 🔴 高优先级（立即补充）

这些模块覆盖率为 0-5%，补充测试能快速提升整体覆盖率：

#### 1. `src/funclist.cpp` (0% → 目标 60%+)
**未测试功能**：
- FuncList 构造函数
- `setWidgetUi()` - UI 初始化
- `setWidgetStyle()` - LIGHT/DARK 主题分支

**补充测试用例**：
```cpp
// tests/testfunlist.h (已创建)
void testConstruct();
void testSetWidgetUi();
void testSetWidgetStyleLight();
void testSetWidgetStyleDark();
void testFuncModelLabel();
void testFuncModelWid();
void testFuncToolLabel();
void testFuncToolWid();
void testThemeSwitch();
```

#### 2. `src/scientificmodel.cpp` (1.9% → 目标 40%+)
**未测试功能**：
- `setWidgetUi()` - 科学计算器 UI
- `setWidgetStyle()` - 主题分支
- `sciFormatInput()` - 输入格式化
- `changeBtnSinDisplay()` - 三角函数切换
- `changeBtnRadDisplay()` - 角度/弧度切换

**补充测试用例**：
```cpp
void testScientificModelConstruct();
void testSetWidgetUi();
void testSetWidgetStyleLight();
void testSetWidgetStyleDark();
void testSciFormatInput();
void testChangeBtnSinDisplay();
void testChangeBtnRadDisplay();
void testUpdateBtnSinDisplay();
void testUpdateBtnRadDisplay();
```

#### 3. `src/standardmodel.cpp` (3.2% → 目标 40%+)
**未测试功能**：
- `setWidgetUi()` - 标准计算器 UI
- `setWidgetStyle()` - 主题分支
- 基础计算操作

**补充测试用例**：
```cpp
void testStandardModelConstruct();
void testSetWidgetUi();
void testSetWidgetStyleLight();
void testSetWidgetStyleDark();
void testBasicCalculation();
void testClearOperation();
void testDeleteOperation();
```

#### 4. `src/toolmodel.cpp` (7.7% → 目标 50%+)
**未测试功能**：
- `setWidgetUi()` - 换算器 UI
- `initUnitList()` - 单位列表初始化
- `unitConversion()` - 单位换算逻辑
- `updateRate()` - 汇率更新
- `unitListBefShow()` / `unitListAftShow()` - 单位列表显示

**补充测试用例**：
```cpp
void testToolModelConstruct();
void testSetWidgetUi();
void testInitUnitList();
void testUnitConversion();
void testUpdateRate();
void testUnitListBefShow();
void testUnitListAftShow();
void testDealDone();
void testUpdateRateName();
```

#### 5. `src/titlebar.cpp` (6.4% → 目标 40%+)
**未测试功能**：
- `setWidgetUi()` - 标题栏 UI
- `setWidgetStyle()` - 主题分支
- `setFuncLabel()` - 功能标签设置
- 模式切换槽函数

**补充测试用例**：
```cpp
void testTitleBarConstruct();
void testSetWidgetUi();
void testSetWidgetStyleLight();
void testSetWidgetStyleDark();
void testSetFuncLabel();
void testCreateInterUi();
void testSlotModeChange();
void testSlotChangeStandard();
void testSlotChangeScientific();
```

### 🟡 中优先级（后续补充）

| 模块 | 当前覆盖率 | 重点测试内容 |
|-------|-----------|-------------|
| `src/basicbutton.cpp` | 20.8% | 按钮点击、图片加载、文本设置 |
| `calc_programmer/processformula.cpp` | 17.9% | 公式解析、错误处理 |
| `logic_center.cpp` | 38.9% | 核心计算逻辑、输入处理 |
| `calculate/input/InputProcess.cpp` | 41.7% | 输入验证、格式转换 |

---

## 二、测试用例补充方法

### 方法 1：覆盖率驱动测试

**步骤**：
1. 生成覆盖率报告
2. 查看未覆盖的代码行
3. 为每个未覆盖行编写测试
4. 重新运行测试验证覆盖率提升

**命令**：
```bash
# 生成详细覆盖率报告
lcov --list coverage_fixed.info > coverage_details.txt

# 查看未覆盖行
grep "0%$" coverage_details.txt
```

### 方法 2：功能驱动测试

**按照功能模块分类补充测试**：

```cpp
// 示例：ScientificModel 测试框架
class TestScientificModel : public QObject
{
    Q_OBJECT

private slots:
    // UI 初始化测试
    void testConstruct();
    void testSetWidgetUi();
    void testSetWidgetStyleLight();
    void testSetWidgetStyleDark();

    // 输入处理测试
    void testSciFormatInputEmpty();
    void testSciFormatInputNumbers();
    void testSciFormatInputOperators();

    // 三角函数测试
    void testChangeBtnSinDisplay();
    void testUpdateBtnSinDisplay();

    // 角度/弧度测试
    void testChangeBtnRadDisplay();
    void testUpdateBtnRadDisplay();
};
```

### 方法 3：边界条件测试

**为每个函数补充边界条件测试**：

```cpp
// 示例：输入处理边界测试
void testInputProcess_EmptyInput();
void testInputProcess_SingleDigit();
void testInputProcess_MaxLength();
void testInputProcess_SpecialChars();
void testInputProcess_OperatorSequence();
void testInputProcess_BracketMismatch();
```

---

## 三、集成到单元测试项目

### 步骤 1：更新 unitTest.pro

```qmake
SOURCES += \
    $$PWD/testclass.cpp  \
    $$PWD/testfunlist.cpp \
    $$PWD/testscientificmodel.cpp \
    $$PWD/teststandardmodel.cpp \
    $$PWD/testtoolmodel.cpp \
    $$PWD/testtitlebar.cpp

HEADERS += \
    $$PWD/testclass.h    \
    $$PWD/testfunlist.h \
    $$PWD/testscientificmodel.h \
    $$PWD/teststandardmodel.h \
    $$PWD/testtoolmodel.h \
    $$PWD/testtitlebar.h
```

### 步骤 2：更新 mainTest.cpp

```cpp
#include <QtTest/QtTest>
#include "testclass.h"
#include "testfunlist.h"
#include "testscientificmodel.h"
#include "teststandardmodel.h"
#include "testtoolmodel.h"
#include "testtitlebar.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTest::qExec(new TestClass, argc, argv);
    QTest::qExec(new TestFunList, argc, argv);
    QTest::qExec(new TestScientificModel, argc, argv);
    QTest::qExec(new TestStandardModel, argc, argv);
    QTest::qExec(new TestToolModel, argc, argv);
    QTest::qExec(new TestTitleBar, argc, argv);
}
```

### 步骤 3：重新编译和测试

```bash
cd tests
qmake unitTest.pro
# make clean
make -j9

# 运行测试
export QT_QPA_PLATFORM=offscreen
./test

# 生成覆盖率报告
lcov --capture --directory . --output-file coverage.info \
    --base-directory .. --gcov-tool /usr/bin/gcov \
    --ignore-errors source --exclude '*/usr/include/*'

genhtml coverage.info --output-directory coverage_report \
    --title "Kylin Calculator Test Coverage"
```

---

## 四、测试编写最佳实践

### 1. 测试命名规范

```cpp
// ✅ 良好的命名
void testSetWidgetStyleLight();
void testUnitConversion_CelsiusToFahrenheit();
void testInputProcess_EmptyInput();

// ❌ 不好的命名
void test1();
void testStyle();
void testInput();
```

### 2. 测试结构模式

```cpp
void TestExample::testFeature()
{
    // Arrange - 准备测试数据
    Calculator calc;
    QString input = "123+456";

    // Act - 执行被测操作
    calc.calculate(input);

    // Assert - 验证结果
    QCOMPARE(calc.result(), "579");
}
```

### 3. 避免测试依赖

```cpp
// ❌ 错误：依赖全局状态
void TestExample::testTheme()
{
    WidgetStyle::themeColor = ThemeColor::LIGHT;
    // 测试...
}

// ✅ 正确：每个测试独立设置
void TestExample::testThemeLight()
{
    WidgetStyle::themeColor = ThemeColor::LIGHT;
    // 测试...
}

void TestExample::testThemeDark()
{
    WidgetStyle::themeColor = ThemeColor::DARK;
    // 测试...
}
```

### 4. Mock 外部依赖

```cpp
// 对于需要网络或系统的测试，使用 Mock
void TestToolModel::testUpdateRate_Mock()
{
    // Mock 网络响应
    MockNetworkReply reply;
    reply.setData("mock_rate_data");

    ToolModel model;
    model.setMockReply(&reply);

    // 执行测试
    model.updateRate();

    // 验证
    QCOMPARE(model.getRate(), "mock_rate");
}
```

---

## 五、自动化检查

### 创建覆盖率检查脚本

```bash
#!/bin/bash
# scripts/check_coverage.sh

THRESHOLD=70
COVERAGE=$(lcov --summary coverage_fixed.info | grep "lines" | grep -oP '\d+\.\d+')

if (( $(echo "$COVERAGE < $THRESHOLD" | bc -l) )); then
    echo "覆盖率 $COVERAGE% 低于目标 $THRESHOLD%"
    exit 1
else
    echo "覆盖率 $COVERAGE% 达到目标"
    exit 0
fi
```

### 持续集成配置

```yaml
# .github/workflows/test.yml
name: Run Tests and Check Coverage

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2

      - name: Build and Test
        run: |
          cd tests
          qmake unitTest.pro
          make -j4
          ./test

      - name: Generate Coverage
        run: |
          lcov --capture --directory tests --output-file coverage.info
          genhtml coverage.info --output-directory coverage_report

      - name: Check Coverage Threshold
        run: ./scripts/check_coverage.sh
```

---

## 六、补充测试预期效果

补充高优先级模块测试后，预计覆盖率提升：

| 模块 | 当前 | 补充后 | 提升幅度 |
|------|-------|---------|---------|
| `src/funclist.cpp` | 0% | 60%+ | +60% |
| `src/scientificmodel.cpp` | 1.9% | 40%+ | +38% |
| `src/standardmodel.cpp` | 3.2% | 40%+ | +37% |
| `src/titlebar.cpp` | 6.4% | 40%+ | +34% |
| `src/toolmodel.cpp` | 7.7% | 50%+ | +42% |

**整体覆盖率预期提升**：51.4% → **65-70%**

---

## 七、下一步行动

1. **本周任务**：
   - [ ] 补充 `testfunlist.cpp` 完整测试
   - [ ] 补充 `testscientificmodel.cpp` 基础测试
   - [ ] 补充 `teststandardmodel.cpp` 基础测试

2. **下周任务**：
   - [ ] 补充 `testtoolmodel.cpp` 测试
   - [ ] 补充 `testtitlebar.cpp` 测试
   - [ ] 更新 unitTest.pro 和 mainTest.cpp

3. **持续优化**：
   - [ ] 运行测试并检查覆盖率
   - [ ] 针对 0% 覆盖行补充测试
   - [ ] 设置 CI 自动检查覆盖率
