# Hermes Agent 功能清单

本清单按当前仓库代码和文档整理，覆盖用户可见能力、开发扩展能力和平台能力。

## 1. 核心对话能力

- 多轮 AI 对话。
- 工具调用循环。
- 流式输出。
- reasoning 显示与隐藏。
- fast/priority service tier 切换。
- 运行中中断、停止、排队、steer。
- 重试上一轮。
- 撤销上一轮 user/assistant exchange。
- 会话标题、分支、恢复、压缩。
- token 用量和成本估算。
- 会话洞察和历史搜索。

## 2. 入口和运行形态

- 经典交互式 CLI：`hermes`。
- Ink TUI：`kylin-agent-runtime --tui`。
- 消息网关：`kylin-agent-runtime gateway`。
- OpenAI-compatible API server。
- Web Dashboard。
- ACP 编辑器集成：`hermes-acp`。
- Cron 调度器。
- Batch Runner 批处理和轨迹生成。
- Python 库式调用：`AIAgent.chat()` / `run_conversation()`。

## 3. 模型和 Provider

- OpenAI-compatible chat completions。
- OpenAI Responses/Codex。
- Anthropic Messages。
- OpenRouter、Nous Portal、OpenAI、Anthropic、NVIDIA、xAI、Gemini、Kimi/Moonshot、MiniMax、z.ai/GLM、Hugging Face、自定义 endpoint 等 provider 路径。
- Provider 插件。
- provider/model slash command 切换。
- fallback providers。
- credential pools。
- provider routing。
- auxiliary model 配置：vision、embedding、title、compression 等。

## 4. 工具能力

默认核心工具覆盖：

- Web 搜索和网页抽取：`web_search`、`web_extract`。
- 终端和进程：`terminal`、`process`。
- 文件：`read_file`、`write_file`、`patch`、`search_files`。
- 视觉：`vision_analyze`。
- 图像生成：`image_generate`。
- 技能管理：`skills_list`、`skill_view`、`skill_manage`。
- 浏览器自动化：navigate、snapshot、click、type、scroll、back、press、images、vision、console、CDP、dialog。
- TTS：`text_to_speech`。
- 计划：`todo`。
- 会话记忆检索：`memory`（独立 `memory.db`）。
- 澄清问题：`clarify`。
- 代码执行：`execute_code`。
- 子代理委派：`delegate_task`。
- Cron 任务管理：`cronjob`。
- 跨平台消息：`send_message`。
- Home Assistant：实体列表、状态读取、服务列表、服务调用。
- Kanban 多代理协作：show、list、complete、block、heartbeat、comment、create、link、unblock。
- Computer use：macOS 背景桌面控制。

可选或平台工具包括：

- x/Twitter 搜索：`x_search`。
- 视频分析：`video_analyze`。
- 视频生成：`video_generate`。
- Mixture of Agents：`mixture_of_agents`。
- Discord read/admin 工具。
- Feishu/Lark 文档和 Drive 评论工具。
- Yuanbao 群信息、成员、私聊、贴纸工具。
- Spotify 播放、搜索、队列、歌单、专辑、资料库工具。
- MCP server 动态工具。
- 插件注册工具。

## 5. 终端和执行环境

- 本地终端。
- Docker。
- SSH。
- Singularity。
- Modal。
- Daytona。
- Vercel Sandbox。
- 后台进程管理。
- 危险命令检测和审批。
- per-task cwd。
- 环境变量和 credential 文件透传。

## 6. Web、浏览器和媒体

- Web search provider。
- 网页内容抽取。
- 浏览器自动化。
- 本地 Chrome/Brave/Chromium/Edge CDP 连接。
- 浏览器截图/视觉分析。
- 控制台读取。
- 对话框处理。
- 图片粘贴和本地图片附件。
- 图像生成 provider 插件。
- 视频生成 provider 插件。
- TTS 和语音模式。
- 语音消息转写。
- Discord 语音频道输入输出。

## 7. 记忆、技能和上下文

- `MEMORY.md` 长期记忆。
- `USER.md` 用户画像。
- Memory provider 插件：Honcho、Mem0、Supermemory、ByteRover、Hindsight、Holographic、OpenViking、RetainDB 等。
- 技能系统：内置技能、用户技能、可选官方技能。
- Skills Hub。
- slash skill 调用。
- skill reload。
- skill curator 后台维护。
- 项目上下文文件：`AGENTS.md`、`.kylin-agent-runtime.md`、`CLAUDE.md`、`SOUL.md`、`.cursorrules`。
- `@` context references：文件、目录、git diff、URL 注入。
- 上下文压缩。
- prompt caching。

