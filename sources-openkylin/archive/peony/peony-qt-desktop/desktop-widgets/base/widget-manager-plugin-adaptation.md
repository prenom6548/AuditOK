# 桌面小插件管理（WidgetManagerDialog）适配说明

桌面小插件管理器与桌面实例共用 ukui-quick 的标准加载链路和同一个 `Contents.Main`。插件不再维护 `previewMain`、`previewSize` 或 `ui/preview/*.qml`。

实现代码位于：

- `peony-qt-desktop/desktop-widgets/base/widget-manager-dialog.{h,cpp}`
- `peony-qt-desktop/qml-desktop/WidgetManagerPreviewPane.qml`

## 1. metadata.json

插件只需在 `Contents` 中声明是否展示及支持的标准尺寸：

```json
{
  "ShowIn": "Desktop",
  "Contents": {
    "Main": "main.qml",
    "ShowInManager": true,
    "ManagerSizes": [
      {
        "id": "small",
        "description": "Compact description",
        "description[zh_CN]": "小尺寸描述"
      },
      {
        "id": "medium",
        "description": "Wide description",
        "description[zh_CN]": "中尺寸描述"
      },
      {
        "id": "large",
        "description": "Large description",
        "description[zh_CN]": "大尺寸描述"
      }
    ]
  }
}
```

`ManagerSizes` 的顺序就是管理器中的展示顺序。名称始终读取插件顶层本地化 `Name`；描述优先读取当前项的 `description[语言_地区]`，然后依次回退到 `description` 和插件顶层本地化 `Description`。三档尺寸由 Peony 统一定义：

| id | SizeHint | 桌面尺寸 | 管理器预览尺寸 |
|---|---:|---:|---:|
| `small` | `[2, 2]` | `136 × 136` | `96 × 96` |
| `medium` | `[4, 2]` | `296 × 136` | `209 × 96` |
| `large` | `[4, 4]` | `296 × 296` | `209 × 209` |

管理器固定使用 `12 / 17`（约 `0.706`）做整体等比缩放。预览宽高由桌面尺寸乘该比例后四舍五入得到，因此插件不需要重复配置像素尺寸。

迁移期间仍兼容旧 `ManagerVariants`，但只读取其中的 `id`；`previewMain`、`previewSize`、自定义 `desktopSize` 等预览字段会被忽略。新插件应只使用 `ManagerSizes`。

## 2. 加载与显示

每一张预览卡片都执行以下 ukui-quick 标准流程：

1. `WidgetLoader::globalLoader().loadWidget(widgetId)` 创建独立 `Widget`。
2. 为该实例写入 `variantId`、`SizeHint`、桌面 `width/height` 等临时配置。
3. `WidgetQuickItem::loadWidgetItem()` 从 `Contents.Main` 创建真实 `WidgetItem`。
4. QML 将该 `WidgetItem` 放进卡片，保持桌面宽高，并对整个 Item 应用 `12 / 17` 缩放。

这保证预览和添加到桌面的内容来自同一棵 QML UI。管理器仍只创建一个 `SharedEngineView`，各插件实例共享 ukui-quick 引擎，但拥有独立上下文和配置。

## 3. 插件 QML 要求

- `Contents.Main` 的根项必须是 `WidgetItem`，遵循 ukui-quick 约定。
- 可以读取 `widgetPreviewMode`；管理器中该值为 `true`。它只应用于禁用右键菜单、文件选择、删除等桌面交互，不应切换到另一套视觉实现。
- 从 `Widget.config.variantId` 获取当前 `small`、`medium` 或 `large`；管理器不再额外注入一份重复的变体上下文属性。
- 插件应根据自身真实宽高或 `variantId` 切换布局，保证桌面与管理器逻辑一致。
- 预览实例的 `instanceId` 为 `-1`，不应读写真实桌面实例数据。
- 不再安装 `small.qml`、`medium.qml`、`large.qml` 等预览文件；只安装 `main.qml` 及其真实运行依赖。

## 4. 添加和拖放

点击与拖放都调用 `PlatformWindow::addWidgetFromVariant`。Peony 会写入：

- `id`、`instanceId`
- `variantId`
- `SizeHint`
- `Resizable`
- `width`、`height`
- 拖放或自动放置计算出的 `x`、`y`

拖拽载荷使用 `application/x-peony-widget`，内容是 `widgetId` 和标准变体字段。桌面仍负责空位查找、相交规避与跨屏处理。

## 5. 排查

- 管理器不显示插件：检查 `ShowIn` 是否包含 `Desktop`，以及 `ShowInManager` 是否为 `true`。
- 缺少某档卡片：检查 `ManagerSizes` 拼写，仅支持 `small`、`medium`、`large`。
- 预览不显示：检查 `Contents.Main` 是否可由 ukui-quick 加载，且根项是否为 `WidgetItem`。
- 预览与桌面不同：检查插件是否错误地在 `widgetPreviewMode` 下切换了视觉内容；该属性只应屏蔽交互或副作用。
- 尺寸不对：检查插件内部的尺寸判断是否与三档桌面尺寸一致。
- 尺寸切换不可用：`canResizeWidgetTo(widgetItem, width, height)` 会同时拒绝越过当前屏幕或与其他桌面项相交的目标尺寸。

参考实现：`org.ukui.desktop-clock-widget`、`org.ukui.desktop-image-widget`。
