# KylinAgent 与 Hermes Agent 后端对接说明

本文根据父目录 `../hermes-agent` 的代码和文档整理，面向 KylinAgent
桌面端维护者，说明当前 Qt 前端与 Hermes Agent 后端如何对接，以及后续支持
远程后端时需要补齐的接口。

## 关键路径

Hermes Agent 使用 `HERMES_HOME` 作为运行数据根目录。未设置时默认是
`~/.hermes`。

常用文件：

| 文件 | 作用 |
|---|---|
| `config.yaml` | 模型、提供商、平台、工具、网络等用户配置 |
| `.env` | API key、token 等敏感配置 |
| `state.db` | 会话和消息的 SQLite 主存储 |
| `SOUL.md` | 智能体个性 |
| `MEMORY.md` | 智能体长期记忆 |
| `USER.md` | 用户画像 |
| `skills/` | 技能目录 |
| `logs/` | `agent.log`、`errors.log`、`gateway.log` 等日志 |

后端代码中的关键来源：

- `hermes_constants.py`：`get_hermes_home()`、`get_config_path()` 等路径入口
- `hermes_cli/config.py`：`config.yaml` 读写、合并默认配置、YAML 解析错误告警
- `hermes_state.py`：`state.db` schema、WAL、FTS5、会话和消息持久化
- `gateway/platforms/api_server.py`：OpenAI 兼容 HTTP API Server
- `agent/model_metadata.py`：模型元数据、上下文长度、OpenAI 兼容 `/models` 探测

## 当前 HTTP API Server

父项目 `gateway/platforms/api_server.py` 暴露的是 OpenAI 兼容 API，默认监听：

```text
http://127.0.0.1:8642
```

主要端点：

| 方法 | 路径 | 说明 |
|---|---|---|
| `GET` | `/health` | 健康检查 |
| `GET` | `/health/detailed` | 更详细的运行状态 |
| `GET` | `/v1/models` | 返回 API Server 对外暴露的模型名 |
| `GET` | `/v1/capabilities` | 机器可读能力描述 |
| `POST` | `/v1/chat/completions` | OpenAI Chat Completions 格式 |
| `POST` | `/v1/responses` | OpenAI Responses API 格式 |
| `GET` | `/v1/responses/{response_id}` | 读取已存储 response |
| `DELETE` | `/v1/responses/{response_id}` | 删除 response |
| `POST` | `/v1/runs` | 启动结构化 agent run |
| `GET` | `/v1/runs/{run_id}` | 获取 run 状态 |
| `GET` | `/v1/runs/{run_id}/events` | SSE 事件流 |
| `POST` | `/v1/runs/{run_id}/approval` | 处理审批 |
| `POST` | `/v1/runs/{run_id}/stop` | 停止 run |
| `GET/POST/PATCH/DELETE` | `/api/jobs...` | cron job 管理 |

`/v1/capabilities` 会声明 `X-Hermes-Session-Id` 和
`X-Hermes-Session-Key`。前者用于短期会话连续性，后者用于长期记忆或平台通道
级别的稳定作用域。

## 会话与消息存储

后端主存储是 `state.db`。核心表来自 `hermes_state.py`：

- `sessions`
  - `id`
  - `source`
  - `user_id`
  - `model`
  - `model_config`
  - `system_prompt`
  - `parent_session_id`
  - `started_at`
  - `ended_at`
  - `message_count`
  - `title`
  - token、费用、handoff 等统计字段
- `messages`
  - `id`
  - `session_id`
  - `role`
  - `content`
  - `tool_calls`
  - `tool_name`
  - `timestamp`
  - `reasoning_content`
  - `platform_message_id`

后端还启用了 `messages_fts` FTS5 全文索引。

桌面端如果连接本机后端，可以继续读本机 `~/.hermes/state.db`。如果连接远程
后端，不应该尝试直接读取远程机器的 home 目录或 SQLite 文件。正确方式是新增
后端 HTTP API，由后端读取自己的 `state.db` 并返回 JSON。

