# 桌面小插件管理（WidgetManagerDialog）适配说明

本文档说明 `peony-extensions-v11/peony-extensions` 中**桌面时钟**与**桌面图片**小插件的配置方式，并归纳第三方插件若要出现在「桌面小插件管理」中、且点击/拖拽添加行为正确时，需要在 `metadata.json` 与 QML 侧注意的事项。

实现代码位于：

- `peony-qt-desktop/desktop-widgets/base/widget-manager-dialog.{h,cpp}`

---

## 1. 示例：`org.ukui.desktop-clock-widget`（桌面时钟）

### 1.1 概要

- **用途**：在桌面显示模拟/数字时钟，多种尺寸变体。
- **入口**：`metadata.json` 中 `ShowIn` 含 `Desktop`，`Contents.ShowInManager: true`。
- **主 QML**：`Contents.Main` → `ui/main.qml`（实际桌面实例）；各变体预览使用独立文件 `ui/preview/small.qml`、`medium.qml`、`large.qml`。
- **C++ 后端**：`src/clockbackend.*` 等（与桌面实例逻辑相关；管理器内预览仅加载预览 QML）。

### 1.2 `metadata.json` 要点

| 字段 | 说明 |
|------|------|
| `Contents.ShowInManager` | 必须为 `true`，否则不会出现在管理器列表。 |
| `Contents.ManagerVariants` | 数组，每一项对应管理器中的一张卡片（一种「规格」）。 |
| 变体内 `id` | 变体标识，写入配置 `variantId`，供 QML 或后端区分 small/medium/large。 |
| `name` / `description` | 卡片标题与副标题；可覆盖全局 `Name`/`Description`。 |
| `sizeHint` | 网格占位 `[列数, 行数]`，与桌面布局网格一致。 |
| `previewSize` | **仅管理器内** QQuickWidget 的固定逻辑尺寸 `[宽, 高]`，应与对应 `preview/*.qml` 设计分辨率一致，避免预览拉伸变形。 |
| `desktopSize` | **桌面实例**像素 `[宽, 高]`，由 `PlatformWindow::addWidgetFromVariant` 写入子配置 `width`/`height`。 |
| `previewMain` | 相对插件根目录的预览 QML 路径（如 `ui/preview/small.qml`）；解析时会尝试 `root/相对路径` 及「仅文件名」铺平路径。 |
| `fixedResizable` | 映射为配置中的 `Resizable`（布尔）。 |

### 1.3 与代码的对应关系

- 管理器通过 `previewUrlsFromMetadata` 解析 `previewMain` 或顶层 `Contents.Preview`。
- 预览上下文注入：`widgetPreviewMode = true`，`widgetPreviewVariantId = variant["id"]`，预览 QML 可根据二者减少动效或读表盘样式。

---

## 2. 示例：`org.ukui.desktop-image-widget`（桌面图片）

### 2.1 概要

- **用途**：在桌面选择并显示本地图片，固定多种尺寸。
- **结构与时钟插件对称**：同样使用 `ManagerVariants`（small/medium/large）、`ui/main.qml` + `ui/preview/*.qml`、`src/imagebackend.*` 处理选图与显示。

### 2.2 `metadata.json` 要点

与时钟插件**同一套字段语义**（`ShowInManager`、`ManagerVariants`、`previewSize`、`desktopSize`、`previewMain`、`sizeHint`、`fixedResizable`）。二者差异仅在业务 QML 与后端（时钟 vs 图片），**适配管理器的约定相同**。

---

## 3. 其他插件适配「桌面小插件管理」的检查清单

### 3.1 必备与可见性

