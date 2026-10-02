# UKUI Preview Hub 用户手册

本手册说明如何在自有应用中嵌入 **ukui-previewhub** 动态库，获得与历史 Peony 预览插件相近的文件预览能力（文本、图片、视频、音频、Office 文档经 LibreOffice 转 PDF 后预览、PDF 等）。**本库不依赖 Peony 运行库或开发包**，元数据与本地监视由库内 `PreviewHubFileInfo` / `PreviewHubFileWatcher` 实现。

## 1. 概述

- **动态库名称**：`libukui-previewhub.so`（带版本号 SONAME，例如 `libukui-previewhub.so.1`）。
- **实现方式**：库内源码为原 `peony-preview-file-content-plugin` 预览相关逻辑的**独立副本**，通过导出 API 创建可嵌入的 `QWidget`。
- **运行环境**：Qt 5，Linux（Ubuntu 及衍生版）。显示栈由宿主应用决定，可同时适配 **X11（xcb）** 与 **Wayland**：只要宿主进程以 Qt 正常初始化 GUI（例如 `QApplication`），无需在本库中单独指定显示后端。

## 2. 依赖与系统准备

### 2.1 运行时依赖

宿主应用及本库需要链接并能在运行期加载：

| 类别 | 说明 |
|------|------|
| Qt 5 | Core、Gui、Widgets、Multimedia、MultimediaWidgets、WebEngine、WebChannel 等 |
| KF5 Syntax Highlighting | 文本语法高亮 |
| LibreOffice | 用于 Office 文档转 PDF 再预览（需 `which libreoffice` 可用） |
| 系统多媒体后端 | 音视频预览依赖 Qt Multimedia 所选后端 |

### 2.2 Wayland 与 X11

- **Wayland 会话**：通常无需设置环境变量；若需强制 Wayland 平台插件，可设置 `QT_QPA_PLATFORM=wayland`（具体以发行版与 Qt 插件为准）。
- **X11 会话**：一般使用 `xcb`；可设置 `QT_QPA_PLATFORM=xcb`。
- 嵌入规则：将预览控件作为子控件加入父窗口布局即可；在 Wayland 下请避免对已嵌入的子窗口做违反合成器规则的跨窗口抓取（一般文件预览不涉及）。

## 3. 安装位置（deb 包默认）

多架构 Debian/Ubuntu 常见布局：

- 库文件：`/usr/lib/<triplet>/libukui-previewhub.so*`  
  例如：`/usr/lib/x86_64-linux-gnu/libukui-previewhub.so.1`
- 头文件：`/usr/include/ukui-previewhub/`
- 文档：`/usr/share/doc/libukui-previewhub1/USER_MANUAL.md`

## 4. C++ 集成（推荐）

### 4.1 编译

在 CMake 中：

```cmake
find_package(Qt5 REQUIRED COMPONENTS Widgets)
target_link_libraries(your_app PRIVATE Qt5::Widgets /usr/lib/x86_64-linux-gnu/libukui-previewhub.so)
target_include_directories(your_app PRIVATE /usr/include/ukui-previewhub)
```

或使用 `-lukui-previewhub` 并保证 `LD_LIBRARY_PATH` / 运行链接器缓存已包含安装路径。

### 4.2 头文件

```cpp
#include <ukui_previewhub_api.h>
```

### 4.3 最小流程

1. 在 `main` 中创建 `QApplication`（必须先于任何预览控件）。
2. 调用 `UkuiPreviewHub::createPreviewWidget(parent)` 得到 `QWidget*`。
3. 将返回的指针加入父窗口的布局（例如 `QVBoxLayout::addWidget`）。
4. 选中文件后：
   - `UkuiPreviewHub::preparePreview(widget, QStringLiteral("file:///home/user/a.pdf"));`
   - `UkuiPreviewHub::startPreview(widget);`
5. 切换文件或关闭前可调用 `UkuiPreviewHub::cancelPreview(widget);`。
6. 销毁时 `UkuiPreviewHub::destroyPreviewWidget(widget);` 或随父控件树一并释放。

