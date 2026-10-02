# KylinAgent 外部控制接口

重新编译并启动新版 KylinAgent 后，Python 或 shell 可以控制该进程的真实聊天界面。无需额外 Python 包，也无需开放 TCP 端口。

服务随主窗口初始化启动，退出进程时关闭。Unix Socket 默认路径为 `~/.cache/kylin-agent-control.socket`，仅允许当前用户访问，与原有窗口激活 socket 独立。旧版正在运行的进程必须重启才能提供此接口；最小化到托盘后接口仍可用。

## 找不到 Socket 时

`No such file or directory` 或 `control_unavailable` 表示客户端未能连接控制服务。若只有 `~/.cache/kylin-agent.socket`，没有 `~/.cache/kylin-agent-control.socket`，可能仍在运行不包含新接口的旧版。不要用 `touch` 创建 Socket，也不要将客户端指向旧的窗口激活 Socket，它们的协议不同。

在项目根目录安装编译依赖并构建：

```bash
sudo apt install build-essential cmake pkg-config zlib1g-dev qtbase5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5websockets5-dev qtmultimedia5-dev libkysdk-qtwidgets-dev libgsettings-qt-dev
cmake -S . -B build
cmake --build build -j2
```

先通过托盘菜单“退出”完全退出旧进程（关闭窗口通常只是进入托盘），然后在项目根目录运行 `./build/kylin-agent`。KylinAgent 为单实例应用，旧进程未退出时，启动新二进制只会激活旧进程。等待主窗口加载后，在另一个终端执行：

```bash
ls -l ~/.cache/kylin-agent-control.socket
python3 scripts/kylin_agent_ctl.py status

# 获取正在执行任务的会话个数（包含后台会话）
python3 scripts/kylin_agent_ctl.py running-count
python3 scripts/kylin_agent_ctl.py new-session
```

直接运行构建产物不更新 `/usr/bin/kylin-agent`；通过系统应用菜单启动的仍是已安装版本。若新版进程仍未创建控制 Socket，检查启动终端中的 `Unable to start KylinAgent control interface` 日志及客户端路径覆盖设置。

## shell 使用

在项目根目录执行（安装后脚本位于 `/usr/local/share/kylin-agent/scripts/`）：

```bash
# 查看当前会话、输入框和所有桌面会话的任务状态
python3 scripts/kylin_agent_ctl.py status

# 切换到无标题的空白草稿，清空输入和附件，保留其他会话的后台任务
python3 scripts/kylin_agent_ctl.py new-session

# 只填入文本，不发送；支持中文和多行
printf '请解释这段代码：\nprint("hello")\n' | python3 scripts/kylin_agent_ctl.py set-input --stdin

# 发送当前输入框内容（包括界面上已有附件）
python3 scripts/kylin_agent_ctl.py send

# 也可以一次填入并发送；显式 --text/--stdin 会清除界面上已有附件
python3 scripts/kylin_agent_ctl.py send --text '你好'

# 停止当前会话，等同于界面的停止按钮
python3 scripts/kylin_agent_ctl.py stop
```

标准输出为 JSON，成功退出码为 0；失败信息以 JSON 写入标准错误，退出码为 1。全局参数 `--socket PATH`、`--timeout 30` 放在子命令之前。环境变量 `KYLIN_AGENT_CONTROL_SOCKET` 也可覆盖客户端连接路径（不改变服务端路径）。

## Python 使用

```python
import sys
sys.path.insert(0, "scripts")  # 或安装后的 scripts 目录
from kylin_agent_ctl import KylinAgentClient

agent = KylinAgentClient()
agent.new_session()  # 返回 session_id: ""，尚未持久化
agent.set_input("请说明二分查找的原理", session_id="")
result = agent.send(session_id="")
session_id = result["session_id"]  # 首次发送时创建，后续操作使用此 ID
print(result["accepted"])
print(agent.status()["running"])
agent.stop(session_id=session_id)
```

`session_id` 是当前会话保护条件：若用户或其他脚本切换了界面，会返回 `session_mismatch`，不会误操作新选中的会话。它不会切换会话。省略时操作调用瞬间的当前会话。传入空字符串表示要求当前仍为草稿；首次发送后必须改用响应中的非空 `session_id`。空字符串不能区分先后打开的两个草稿。

`new_session` 不访问后端创建接口、不新增侧栏记录，也不接受 `title` / `--title`。反复新建或填写输入都不会持久化空会话；发送第一条消息后，沿用界面逻辑从消息内容生成标题。

## 原始协议

