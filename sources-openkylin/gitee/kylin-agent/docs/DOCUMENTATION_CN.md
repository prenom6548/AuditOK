# Kylin Agent 技术概览

本文总结 Kylin Agent 桌面端的目标架构、运行边界和后续维护要点。

## 架构

Kylin Agent 是基于 Qt5 C++ 的桌面客户端，通过本机或远程 HTTP API 与
Hermes Agent 兼容后端通信。

应用按三层组织：

```text
UI 层
  MainWindow, Sidebar, ChatWidget, SettingsDialog, 会话与设置类 Widget

服务层
  ApiService, DatabaseService, SessionService, ModelService, 配置辅助模块

模型层
  SavedModel, ModelProvider, SessionMessage, SessionSummary, SearchResult
```

UI 层负责交互状态，把网络、数据库和长耗时操作交给服务层。服务层屏蔽持久化、
HTTP 流式响应、模型配置和会话操作细节。模型层用于明确 UI、存储和后端 API
之间的数据契约。

## 运行流程

1. 应用启动 `QApplication` 封装并创建主窗口。
2. 主窗口构建无边框窗口、侧边栏、聊天视图和设置入口。
3. 本机模式下，从本地存储加载会话状态。
4. 聊天请求通过 API 服务发送到 Hermes 兼容端点。
5. 流式响应更新当前聊天视图，并携带消息元数据持久化。

## 核心功能

- 通过 Server-Sent Events 处理流式聊天。
- 支持会话创建、搜索、重命名、删除和每会话 UI 状态。
- 支持多个模型提供商，包括 OpenAI 兼容的本地服务。
- 使用本地 SQLite 持久化会话和消息。
- 提供配置文件、模型、技能、个性、记忆、工具、计划和通用设置页面。
- 使用无边框桌面窗口、自定义标题栏和侧边栏控制。

## API 边界

桌面端应把 HTTP API 作为生成请求和远程状态的边界。只有在本机模式下，才适合
直接访问 `~/.hermes` 下的文件。

期望的后端能力包括：

- 通过 `/health` 做健康检查。
- 通过 `/v1/models` 发现模型。
- 通过 `/v1/chat/completions` 或结构化 run 处理流式聊天。
- 通过 `/v1/capabilities` 做可选能力发现。

远程模式不应直接读取远程 SQLite 数据库。会话、消息、模型、技能、个性、记忆、
日志和用量数据应由明确的后端 API 提供。

## 存储

本机模式使用 Hermes 数据目录保存助手状态：

| 路径 | 用途 |
|---|---|
| `~/.hermes/state.db` | 会话和消息存储 |
| `~/.hermes/SOUL.md` | 个性文本 |
| `~/.hermes/MEMORY.md` | 长期记忆 |
| `~/.hermes/USER.md` | 用户画像 |
| `~/.hermes/skills/` | 本地技能 |

Kylin Agent 的桌面端模型配置保存在：

```text
~/.config/kylin-agent/models.json
```

设置了 `$HERMES_HOME` 时，桌面端应优先使用该目录定位 Hermes 运行文件。

## 实现注意事项

- 优先使用 Qt 信号和槽处理跨 Widget 通信。
- Widget 需要跨进程、网络或数据库边界时，应通过服务层完成。
- 切换会话时保留草稿、滚动位置和流式状态。
- API key 和 provider 密钥不要写入项目文件。
- 后续远程模式能力应收敛在服务接口之后，让本机与远程数据源可以并存。