1. **`ShowIn`**：需包含 UkuiQuick 所识别的 **Desktop**（与现有 ukui-quick 约定一致）。
2. **管理器开关**：在 `Contents` 中配置 **`ShowInManager`** 或 **`ManagerVisible`**。解析顺序为：若存在键 `ShowInManager` 则仅取其布尔值；否则若存在 `ManagerVisible` 则取其布尔值；若两键都不存在则**不显示**在管理器。
3. **无变体时**：若省略 `ManagerVariants`，管理器会为该插件生成**一条**空 `QVariantMap()` 卡片；此时应保证默认 `Contents.Main` + 可选 `Preview` 能加载。

### 3.2 `ManagerVariants` 各字段（JSON → QVariantMap）

| 键名 | 类型 | 含义 |
|------|------|------|
| `id` | string | 变体 ID；写入桌面配置 `variantId`。 |
| `name` | string | 卡片标题（可选）。 |
| `description` | string | 卡片描述（可选）。 |
| `sizeHint` | `[int, int]` | 网格尺寸；`onAddWidget` 若检测到含 `sizeHint` 会走 `addWidgetFromVariant`。 |
| `previewSize` | `[int, int]` | 管理器预览控件固定宽高（逻辑像素）。 |
| `desktopSize` | `[int, int]` | 桌面上实例初始像素宽高。 |
| `previewMain` | string | 预览专用 QML 相对路径。 |
| `fixedResizable` | bool | 是否可缩放，映射为配置 `Resizable`。 |

**注意**：`WidgetManagerDialog::onAddWidget` 在 variant 含 **`sizeHint`**、**`fixedResizable`** 或 **`desktopSize`** 任一即调用 `PlatformWindow::addWidgetFromVariant`；否则走 `DesktopWidgetsManager::addWidget(window, widgetId)` 默认路径。

### 3.3 平台写入配置（`addWidgetFromVariant`）

下列由 Peony 写入子配置（见 `platformwindow.cpp`），插件或 `WidgetItem` 应能消费：

- `id`、`instanceId`
- `variantId`（来自 variant `id`）
- `SizeHint`（来自 `sizeHint`）
- `Resizable`（来自 `fixedResizable`）
- `width`、`height`（来自 `desktopSize` 两项）
- 拖拽落点时还有 `x`、`y`（与布局镜像逻辑相关）

### 3.4 预览 QML 建议

- 使用上下文属性 **`widgetPreviewMode`** 区分管理器预览与真实桌面，避免重负载动画、定时器或真实文件对话框。
- 使用 **`widgetPreviewVariantId`** 区分同一插件的多张卡片。
- 预览路径尽量与 **`previewSize`** 一致，减少模糊与圆角裁切问题；若 `QtGraphicalEffects` 不可用，会回退为 `setSource` + 矩形 `mask`。

### 3.5 拖拽 MIME

- 管理器拖拽载荷为 JSON：`widgetId` + 变体字段；类型含 `application/x-peony-widget`。
- 桌面 Drop 侧由 `addWidgetFromDropJson` 解析并与点击添加同样进入 `addWidgetFromVariant`。

### 3.6 常见问题

- **管理器不显示插件**：检查 `ShowInManager`/`ManagerVisible` 与 `ShowIn` Desktop。
- **点击添加尺寸不对**：确认 variant 含 `desktopSize` 或 `sizeHint` 等，使 `onAddWidget` 走 `addWidgetFromVariant`。
- **预览黑块/透底**：预览区依赖不透明调色板；插件 QML 避免假定全透明清屏为桌面色。
- **仅 `Preview` 无 `previewMain`**：可在顶层 `Contents` 设 `Preview`，变体级 `previewMain` 优先。

---

## 4. 参考路径

- 扩展包示例：`peony-extensions-v11/peony-extensions/org.ukui.desktop-clock-widget/`、`org.ukui.desktop-image-widget/`
- 管理器实现：`peony-qt-desktop/desktop-widgets/base/widget-manager-dialog.cpp`
- 桌面添加：`peony-qt-desktop/qml-desktop/platformwindow.cpp`（`addWidgetFromVariant` / `addWidgetFromDropJson`）
