# 任务栏 (Panel) 对话测试

## 测试流程

### 1. 查合并图标
> 任务栏图标是合并的吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetMergeIcons
```
返回值: 0=从不合并, 1=始终合并, 2=窗口满时合并

### 2. 设置合并图标
> 任务栏图标始终合并

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetMergeIcon 1
```

### 3. 面板位置
> 任务栏位置

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelLocation
```
返回值: 0=底部, 1=顶部, 2=左侧, 3=右侧

### 4. 设置面板位置
> 任务栏移到顶部

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelLocation 1
```

### 5. 面板尺寸
> 任务栏调大

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelSizePolicy

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelSizePolicy 2
```
可选值: 0=小, 1=中, 2=大

### 6. 自动隐藏
> 任务栏自动隐藏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelAutoHide

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelAutoHide true
```

### 7. 锁定面板
> 锁定任务栏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelLockState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelLockState true
```

### 8. 所有屏幕显示面板
> 所有屏幕都显示任务栏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetShowPanelOnAllScreens

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowPanelOnAllScreens true
```

### 9. 所有面板显示托盘
> 所有面板都显示系统托盘

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowSystemTrayOnAllPanels true
```

### 10. 任务栏图标显示策略
> 任务栏图标显示在所有工作区

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetTaskBarIconsShowedOn 1
```
可选值: 0=仅当前工作区, 1=所有工作区

### 11. 任务视图按钮
> 显示任务视图按钮

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowTaskView true
```

### 12. 搜索按钮
> 显示搜索按钮

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowSearch true
```

---

## 完整对话样例

**你**: 任务栏调大
**誩**: ✅ 已设为最大尺寸

**你**: 任务栏自动隐藏
**誩**: ✅ 已开启自动隐藏

**你**: 任务栏移到顶部
**誩**: ✅ 已移到顶部

**你**: 图标始终合并
**誩**: ✅ 图标已设为始终合并

**你**: 锁住任务栏
**誩**: ✅ 任务栏已锁定

**你**: 所有屏幕都显示任务栏
**誩**: ✅ 已开启多屏显示任务栏

**你**: 显示搜索和任务视图按钮
**誩**: ✅ 已显示搜索按钮和任务视图按钮
