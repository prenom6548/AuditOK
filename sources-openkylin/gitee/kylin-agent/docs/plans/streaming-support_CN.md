# Hermes Agent 流式 LLM 响应支持

## 概述

目标是在所有平台上支持逐 token 或逐 chunk 输出 LLM 响应。开启后，用户可以看到
回复实时打字，而不是等待整段生成完成。流式能力通过配置开关启用，默认关闭，
现有非流式路径保持不变。

## 设计原则

1. **特性开关控制**：通过 `streaming.enabled: true` 启用，默认关闭。
2. **基于 callback**：向 `AIAgent` 注入简单的 `stream_callback(text_delta)`。
   agent 不关心消费者如何处理 token。
3. **优雅降级**：如果 provider 不支持 streaming，或 streaming 失败，则回退到
   非流式路径。
4. **核心平台无关**：CLI、Telegram、Discord、API Server 都使用同一套 agent
   streaming 机制，只是消费方式不同。

## 架构

```text
                              stream_callback(delta)
                                    │
  ┌─────────────┐    ┌─────────────▼──────────────┐
  │  LLM API    │    │      queue.Queue()          │
  │  (stream)   │───►│  agent 线程与消费者之间的桥接 │
  └─────────────┘    └─────────────┬──────────────┘
                                   │
                    ┌──────────────┼──────────────┐
                    │              │              │
              ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐
              │    CLI     │ │  Gateway  │ │ API Server│
              │ 终端打印   │ │ 编辑消息  │ │ SSE event │
              └───────────┘ └───────────┘ └───────────┘
```

agent 在线程中运行，callback 将 token 放入线程安全队列。不同消费者在自己的上下文
中读取队列，例如 async task、主线程或 SSE 写入协程。

## 配置

### config.yaml

```yaml
streaming:
  enabled: false
  # 可选平台级覆盖：
  # cli: true
  # telegram: true
  # discord: false
  # api_server: true
```

### 环境变量

```text
HERMES_STREAMING_ENABLED=true
```

### 优先级

1. API Server 中，请求体 `stream` 字段优先级最高。
2. 平台级配置优先于全局配置。
3. 环境变量可作为全局开关。
4. 默认关闭。

## AIAgent 接口

建议在 `AIAgent` 初始化参数中加入：

```python
stream_callback: Optional[Callable[[str], None]] = None
```

当 provider 返回 delta 时调用：

```python
if self.stream_callback:
    self.stream_callback(delta_text)
```

agent 只负责产出 delta，不负责 UI 更新、消息编辑或 SSE 协议细节。

## Provider 适配

不同 provider 的 streaming 格式不同，但都应规范化为文本 delta：

- OpenAI Chat Completions：读取 `choices[0].delta.content`
- Anthropic Messages：读取 `content_block_delta`
- OpenRouter：通常兼容 OpenAI chunk
- 本地 OpenAI 兼容服务：按 OpenAI 兼容 chunk 处理

如果某 provider 不支持 streaming：

1. 记录 debug/warning。
2. 继续走非流式请求。
3. 不影响用户完成任务。

## CLI 消费方式

CLI 可以直接把 delta 打印到终端：

```python
def stream_callback(delta: str):
    print(delta, end="", flush=True)
```

最终仍需要保存完整 assistant 消息到会话历史和 `state.db`。

## Gateway 消费方式

Telegram、Discord、Slack 等平台不适合每个 token 发送一条消息。建议采用节流编辑：

- 收到 delta 后追加到 buffer。
- 每隔固定时间编辑一次消息，例如 0.5 到 1 秒。
- 达到平台最大消息长度时拆分。
- 最终完成后再做一次完整更新。

注意不同平台限制：

- Telegram 支持编辑消息，但有频率限制。
- Discord 支持编辑消息，但也有 rate limit。
- SMS/email 不适合流式，可保持非流式。

## API Server 消费方式

OpenAI 兼容 API Server 应使用 SSE：

```text
Content-Type: text/event-stream
Cache-Control: no-cache
Connection: keep-alive
```

每个 delta 输出一个 chunk：

```text
data: {"choices":[{"delta":{"content":"hello"}}]}

```

结束时输出：

```text
data: [DONE]
```

这使 Open WebUI、LobeChat、LibreChat 等前端可以直接显示实时回复。

## 错误处理

流式过程中可能出现：

- provider 中断
- 网络断开
- 客户端取消请求
- 平台编辑失败
- agent 工具调用报错

建议策略：

1. 已产生的内容保留。
2. 尽量发送最终错误提示或 event。
3. 会话状态标记为结束或 interrupted。
4. 清理后台任务、队列和正在编辑的消息。
5. 不重复写入同一 assistant 内容。

## 持久化

无论是否 streaming，都应保证最终完整消息写入 `state.db`。推荐：

- delta 只用于 UI 实时显示。
- 完整 assistant 内容在结束时统一持久化。
- 如果中途异常，可保存已生成内容并标记 finish reason。

## 测试建议

需要覆盖：

- streaming disabled 时行为不变
- OpenAI 风格 streaming
- Anthropic 风格 streaming
- provider 不支持 streaming 时回退
- API Server `stream=true`
- API Server 客户端中途断开
- Gateway 节流编辑
- 长消息拆分
- 错误时不重复写 DB

## 对 KylinAgent 的意义

KylinAgent 的聊天窗口已经支持 SSE/流式追加消息。后端统一 streaming 机制后，
桌面端可以：

- 对本机后端和远程后端使用同一流式协议
- 在收到 delta 时实时追加 assistant 气泡
- 在完成事件后切回发送按钮
- 避免阻塞 UI
- 与 OpenAI 兼容前端共享同一后端能力

后续建议桌面端优先适配 `/v1/chat/completions` 的 SSE 格式，同时保留现有
Hermes segment 格式兼容层。