### 4.4 代码示例

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <ukui_previewhub_api.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QMainWindow win;
    auto *central = new QWidget(&win);
    auto *layout = new QVBoxLayout(central);

    QWidget *preview = UkuiPreviewHub::createPreviewWidget(central);
    layout->addWidget(preview);

    preparePreview(preview, QStringLiteral("file:///tmp/demo.pdf"));
    startPreview(preview);

    win.setCentralWidget(central);
    win.resize(800, 600);
    win.show();
    return app.exec();
}
```

将 `file:///tmp/demo.pdf` 换为实际文件 URI；**仅支持 `file:///` 本地 URI** 时与原版插件行为一致（非 file 协议会显示无法预览提示）。

## 5. C 语言 / `dlopen` 集成

导出符号为 C 接口，便于非 Qt 或动态加载场景（仍建议在显示预览的进程中初始化 Qt GUI）。

```c
#include <dlfcn.h>

typedef void *(*create_fn)(void *);
typedef void (*void_fn)(void *);
typedef void (*prepare_fn)(void *, const char *);

void *lib = dlopen("libukui-previewhub.so.1", RTLD_NOW);
create_fn create = (create_fn)dlsym(lib, "ukui_previewhub_preview_widget_create");
prepare_fn prepare = (prepare_fn)dlsym(lib, "ukui_previewhub_preview_prepare");
void_fn start = (void_fn)dlsym(lib, "ukui_previewhub_preview_start");

void *w = create(parent_native); /* 若无可传 NULL，对应 QWidget* parent */
prepare(w, "file:///path/to/file");
start(w);
```

**注意**：预览控件内部为 Qt 对象，宿主仍需运行 Qt 事件循环（`QApplication::exec()`），否则界面不会刷新。

## 6. API 参考

| 符号 | 说明 |
|------|------|
| `ukui_previewhub_preview_widget_create(void *parent)` | 创建预览控件，返回指针需按 `QWidget*` 使用 |
| `ukui_previewhub_preview_widget_destroy(void *widget)` | 销毁控件 |
| `ukui_previewhub_preview_prepare(void *widget, const char *uri_utf8)` | 设置要预览的 URI |
| `ukui_previewhub_preview_start(void *widget)` | 开始预览 |
| `ukui_previewhub_preview_cancel(void *widget)` | 取消当前预览 |

C++ 内联封装见 `ukui_previewhub_api.h` 中命名空间 `UkuiPreviewHub`。

## 7. 构建与打包 deb

在源码目录 `ukui-previewhub` 下：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

生成 Debian 包时，源码包名为 `ukui-previewhub`，安装生成的二进制包名为 **`libukui-previewhub1`**（含库、头文件与用户手册）。在 `ukui-previewhub` 目录执行（需已安装 `build-essential`、`debhelper`、`cmake`、Qt5 与 KF5 Syntax Highlighting 等开发包，**不需要** `libpeony-dev`）：

```bash
dpkg-buildpackage -us -uc -b
```

## 8. 与原版插件的关系

- 预览页面、类型判断、PDF/Web、多媒体等逻辑与 **复制时的** `peony-preview-file-content-plugin` 源码保持一致；若上游插件更新，需将 `core/` 目录与上游同步后重新编译本库（同步操作在子项目内进行，不修改原插件目录中的文件）。

## 9. 常见问题

**Q：预览空白或 WebEngine 报错？**  
A：确认已安装 `qtwebengine5` 相关运行库，且在沙箱/容器内具备渲染所需权限。

**Q：Office 文档无法预览？**  
A：确认已安装 LibreOffice，且 `libreoffice` 在 `PATH` 中（与原版 `preview-file-type` 检测逻辑一致）。

**Q：同目录重命名文件后预览不跟随？**  
A：本地 `file://` 文件在同目录下改名时，库会尝试通过 inode 匹配新路径并刷新；若移动到其它目录或跨文件系统，请由宿主重新调用 `prepare` / `start`。

---

文档路径随安装前缀变化；若从源码阅读，见仓库内 `docs/USER_MANUAL.md`。
