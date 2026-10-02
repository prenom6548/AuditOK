# Kylin Agent Runtime 完整迁移方案

本文档整理 Hermes Agent 运行时目录在同发行版、同架构、同 Python 主版本系统之间迁移时的文件范围、清理原则和校验重点。

## 1. 当前备份结构

`install-dir` 记录的目录为：

```text
~/.local/share/uv
~/.local/lib/python3.12
~/.local/lib/bin
~/.hermes
```

实际备份目录通常对应：

```text
.hermes
uv
python3.12
bin
```

建议按下面关系恢复：

```text
备份 .hermes      -> 目标用户 ~/.hermes
备份 uv           -> 目标用户 ~/.local/share/uv
备份 python3.12   -> 目标用户 ~/.local/lib/python3.12
备份 bin          -> 目标用户 ~/.local/bin
```

`install-dir` 中的 `~/.local/lib/bin` 与实际 pip `--user` 默认脚本目录不一致时，以备份目录内容为准恢复到 `~/.local/bin`。

## 2. 安装产物范围

安装流程主要产生以下内容：

```text
~/.hermes/bin/uv
~/.hermes/bin/uvx
~/.hermes/node
~/.hermes/hermes-agent
~/.hermes/agent-runtime/venv
~/.hermes/config.yaml
~/.hermes/.env
~/.hermes/skills
~/.hermes/cron
~/.hermes/sessions
~/.hermes/logs
~/.local/share/uv/python/...
~/.local/lib/python3.12/site-packages
~/.local/bin
```

CUA 相关安装还可能产生：

```text
~/.local/share/kylin-agent/kylin-cua
~/.cache/ms-playwright
~/.cache/pip
~/.cache/uv
```

如果目标系统允许联网，缓存目录缺失时通常可由安装程序重新下载或重新生成。若目标是离线迁移，应优先补齐这些缓存。

## 3. 推荐迁移范围

同类新系统完整迁移建议包含：

```text
~/.hermes
~/.local/share/uv
~/.local/lib/python3.12
~/.local/bin
~/.cache/uv
~/.cache/pip
~/.cache/ms-playwright
~/.local/share/kylin-agent/kylin-cua
```

其中必须迁移：

```text
~/.hermes
~/.local/share/uv
```

建议迁移，用于减少 CUA 依赖重装：

```text
~/.local/lib/python3.12
~/.local/bin
~/.cache/pip
~/.local/share/kylin-agent/kylin-cua
```

建议迁移，用于减少 Playwright 浏览器下载：

```text
~/.cache/ms-playwright
```

建议迁移，用于减少 uv 依赖下载：

```text
~/.cache/uv
```

## 4. 不应直接迁移的内容

以下内容包含旧用户路径、旧用户配置、旧进程状态、临时数据库状态、旧会话历史、旧任务状态或生成缓存。若目标是接近首次安装完成的初始状态，应删除后让安装程序和首次运行重新创建：

```text
~/.hermes/agent-runtime/venv
~/.hermes/cron
~/.hermes/sessions
~/.hermes/logs
~/.hermes/pairing
~/.hermes/hooks
~/.hermes/image_cache
~/.hermes/audio_cache
~/.hermes/memories
~/.hermes/kylin-agent/message-details
~/.hermes/state.db
~/.hermes/response_store.db
~/.hermes/kanban.db
~/.hermes/desktop/sessions.json
~/.hermes/channel_directory.json
~/.hermes/cache-installed
~/.hermes/.tirith-install-failed
~/.hermes/config.yaml.bak-*
~/.hermes/.env
~/.hermes/config.yaml
~/.hermes/models.json
~/.hermes/SOUL.md
~/.hermes/gateway.pid
~/.hermes/gateway.lock
~/.hermes/gateway_state.json
~/.hermes/cron/.tick.lock
~/.hermes/*.lock
~/.hermes/*-shm
~/.hermes/*-wal
```

需要保留旧历史或旧配置时，应单独按隐私和用户归属策略导出或导入，不建议混进初始状态迁移包。
