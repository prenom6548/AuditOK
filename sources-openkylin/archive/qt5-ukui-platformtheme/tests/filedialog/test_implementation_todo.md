# KyFileDialog 测试覆盖率提升 - TODO 列表

## 阶段 1: 准备工作 (Phase 1: Preparation)

### 1.1 修改 CMakeLists.txt
- [ ] 添加 Qt5::Test 依赖
- [ ] 添加 --coverage 编译标志
- [ ] 添加项目依赖库链接 (如需要)

### 1.2 创建测试夹具类
- [ ] 创建 `TestKyFileDialog` 类继承自 `QObject`
- [ ] 添加 `m_dialog` 成员变量 (KyNativeFileDialog*)
- [ ] 添加 `m_ui` 成员变量 (Ui_KyFileDialog*)
- [ ] 实现 `initTestCase()` - 初始化应用和对话框
- [ ] 实现 `cleanupTestCase()` - 清理资源

---

## 阶段 2: 基础功能测试 (Phase 2: Basic Function Tests)

### 2.1 导航功能测试
- [ ] `test_goBack()` - 测试 goBack() 方法
- [ ] `test_goForward()` - 测试 goForward() 方法  
- [ ] `test_goToParent()` - 测试 goToParent() 方法
- [ ] `test_goToUri()` - 测试 goToUri() 方法

### 2.2 视图功能测试
- [ ] `test_setViewMode()` - 测试设置视图模式 (List/Icon)
- [ ] `test_viewMode()` - 测试获取当前视图模式
- [ ] `test_onSwitchView()` - 测试视图切换回调

### 2.3 目录和文件选择测试
- [ ] `test_setDirectory()` - 测试设置目录
- [ ] `test_directory()` - 测试获取当前目录
- [ ] `test_setFileMode()` - 测试设置文件模式
- [ ] `test_fileMode()` - 测试获取文件模式

---

## 阶段 3: 信号触发测试 (Phase 3: Signal Trigger Tests)

### 3.1 自定义信号测试
- [ ] `test_locationChangeStartSignal()` - 触发 locationChangeStart 信号
- [ ] `test_locationChangeEndSignal()` - 触发 locationChangeEnd 信号
- [ ] `test_updateSideModelSignal()` - 触发 updateSideModel 信号

### 3.2 槽函数直接调用测试
- [ ] `test_updateWindowState()` - 直接调用 updateWindowState()
- [ ] `test_selectName()` - 直接调用 selectName()
- [ ] `test_updateStatusBar()` - 直接调用 updateStatusBar()
- [ ] `test_setSortType()` - 直接调用 setSortType()

---

## 阶段 4: UI 组件模拟测试 (Phase 4: UI Component Simulation Tests)

### 4.1 按钮点击测试
- [ ] `test_backButton_click()` - 模拟后退按钮点击
- [ ] `test_forwardButton_click()` - 模拟前进按钮点击
- [ ] `test_toParentButton_click()` - 模拟返回上级按钮点击
- [ ] `test_acceptButton_click()` - 模拟确认按钮点击
- [ ] `test_rejectButton_click()` - 模拟取消按钮点击

### 4.2 文本输入测试
- [ ] `test_fileNameEdit_textChanged()` - 模拟文件名输入
- [ ] `test_lineEditTextChange_slot()` - 测试 lineEditTextChange 槽函数

### 4.3 下拉框测试
- [ ] `test_fileTypeCombo_activated()` - 模拟文件类型选择
- [ ] `test_setNameFilters()` - 测试设置过滤器
- [ ] `test_selectNameFilter()` - 测试选择过滤器

### 4.4 菜单测试
- [ ] `test_modeMenu_triggered()` - 模拟视图模式菜单选择
- [ ] `test_sortMenu_aboutToShow()` - 测试排序菜单显示

---

## 阶段 5: 属性和状态测试 (Phase 5: Property and State Tests)

### 5.1 接受模式测试
- [ ] `test_setAcceptMode()` - 测试设置接受模式
- [ ] `test_acceptMode()` - 测试获取接受模式

### 5.2 选项测试
- [ ] `test_setOptions()` - 测试设置选项
- [ ] `test_setOption()` - 测试设置单个选项
- [ ] `test_testOption()` - 测试检查选项

### 5.3 标签文本测试
- [ ] `test_setLabelText()` - 测试设置标签文本
- [ ] `test_labelText()` - 测试获取标签文本

---

## 阶段 6: 信号验证测试 (Phase 6: Signal Verification Tests)

### 6.1 使用 QSignalSpy 验证信号
- [ ] `test_filesSelected_signal()` - 验证 filesSelected 信号发出
- [ ] `test_directoryEntered_signal()` - 验证 directoryEntered 信号发出
- [ ] `test_filterSelected_signal()` - 验证 filterSelected 信号发出
- [ ] `test_currentChanged_signal()` - 验证 currentChanged 信号发出

---

## 测试用例模板

```cpp
void TestKyFileDialog::test_xxx()
{
    // Arrange: 准备测试数据
    
    // Act: 执行要测试的操作
    m_dialog->someMethod();
    
    // Assert: 验证结果
    QVERIFY(expectedValue == actualValue);
}
```

---

## 关键实现细节

### 访问 UI 组件
```cpp
// 通过 mKyFileDialogUi 访问
m_dialog->mKyFileDialogUi->m_backButton
m_dialog->mKyFileDialogUi->m_forwardButton
m_dialog->mKyFileDialogUi->m_fileNameEdit
// ... 其他组件
```

### 模拟信号触发
```cpp
// 方法1: 直接调用槽函数
m_dialog->goBack();

// 方法2: 手动触发信号
emit m_dialog->locationChangeStart();

// 方法3: 模拟 UI 事件
QTest::mouseClick(m_dialog->mKyFileDialogUi->m_backButton, Qt::LeftButton);
```

### 使用 QSignalSpy 验证信号
```cpp
QSignalSpy spy(m_dialog, &KyNativeFileDialog::directoryEntered);
m_dialog->setDirectory("/tmp");
QCOMPARE(spy.count(), 1);
```

---

## 预期覆盖率提升

通过以上测试用例，预计可以覆盖以下函数：
- 导航: goBack, goForward, goToParent, goToUri
- 视图: setViewMode, viewMode, onSwitchView
- 文件选择: setNameFilters, selectNameFilter, selectedFiles
- 按钮: onAcceptButtonClicked, onRejectButtonClicked, onNewFolder
- 状态: updateWindowState, updateStatusBar, updateAcceptButtonState
- 信号: locationChangeStart/End, updateSideModel, directoryEntered
- 以及多个 lambda 槽函数

---

## 优先级排序

1. **高优先级**: 导航功能、按钮点击、文件选择
2. **中优先级**: 视图切换、过滤器设置、信号验证
3. **低优先级**: 菜单操作、高级功能