每次连接发送一个 UTF-8 JSON 对象加换行，收到一个 JSON 对象加换行后关闭连接。请求上限 1 MiB（包含换行），须在连接后 10 秒内发完。文本内的换行应由 JSON 编码为 `\n`。服务端支持分包读取；同一连接只处理第一条请求。版本 1 支持以下命令：

| command | 参数 | 行为 |
| --- | --- | --- |
| `status` | 无 | 读取当前输入及桌面会话执行状态 |
| `running_count` | 无 | 返回 `running_count` 和 `running_session_ids`，统计正在执行的桌面会话 |
| `new_session` | 无 | 等同界面新建会话：打开无名称草稿，返回空字符串 `session_id`；首次发送时才创建会话和标题 |
| `set_input` | 字符串 `text`，可选 `session_id` | 替换输入框文本，空字符串表示清空 |
| `send` | 可选字符串 `text`、`session_id` | 发送当前输入或指定文本；当前会话执行中时返回错误，不自动排队 |
| `stop` | 可选 `session_id` | 请求停止当前会话；空闲时幂等成功 |

请求可携带任意 JSON `id`，解析成功后会原样返回：

```json
{"id":"request-1","command":"set_input","text":"你好\n第二行"}
```

成功响应格式为 `{"id":"request-1","ok":true,"result":{...}}`；失败格式为 `{"ok":false,"error":{"code":"...","message":"..."}}`。解析前的协议错误不包含 `id`。除 `running_count` 仅返回计数和会话 ID 列表外，其他成功响应包含操作后的状态：

| 字段 | 含义 |
| --- | --- |
| `session_id` | 当前选中会话；未创建的新会话草稿为空字符串 |
| `input` | 界面输入框的原始文本 |
| `current_running` | 当前会话在桌面端是否处于执行/等待响应状态 |
| `running` | 任意桌面会话是否正在执行，包括切换到后台的会话 |
| `running_session_ids` | 正在执行的桌面会话 ID 列表 |
| `running_count` | 正在执行的桌面会话个数，等于 `running_session_ids` 的长度；不计仅排队的空闲会话 |
| `queued_count` | 所有桌面会话待发送消息总数 |
| `has_pending_tasks` | 正在执行或存在排队消息 |
| `messages_loading` | 当前会话历史是否正在加载 |
| `model_available` | 当前是否有已选择的模型，不代表服务商连接可用 |

`send` 成功额外返回 `accepted: true`，只表示已调用发送流程，不代表模型已完成或成功回答。后端异步失败会显示在聊天界面，状态最终恢复为空闲。`stop` 返回 `stop_requested: true`，表示停止命令已处理，不代表后端工具进程已经全部退出；沿用界面的可恢复中断语义，已有排队消息也沿用界面规则。

状态范围是**本桌面进程管理的聊天任务**，不包含网关独立运行的定时任务、其他客户端任务或模型下载。停止仅作用于当前会话，不会停止后台其他会话。多个控制请求在同步后端调用的嵌套事件循环期间会收到 `busy`，可稍后重试。客户端连接或读取超时无法证明命令未执行，应先查询状态，避免盲目重发 `send` 或 `new_session`。

错误码包括 `unknown_command`、`invalid_argument`、`invalid_json`、`request_too_large`、`timeout`、`busy`、`session_mismatch`、`session_loading`、`task_running`、`no_model`、`empty_input`、`backend_error`。

## 验证

客户端单元测试（真实临时 Unix Socket，不调用模型）：

```bash
python3 -m unittest discover -s tests -p 'test_kylin_agent_ctl.py' -v
```

编译后可运行独立 Qt 界面回归测试（CMake Unix Makefiles 构建）。测试使用临时配置目录，不连接正在运行的桌面进程或模型；覆盖已有消息清空、旧消息容器移除、迟到历史/流响应，以及三个新建入口的侧栏状态：

```bash
cmake --build build -j2
python3 tests/run_control_ui_regression.py --build build
```

新版 KylinAgent 启动后，确保当前没有任务或待发送文本：

```bash
# 检查分包和错误响应；打开草稿，检查输入、状态、会话保护和空闲停止
python3 scripts/verify_kylin_agent_control.py

# 额外实际发送并立即停止，需要已配置模型，可能产生模型费用
python3 scripts/verify_kylin_agent_control.py --send
```

默认验证只打开草稿，不新增会话记录。加 `--send` 时才创建会话并保留在侧栏以便检查，不会删除历史数据。验证脚本检查的是桌面状态和停止请求，不验证后端进程退出或模型回答质量。

查询数量的 Python 示例：

```python
from kylin_agent_ctl import KylinAgentClient
count = KylinAgentClient().running_count()  # 返回整数，无任务时为 0
```

该计数包含当前会话和后台会话，每个会话计一次；不统计模型下载或网关独立定时任务。