## 8. 自动化和协作

- Cron 自然语言任务。
- Cron 表达式任务。
- 任务 pause/resume/edit/remove/run。
- 结果投递到消息平台。
- 子代理并行。
- `execute_code` 用 Python 编排工具。
- Kanban 多 profile 协作板。
- background prompt。
- queued follow-up。
- standing goals 和 subgoals。
- webhook 集成。
- batch trajectory generation。
- trajectory compression。

## 9. 消息平台

Gateway 支持或预留的适配器包括：

- Telegram。
- Discord。
- Slack。
- WhatsApp。
- Signal。
- Matrix。
- Mattermost。
- Email。
- SMS/Twilio。
- DingTalk。
- Feishu/Lark。
- WeCom。
- WeCom callback。
- Weixin。
- QQBot。
- BlueBubbles/iMessage。
- Yuanbao。
- Home Assistant。
- Webhook。
- API Server。
- Open WebUI。
- Teams / Microsoft Graph webhook。
- Google Chat / LINE 等文档路径。

网关功能：

- 多平台统一 session。
- DM pairing。
- allowlist / allow-all。
- home channel。
- `/approve`、`/deny` 危险命令审批。
- `/sethome`。
- `/topic` Telegram DM topic sessions。
- 运行中 agent 状态和控制。
- 跨平台 `send_message`。

## 10. Slash Commands

命令由 `kylin_agent_runtime_cli/commands.py` 的 `COMMAND_REGISTRY` 统一定义。当前主要命令包括：

- Session：`/new`、`/topic`、`/clear`、`/redraw`、`/history`、`/save`、`/retry`、`/undo`、`/title`、`/handoff`、`/branch`、`/compress`、`/rollback`、`/snapshot`、`/stop`、`/approve`、`/deny`、`/background`、`/agents`、`/queue`、`/steer`、`/goal`、`/subgoal`、`/status`、`/sethome`、`/resume`、`/sessions`。
- Configuration：`/config`、`/model`、`/codex-runtime`、`/personality`、`/statusbar`、`/verbose`、`/footer`、`/yolo`、`/reasoning`、`/fast`、`/skin`、`/indicator`、`/voice`、`/busy`。
- Tools & Skills：`/tools`、`/toolsets`、`/skills`、`/bundles`、`/cron`、`/curator`、`/kanban`、`/reload`、`/reload-mcp`、`/reload-skills`、`/browser`、`/plugins`。
- Info：`/commands`、`/help`、`/restart`、`/usage`、`/insights`、`/platforms`、`/platform`、`/copy`、`/paste`、`/image`、`/update`、`/debug`、`/whoami`、`/profile`、`/gquota`。
- Exit：`/quit`、`/exit`。

## 11. 配置和主题

- `~/.kylin-agent-runtime/config.yaml` 用户配置。
- `~/.kylin-agent-runtime/.env` 密钥。
- profile-aware `HERMES_HOME`。
- 多 profile。
- setup wizard。
- provider/model/tools/skills/gateway 配置。
- CLI skin engine。
- prompt symbol、banner、spinner、tool prefix、response label 定制。
- snapshot 配置备份/恢复。

## 12. 安全和可靠性

- 危险命令审批。
- 命令 allowlist。
- Gateway 用户授权。
- DM pairing。
- token lock 防止多个 profile 抢同一 bot token。
- 终端 sandbox 后端。
- 非秘密配置和秘密环境变量分离。
- 依赖 exact pin 和 lock。
- 插件隔离和工具白名单。
- 日志按 session 过滤。
- checkpoint 和 rollback。

## 13. 开发和扩展能力

- 新增原生工具。
- 新增 toolset。
- 新增 slash command。
- 新增 Gateway platform adapter。
- 新增 plugin。
- 新增 memory provider 插件。
- 新增 model provider 插件。
- 新增 context engine 插件。
- 新增 image/video generation provider。
- 新增 skill 或 optional skill。
- Dashboard 扩展。
- ACP 扩展。
- 测试脚本：`scripts/run_tests.sh`。
- Docusaurus 文档站。