建议补齐的远程会话 API：

| 方法 | 路径 | 说明 |
|---|---|---|
| `GET` | `/api/sessions` | 会话列表，支持分页、搜索、来源过滤 |
| `POST` | `/api/sessions` | 创建桌面会话 |
| `GET` | `/api/sessions/{id}` | 会话详情 |
| `PATCH` | `/api/sessions/{id}` | 更新标题、模型等元数据 |
| `DELETE` | `/api/sessions/{id}` | 删除会话 |
| `GET` | `/api/sessions/{id}/messages` | 读取消息 |
| `POST` | `/api/sessions/{id}/messages` | 追加本地生成或导入消息 |

## 模型配置

后端模型配置来源主要是 `config.yaml` 和 `.env`。本项目设置页目前会写入
`~/.hermes/models.json`，并同步生成 `config.yaml` 的模型/provider 配置。

注意事项：

- Ollama、LM Studio、vLLM、llama.cpp 等本地 OpenAI 兼容服务通常需要
  `base_url`。
- `agent/model_metadata.py` 可以通过 OpenAI 兼容 `/models` 或 LM Studio
  `/api/v1/models` 探测模型元数据。
- Hermes Agent 对 agent 模型有最低上下文要求，低于 64K 的模型可能被拒绝。
  如果模型具备更长上下文但接口无法自动发现，应在 `config.yaml` 中显式设置
  `model.context_length` 或对应 provider/model 配置。

建议补齐的远程模型 API：

| 方法 | 路径 | 说明 |
|---|---|---|
| `GET` | `/api/config/models` | 返回已配置模型，不自动混入后端 URL 的全部模型 |
| `POST` | `/api/config/models` | 新增模型并写入 `config.yaml` |
| `PATCH` | `/api/config/models/{id}` | 修改模型 |
| `DELETE` | `/api/config/models/{id}` | 删除模型 |
| `POST` | `/api/config/models/probe` | 按 `base_url` 探测模型和上下文信息 |

## 技能、个性、记忆、日志

这些页面目前适合本机模式，因为它们直接读写 `HERMES_HOME` 下的文件。

远程模式建议新增接口：

| 模块 | 建议接口 |
|---|---|
| 技能 | `/api/skills`、`/api/skills/import`、`/api/skills/{id}` |
| 个性 | `/api/persona` 读取/保存 `SOUL.md` |
| 记忆 | `/api/memory` 读取/保存 `MEMORY.md`、`USER.md` |
| 日志 | `/api/logs`、`/api/logs/{name}` |
| 用量 | `/api/usage`，从 `state.db` 统计 token 和费用 |

## 本机模式与远程模式边界

当前 KylinAgent 已支持在通用设置里配置后端服务地址。这个地址适用于走
`ApiService` 的 HTTP 调用。

仍需注意：

- HTTP 调用可以切换到远程 IP。
- 直接读写 `~/.hermes` 的页面仍然是本机数据。
- 远程后端的配置、技能、会话历史、记忆和日志必须通过后端 API 获取。
- 不建议用 SSH/SFTP/共享目录直接操作远程 `~/.hermes`，因为权限、安全、
  文件锁、SQLite WAL 和并发写入都容易出问题。

## 推荐演进路线

1. 将聊天发送统一到 OpenAI 兼容 `/v1/chat/completions` 或 `/v1/runs`。
2. 在 Hermes Agent API Server 中增加 `/api/sessions` 系列端点。
3. 在 Qt 前端中把会话列表和消息加载改成优先走 HTTP API。
4. 增加 `/api/config/models`，让模型设置页不再直接写远程文件。
5. 再逐步迁移技能、个性、记忆、日志、用量页面。

这样可以让 KylinAgent 同时支持：

- 本机后端：直接使用本机 `~/.hermes`
- 远程后端：所有状态由远程 Hermes Agent API 提供
