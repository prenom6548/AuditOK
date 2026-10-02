# 添加新的消息平台

Kylin Agent Runtime Gateway 有两种添加平台的方式：

1. 插件方式：推荐给社区和第三方集成。
2. 内置方式：仅建议核心贡献者使用。

## 插件方式

在 `~/.kylin-agent-runtime/plugins/` 创建插件目录，或在仓库内 `plugins/platforms/`
放置内置插件。插件通常包含：

- `plugin.yaml`
- `adapter.py`

`adapter.py` 继承 `BasePlatformAdapter`，并在 `register(ctx)` 入口中通过
`ctx.register_platform()` 注册平台。这样无需修改 Hermes 核心代码。

插件系统会自动处理：

- adapter 创建
- 配置解析
- 用户授权
- cron 投递
- `send_message` 路由
- system prompt 平台提示
- 状态展示
- gateway setup

### 常用可选 hook

| hook | 作用 |
|---|---|
| `env_enablement_fn` | 在 adapter 创建前从环境变量构造 `PlatformConfig.extra` 和可选 `home_channel` |
| `apply_yaml_config_fn` | 将 `config.yaml` 中的平台配置转换为环境变量或 `PlatformConfig.extra` |
| `cron_deliver_env_var` | 指定 `*_HOME_CHANNEL` 环境变量，让 cron `deliver=<name>` 可路由到该平台 |
| `standalone_sender_fn` | 支持 gateway 外部的 cron 进程独立发送消息 |
| `plugin.yaml requires_env/optional_env` | 让 setup wizard 展示变量说明、输入提示、密码标记和 URL |

如果平台存在特殊交互限制，例如 LINE 的 60 秒 reply token 或 WhatsApp 的 24 小时
会话窗口，adapter 可以覆写 `_keep_typing` 来加入平台特定 UX。覆写时应先
`await super()._keep_typing(...)`，并在 `finally` 中清理后台任务。

参考实现：

- `plugins/platforms/irc/`
- `plugins/platforms/teams/`
- `plugins/platforms/google_chat/`
- `plugins/platforms/line/`

## 内置方式

内置平台需要修改 Hermes 核心代码，适合维护者实现官方平台适配器。以下清单是
实际集成点，缺失任意一项都可能导致功能不完整或行为不一致。

## 1. 核心 Adapter

文件位置：

```text
gateway/platforms/<platform>.py
```

adapter 应继承：

```python
BasePlatformAdapter
```

### 必需方法

| 方法 | 作用 |
|---|---|
| `__init__(self, config)` | 解析配置、初始化状态，并调用 `super().__init__(config, Platform.YOUR_PLATFORM)` |
| `connect() -> bool` | 连接平台并启动监听，成功返回 `True` |
| `disconnect()` | 停止监听、关闭连接、取消任务 |
| `send(chat_id, text, ...) -> SendResult` | 发送文本消息 |
| `send_typing(chat_id)` | 发送正在输入状态 |
| `send_image(chat_id, image_url, caption) -> SendResult` | 发送图片 |
| `get_chat_info(chat_id) -> dict` | 返回 `{name, type, chat_id}` |

### 可选方法

| 方法 | 作用 |
|---|---|
| `send_document(chat_id, path, caption)` | 发送文件 |
| `send_voice(chat_id, path)` | 发送语音 |
| `send_video(chat_id, path, caption)` | 发送视频 |
| `send_animation(chat_id, path, caption)` | 发送 GIF/动画 |
| `send_image_file(chat_id, path, caption)` | 发送本地图片 |

### 必需函数

```python
def check_<platform>_requirements() -> bool:
    """检查平台依赖是否可用。"""
```

### 关键模式

- 使用 `self.build_source(...)` 构造 `SessionSource`
- 使用 `self.handle_message(event)` 将入站消息派发到 gateway
- 使用 base 中的 `MessageEvent`、`MessageType`、`SendResult`
- 使用 `cache_image_from_bytes`、`cache_audio_from_bytes`、
  `cache_document_from_bytes` 缓存附件
- 过滤机器人自身消息，避免自我回复循环
- 过滤平台同步/echo 消息
- 日志中脱敏手机号、token 等敏感标识
- 使用指数退避和 jitter 实现重连
- 如果平台有消息长度限制，设置 `MAX_MESSAGE_LENGTH`

## 2. Platform 枚举

在 `gateway/config.py` 中向 `Platform` enum 添加平台名。

## 3. 配置加载

需要让平台配置能从环境变量和 `config.yaml` 加载，并正确合并优先级。

推荐规则：

1. 显式环境变量优先。
2. `config.yaml` 作为可持久配置。
3. 默认值只用于未配置时。

## 4. Gateway 注册

需要将 adapter 注册到 gateway 的平台注册表中，使其能被：

- `kylin-agent-runtime gateway`
- `kylin-agent-runtime gateway status`
- cron delivery
- setup wizard
- connected platform list

识别和使用。

## 5. 消息事件

入站消息应统一转换为 `MessageEvent`，至少包含：

- 平台 source
- chat id
- user id
- 文本内容
- 消息类型
- 平台 message id
- 附件信息

这样 agent、记忆、会话和审计日志才能使用统一数据模型。

## 6. 发送能力

adapter 应实现文本发送，并尽量实现附件发送。返回值应使用 `SendResult`，包含：

- `ok`
- `platform_message_id`
- `error`
- 其它平台返回信息

## 7. Cron 投递

如需支持：

```bash
hermes cron create ... --deliver <platform>
```

需要配置 home channel，并让 standalone cron 进程也能发送。插件方式建议使用
`cron_deliver_env_var` 和 `standalone_sender_fn`。

## 8. 安全与可靠性

平台适配器应特别注意：

- 不在日志中打印 token、secret、手机号、邮箱等敏感信息
- 对 webhook 使用签名或 token 校验
- 对下载附件做大小限制和类型检查
- 对网络错误做重试和退避
- 对长任务保持 typing/heartbeat
- 对 `/stop`、断连、进程退出做清理

## 9. 测试建议

建议覆盖：

- 配置解析
- 环境变量优先级
- 入站消息转换
- 文本发送
- 附件发送
- cron delivery
- 断线重连
- 自消息过滤
- webhook 鉴权失败

## 10. 文档

新增平台后应更新：

- README 或 docs 中的平台列表
- setup wizard 说明
- 环境变量说明
- `config.yaml` 示例
- 故障排查说明

对于社区平台，优先走插件方式；只有需要进入官方维护范围、且确实需要核心代码支持
时，才采用内置方式。
