# KylinAgent

基于 **Qt5 C++** 构建的端侧操作系统智能体桌面应用。

## 项目简介

KylinAgent通过本地 HTTP API 与智能体运行时后端通信，提供实时流式对话、会话管理、模型配置等功能。

## 文档

除本 README 外，项目文档已整理到 `docs/`：

- `docs/INDEX.md`：文档索引
- `docs/DOCUMENTATION_CN.md`：中文完整文档
- `docs/AGENT_RUNTIME_INTEGRATION_CN.md.md`：后端对接说明

## 功能特性

- **实时流式聊天** — 基于 SSE 的流式响应，支持 OpenAI delta 和 segment 格式
- **会话管理** — 创建、搜索、重命名、删除会话，每个会话拥有独立状态
- **多模型支持** — 8 种 AI 提供商：OpenRouter、Anthropic、OpenAI、LM Studio、Ollama、vLLM、llama.cpp、自定义
- **无边框界面** — 自定义标题栏、窗口拖拽、侧边栏切换、固定到边缘模式
- **设置系统** — 8 个类别：配置文件、模型、技能、个性、记忆、工具、计划、通用
- **SQLite 存储** — 本地会话与消息持久化，支持 FTS5 全文搜索
- **会话状态独立** — 每个会话独立的滚动位置、输入内容、流式状态

## 构建依赖

本项目使用 **CMake** 构建，依赖 **Qt5** 以下模块：
`Core`、`Gui`、`Widgets`、`Sql`（含 SQLite 驱动）、`Network`、`Xml`、`Svg`、`WebSockets`、`Multimedia`。

### openKylin
```bash
sudo apt install build-essential cmake pkg-config zlib1g-dev qtbase5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5websockets5-dev qtmultimedia5-dev libkysdk-qtwidgets-dev libgsettings-qt-dev
```

| 软件包 | 提供内容 |
|---|---|
| `build-essential` | C++17 编译器 (g++)、make |
| `cmake` | 构建系统 (≥ 3.10) |
| `pkg-config` | 查找 `kysdk-qtwidgets` 等系统库 |
| `zlib1g-dev` | ZLIB 头文件、库及 `ZLIB::ZLIB` 所需 CMake 配置 |
| `qtbase5-dev` | Qt5 Core、Gui、Widgets、Sql、Network、Xml — 头文件、库及 CMake 配置 |
| `libqt5sql5-sqlite` | Qt SQLite 驱动插件（运行时必需，用于 `state.db`） |
| `libqt5svg5-dev` | Qt5 Svg 模块 — 头文件、库及 CMake 配置 |
| `libqt5websockets5-dev` | Qt5 WebSockets 模块 — 头文件、库及 CMake 配置 |
| `qtmultimedia5-dev` | Qt5 Multimedia 模块 — 头文件、库及 CMake 配置 |
| `libkysdk-qtwidgets-dev` | openKylin/Kylin Qt 控件库开发文件 |
| `libgsettings-qt-dev` | GSettings Qt 支持库开发文件 |

如需使用 `scripts/build_and_package.sh` 构建 Debian 安装包，还需要安装打包工具：

```bash
sudo apt install dpkg-dev dpkg rsync tar
```

| 软件包 | 提供内容 |
|---|---|
| `dpkg-dev` | `dpkg-shlibdeps`，用于生成安装包依赖 |
| `dpkg` | `dpkg`、`dpkg-deb` |
| `rsync` | 打包工作区文件同步 |
| `tar` | 打包流程中的归档处理 |

> **注意**：如果 CMake 提示找不到 `Qt5WebSocketsConfig.cmake`、`Qt5SvgConfig.cmake` 或 `Qt5MultimediaConfig.cmake`，通常就是缺少对应的 `libqt5websockets5-dev`、`libqt5svg5-dev` 或 `qtmultimedia5-dev` 开发包。

## 构建与运行

### CMake
```bash
mkdir build && cd build
cmake ..
make
./kylin-agent
```

> **注意**：需要 Qt 5.12+、CMake 3.10+ 和 C++17 编译器。
