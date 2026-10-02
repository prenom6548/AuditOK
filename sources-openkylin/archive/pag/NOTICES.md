# 第三方组件许可证声明（Notices）

本文档汇总了项目中所有第三方组件的许可证信息，以满足各开源协议的声明要求。

---

## 一、本项目主体组件

| 子目录 | 组件名称 | 版权方 | 许可证 | 许可证文件 |
|--------|----------|--------|--------|-----------|
| `ffavc/` | ffavc（FFmpeg AVC 解码器封装） | 原作者 | **GNU LGPL 2.1** | `ffavc/LICENSE.txt` |
| `libpag/` | libpag（PAG 动效渲染库） | Tencent | Apache License 2.0 | `libpag/LICENSE.txt` |
| `libpag/third_party/tgfx/` | tgfx（2D 图形底层库） | Tencent | BSD 3-Clause | `libpag/third_party/tgfx/LICENSE.txt` |
| `libpag/third_party/libavc/` | libavc（H.264 解码器） | Android Open Source Project | Apache License 2.0 | `libpag/third_party/libavc/NOTICE` |

---

## 二、tgfx 内嵌第三方组件

以下组件静态编译进 `libtgfx.a`，许可证原文已保留在对应子目录中：

| 子目录 | 组件名称 | 版权方 | 许可证 | 许可证文件 |
|--------|----------|--------|--------|-----------|
| `tgfx/third_party/concurrentqueue/` | concurrentqueue（无锁并发队列） | Cameron Desrochers | Simplified BSD（Boost Software License） | `…/concurrentqueue/LICENSE.md` |
| `tgfx/third_party/digestpp/` | digestpp（哈希算法库） | 原作者 | Public Domain（Unlicense） | `…/digestpp/LICENSE` |
| `tgfx/third_party/highway/` | highway（SIMD 抽象库） | The Highway Project Authors | Apache License 2.0 + BSD 3-Clause | `…/highway/LICENSE`，`…/highway/LICENSE-BSD3` |
| `tgfx/third_party/json/` | nlohmann/json（JSON 解析库） | Niels Lohmann | MIT License | `…/json/LICENSE.MIT` |
| `tgfx/third_party/pathkit/` | pathkit（路径操作库，源自 Skia） | Google Inc. | BSD 3-Clause | `…/pathkit/LICENSE` |
| `tgfx/third_party/skcms/` | skcms（色彩管理，源自 Skia） | Google Inc. | BSD-style（Skia License） | 见 `skcms.h` 文件头部声明 |
| `tgfx/third_party/vendor_tools/` | vendor_tools（构建辅助工具） | Dom Chen | MIT License | `…/vendor_tools/LICENSE` |
| `libpag/third_party/vendor_tools/` | vendor_tools（同上） | Dom Chen | MIT License | `…/vendor_tools/LICENSE` |

---

## 三、关键法律合规说明

### 3.1 ffavc — LGPL 2.1

`ffavc` 组件遵循 **GNU 较宽松公共许可证第 2.1 版（LGPL v2.1）**。

合规要求及现状：

- **动态链接合规**：`libffavc.so` 以独立共享库形式分发（Debian 包 `libffavc`），`libpag.so` 通过动态链接使用它。动态链接是 LGPL 明确允许的使用方式，**无需开放 libpag 自身源码**。
- **源码可获取性**：本仓库保留了 ffavc 完整源码，用户可自行重新编译替换 `libffavc.so`，满足 LGPL 对"允许用户替换库"的要求。
- **版权声明保留**：`ffavc/LICENSE.txt` 已随源码保留，Debian 包安装时 `copyright` 文件同步声明该许可证。

### 3.2 libpag / tgfx — Apache 2.0 / BSD 3-Clause

- 两者均为宽松式许可证，内部使用和分发均无特殊限制。
- 须保留原始版权声明和许可证文件（已保留在 `libpag/LICENSE.txt` 和 `libpag/third_party/tgfx/LICENSE.txt`）。
- Apache 2.0 要求在分发时附带 `NOTICE` 文件中的声明（`libpag/LICENSE.txt` 中已包含相关声明）。

### 3.3 其余第三方组件

MIT、BSD 3-Clause、Unlicense、Public Domain 等均为宽松许可证，保留原始许可证文件即可满足要求，均已就地保留。

---

## 四、系统动态依赖

构建时通过系统包（`pkg-config`）引入的以下库**不打包进本仓库**，分别遵循各自系统包的许可证：

| 库名 | 用途 | 典型许可证 |
|------|------|-----------|
| `liblz4` | 数据压缩 | BSD 2-Clause |
| `libfreetype` | 字体渲染 | FreeType License（BSD-style） |
| `libharfbuzz` | 字形整形 | MIT |
| `libjpeg` | JPEG 解码 | libjpeg License（BSD-style） |
| `libpng` | PNG 解码 | libpng License（zlib-like） |
| `libwebp` / `libwebpdemux` | WebP 解码 | BSD 3-Clause |
| `Qt5（Core/Gui/Quick）` | GUI 框架 | LGPL 3.0 / GPL 2.0 |

以上库均通过动态链接使用，不影响本仓库代码的许可证状态。

---

*最后更新：2026 年 4 月*
