# ukui-previewhub

可嵌入的 Qt5 文件预览动态库（`libukui-previewhub`），与 Peony 无依赖关系。

## 宿主应用适配（最小示例）

1. 先构建并安装库，或在本仓库中先执行 `cmake -S . -B build && cmake --build build` 得到 `build/libukui-previewhub.so`。
2. 包含头文件 `#include <ukui_previewhub_api.h>`，链接 `ukui-previewhub`（并保证运行时能找到该 `.so`）。
3. 在 `QApplication` 创建之后：用 `UkuiPreviewHub::createPreviewWidget(parent)` 创建预览控件，加入布局；对选中文件调用 `UkuiPreviewHub::preparePreview(widget, uri)` 与 `UkuiPreviewHub::startPreview(widget)`；切换或退出时调用 `UkuiPreviewHub::cancelPreview(widget)`，必要时 `destroyPreviewWidget`。

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <ukui_previewhub_api.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QString uri = QStringLiteral("file:///tmp/demo.pdf");
    if (argc > 1)
        uri = QUrl::fromLocalFile(QString::fromUtf8(argv[1])).toString();

    QMainWindow win;
    auto *central = new QWidget(&win);
    auto *layout = new QVBoxLayout(central);

    QWidget *preview = UkuiPreviewHub::createPreviewWidget(central);
    layout->addWidget(preview);

    UkuiPreviewHub::preparePreview(preview, uri);
    UkuiPreviewHub::startPreview(preview);

    win.setCentralWidget(central);
    win.resize(800, 600);
    win.show();
    return app.exec();
}
```

更完整的依赖说明、安装路径与 `dlopen` 用法见 `docs/USER_MANUAL.md`。

## 自测程序（不随 deb 安装）

独立工程位于 `test/`，**不包含**在根目录 `CMakeLists.txt` 中。用于本地编译验证。

**已将库通过 deb 安装到系统**（`/usr/lib/<triplet>/libukui-previewhub.so*`，头文件在 `/usr/include/ukui-previewhub/`）时，一般只需在 `test/` 下配置并编译（CMake 会在系统默认路径中查找库；头文件在 `/usr/include` 下查找 `ukui-previewhub` 子目录）：

```bash
cd ukui-previewhub/test
cmake -S . -B build
cmake --build build
./build/ukui-previewhub-test /path/to/local/file.pdf
```

若尚未安装、仍使用源码树内刚编出的 `.so`，从 `ukui-previewhub` 根目录执行并指定库所在目录（同时设置 `BUILD_RPATH` 便于未 `ldconfig` 时运行）：

```bash
cd ukui-previewhub
cmake -S . -B build && cmake --build build
cmake -S test -B test/build -DUKUI_PREVIEWHUB_BUILD_DIR="$PWD/build"
cmake --build test/build
./test/build/ukui-previewhub-test /path/to/local/file.pdf
```

测试程序链接时仍需本机安装 **Qt5 开发包**（如 `qtbase5-dev`）；运行时由已安装的 `libukui-previewhub` 及其传递依赖（WebEngine、KF5 Syntax Highlighting 等）满足。
