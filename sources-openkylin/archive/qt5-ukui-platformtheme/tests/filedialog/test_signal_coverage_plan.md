# KyFileDialog 测试覆盖率提升计划

## 1. 问题分析

### 1.1 当前测试状态
- 现有测试只测试了 `QFileDialog` 的基本创建和属性设置
- 没有测试 `KyNativeFileDialog` 的任何功能
- 大量 `connect` 的槽函数未被覆盖

### 1.2 KyNativeFileDialog 信号连接分类

| 类别 | 信号源 | 可测试性 |
|------|--------|----------|
| UI组件信号 | 按钮、输入框、菜单等 | ✅ 可直接模拟 |
| 自定义信号 | locationChangeStart/End, updateSideModel | ✅ 可直接触发 |
| Peony组件信号 | DirectoryViewContainer 系列 | ❌ 需要 Mock |
| 系统信号 | QGSettings, QTimer | ⚠️ 部分可模拟 |

---

## 2. 测试策略

### 2.1 直接调用槽函数测试
对于不依赖 Peony 组件的槽函数，可以直接调用：

```cpp
// 可直接测试的槽函数
dialog->goBack();           // 导航功能
dialog->goForward();        // 导航功能
dialog->goToParent();       // 导航功能
dialog->onSwitchView();     // 视图切换
dialog->updateWindowState(); // 状态更新
dialog->selectName();       // 名称选择
dialog->updateStatusBar();  // 状态栏更新
dialog->onNewFolder();      // 新建文件夹
dialog->setSortType();      // 排序类型
dialog->searchButtonClicked(bool); // 搜索按钮
dialog->lineEditTextChange(QString); // 文本变化
```

### 2.2 模拟 UI 组件信号测试
通过模拟 UI 组件的信号来触发槽函数：

```cpp
// 模拟按钮点击
QTest::mouseClick(dialog->getBackButton(), Qt::LeftButton);
QTest::mouseClick(dialog->getForwardButton(), Qt::LeftButton);
QTest::mouseClick(dialog->getToParentButton(), Qt::LeftButton);
QTest::mouseClick(dialog->getAcceptButton(), Qt::LeftButton);
QTest::mouseClick(dialog->getRejectButton(), Qt::LeftButton);

// 模拟文本输入
QTest::keyClicks(dialog->getFileNameEdit(), "test.txt");

// 模拟下拉框选择
QTest::keyClick(dialog->getFileTypeCombo(), Qt::Key_Down);
```

### 2.3 手动触发自定义信号
对于 KyNativeFileDialog 发出的自定义信号，可以直接触发：

```cpp
// 触发自定义信号
emit dialog->locationChangeStart();
emit dialog->locationChangeEnd();
emit dialog->updateSideModel();
```

---

## 3. 测试用例设计

### 3.1 导航功能测试
- `test_goBack()` - 测试返回功能
- `test_goForward()` - 测试前进功能
- `test_goToParent()` - 测试返回上级目录

### 3.2 视图功能测试
- `test_setViewMode()` - 测试设置视图模式
- `test_onSwitchView()` - 测试视图切换回调

### 3.3 文件选择功能测试
- `test_setNameFilters()` - 测试设置过滤器
- `test_selectNameFilter()` - 测试选择过滤器
- `test_lineEditTextChange()` - 测试文本变化处理

### 3.4 按钮功能测试
- `test_acceptButton()` - 测试确认按钮
- `test_rejectButton()` - 测试取消按钮
- `test_newFolderButton()` - 测试新建文件夹按钮

### 3.5 信号触发测试
- `test_locationChangeSignals()` - 测试位置变化信号
- `test_updateSideModelSignal()` - 测试侧边栏更新信号

---

## 4. 实现步骤

### Step 1: 修改 CMakeLists.txt
- 添加必要的依赖库链接

### Step 2: 创建测试夹具 (Test Fixture)
- 初始化 KyNativeFileDialog
- 提供 UI 组件访问接口

### Step 3: 实现基础功能测试
- 导航功能测试
- 视图切换测试

### Step 4: 实现信号触发测试
- 手动触发自定义信号
- 验证槽函数响应

### Step 5: 实现 UI 模拟测试
- 模拟按钮点击
- 模拟文本输入
- 模拟下拉框选择

---

## 5. 注意事项

1. **Peony 依赖**: KyNativeFileDialog 依赖 Peony 库，需要确保测试环境中有这些库
2. **UI 初始化**: 某些功能需要先初始化 UI 组件
3. **信号验证**: 使用 QSignalSpy 验证信号是否正确发出
4. **异常处理**: 某些操作可能失败，需要适当处理异常

---

## 6. 预期效果

通过以上测试用例，预计可以覆盖以下未覆盖的函数：
- `goBack()`, `goForward()`, `goToParent()`
- `onSwitchView()`, `updateWindowState()`
- `setViewMode()`, `setNameFilters()`, `selectNameFilter()`
- `onAcceptButtonClicked()`, `onRejectButtonClicked()`
- `lineEditTextChange()`, `updateStatusBar()`
- `searchButtonClicked()`, `setSortType()`
- 以及多个 lambda 槽函数