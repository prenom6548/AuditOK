# Hermes Agent 的 OpenAI 兼容 API Server

## 动机

主流聊天前端通常都支持 OpenAI 兼容 REST API 和 SSE 流式响应，例如 Open WebUI、
LobeChat、LibreChat、AnythingLLM、NextChat、ChatBox、Jan、HF Chat-UI、
big-AGI 等。Hermes Agent 暴露 OpenAI 兼容接口后，这些前端无需定制 adapter
即可把 Hermes Agent 当作后端使用。

## 启用后的效果

```text
┌──────────────────┐
│  Open WebUI      │──┐
│  LobeChat        │  │    POST /v1/chat/completions
│  LibreChat       │  ├──► Authorization: Bearer <key>     ┌─────────────────┐
│  AnythingLLM     │  │    {"messages": [...]}             │  agent-runtime   │
│  NextChat        │  │                                    │  gateway        │
│  Any OAI client  │──┘    ◄── SSE streaming response      │  (API server)   │
└──────────────────┘                                        └─────────────────┘
```

典型使用流程：

1. 在 `~/.kylin-agent-runtime/.env` 中设置 `API_SERVER_ENABLED=true`。
2. 运行 `kylin-agent-runtime gateway`，API server 随 Telegram、Discord 等平台一起启动。
3. 将 Open WebUI 或任意 OpenAI 兼容前端指向 `http://localhost:8642/v1`。
4. 通过这些前端与 Hermes Agent 对话。

## 端点

| 方法 | 路径 | 作用 |
|---|---|---|
| `POST` | `/v1/chat/completions` | 与 agent 对话，支持流式和非流式 |
| `GET` | `/v1/models` | 列出可用 “model”，通常返回 `agent-runtime` |
| `GET` | `/health` | 健康检查 |

后续实现中还可以扩展：

- `/v1/responses`
- `/v1/runs`
- `/v1/runs/{run_id}/events`
- `/v1/capabilities`

## 架构方案

### 方案 A：Gateway Platform Adapter

推荐新建：

```text
gateway/platforms/api_server.py
```

并让它继承 `BasePlatformAdapter`。这是更干净的方案，因为它可以复用 gateway
已有能力：

- 会话管理
- 认证
- 上下文构建
- interrupt 支持
- 会话持久化
- 与 Telegram、Discord 等平台一致的接入模式
- 复用已有 `aiohttp.web`

adapter 在 `connect()` 中启动 `aiohttp.web.Application`，并将 HTTP 请求路由到
标准 `handle_message()` 管线。

### 方案 B：独立组件

也可以在 `gateway/api_server.py` 中实现独立 HTTP server，直接创建 `AIAgent`
实例。这个方案更简单，但会重复会话和鉴权逻辑，维护成本更高。

推荐使用方案 A。

## Chat Completions 请求格式

非流式请求：

```http
POST /v1/chat/completions
Authorization: Bearer hermes-api-key-here
Content-Type: application/json
```

```json
{
  "model": "agent-runtime",
  "messages": [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content": "What files are in the current directory?"}
  ],
  "stream": false,
  "temperature": 0.7
}
```

响应：

```json
{
  "id": "chatcmpl-abc123",
  "object": "chat.completion",
  "created": 1710000000,
  "model": "agent-runtime",
  "choices": [{
    "index": 0,
    "message": {
      "role": "assistant",
      "content": "Here are the files in the current directory:\n..."
    },
    "finish_reason": "stop"
  }],
  "usage": {
    "prompt_tokens": 50,
    "completion_tokens": 200,
    "total_tokens": 250
  }
}
```

## 流式响应

当请求包含：

```json
{"stream": true}
```

server 应返回 `text/event-stream`，并按 OpenAI Chat Completions 的 chunk 格式
输出：

```text
data: {"id":"chatcmpl-abc123","object":"chat.completion.chunk","choices":[{"delta":{"content":"Hello"}}]}

data: {"id":"chatcmpl-abc123","object":"chat.completion.chunk","choices":[{"delta":{"content":" world"}}]}

data: [DONE]
```

## 认证

建议通过环境变量或 `config.yaml` 设置 API key。请求需要：

```http
Authorization: Bearer <key>
```

如果未配置 key，可以只允许本机访问，或在开发模式下放开认证。生产环境不建议无
认证暴露到公网。

## 会话连续性

OpenAI Chat Completions 本身偏无状态，但 Hermes Agent 需要会话连续性。可以使用：

- `X-Hermes-Session-Id`：短期 transcript 连续性
- `X-Hermes-Session-Key`：长期记忆或平台通道作用域

这样前端可以控制是否复用会话，也不会把不同用户的上下文混在一起。

## 错误格式

建议返回 OpenAI 风格错误：

```json
{
  "error": {
    "message": "Invalid API key",
    "type": "authentication_error",
    "code": "invalid_api_key"
  }
}
```

常见状态码：

- `400`：请求格式错误
- `401`：认证失败
- `404`：端点或资源不存在
- `429`：并发或速率限制
- `500`：内部错误

## 实现建议

1. 新增 `api_server` platform adapter。
2. 使用 `aiohttp.web` 注册 `/v1/chat/completions`、`/v1/models`、`/health`。
3. 解析 OpenAI messages，转换为 Hermes Agent 可使用的文本输入。
4. 非流式请求等待 agent 完成后返回完整响应。
5. 流式请求将 agent token/chunk 写成 SSE。
6. 将会话写入 `state.db`，保证 `hermes sessions list` 可见。
7. 增加 API key 鉴权、CORS 和安全响应头。
8. 添加测试：非流式、流式、认证失败、模型列表、健康检查。

## 对 KylinAgent 的意义

KylinAgent 可以优先使用 OpenAI 兼容接口作为远程后端协议。这样桌面端无需直接访问
远程 `~/.kylin-agent-runtime` 文件，也不需要理解后端内部 Python 对象，只需要通过 HTTP API：

- 发送消息
- 接收 SSE
- 查询模型
- 获取能力描述
- 管理会话和配置

后续如果补齐 `/api/sessions`、`/api/config/models` 等端点，KylinAgent 就能完整
支持远程 Hermes Agent。
