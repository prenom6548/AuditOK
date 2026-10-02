# Kylin-CUA 桌面计算机使用代理

Kylin-CUA 是一个基于大语言模型的桌面自动化项目。它通过截图、键盘、鼠标、剪贴板和窗口控制能力，让模型按自然语言指令在本机可见桌面环境中执行操作。

本文档覆盖源码运行、deb 包安装后的基本使用、模型配置，以及 Linux X11/Wayland 桌面环境适配说明。

## 功能概览

- 支持通过自然语言描述桌面操作目标。
- 支持 Linux X11 桌面环境的截图、点击、输入、拖拽、滚动、窗口激活和应用启动。
- 支持 Linux Wayland 桌面环境的截图、点击、输入、拖拽、滚动和多屏区域选择。
- 支持 Windows 相关控制模块。
- 支持 OpenAI 兼容接口、Ollama、Anthropic、Google、DeepSeek、Kimi、MiniMax 和本地 Transformers 模型。
- 支持多角色模型配置，包括 `brain`、`actor`、`memory`、`planner`。
- 支持可选规划、搜索、Skills、任务恢复和运行日志保存。

## 目录结构

```text
.
├── run/
│   ├── config.json            # 默认配置文件
│   └── kylin-cua.py           # 运行入口
├── skill/
│   └── kylin-cua/SKILL.md     # 上层智能体调用 Kylin-CUA 的 Skill
├── src/
│   ├── agent/                 # 智能体核心逻辑
│   ├── controller/            # 动作注册和执行
│   ├── linux/                 # Linux 桌面操作适配，含 X11/Wayland
│   ├── windows/               # Windows 桌面操作适配
│   └── utils/                 # 搜索、技能、状态窗口、本地模型等工具
├── requirements.txt           # Python 依赖
└── version                    # 项目版本
```

## 环境要求

- Python 3.12。
- 推荐使用 Conda 或其他虚拟环境。
- Linux 桌面推荐优先使用 X11；Wayland 已适配，但需要额外桌面工具支持。
- 如果使用云端或远程模型，需要准备对应模型服务的 API Key 或 OpenAI 兼容接口地址。
- 如果使用本地模型，需要提前准备模型权重和足够的显存或内存。

查看当前桌面会话类型：

```bash
echo "$XDG_SESSION_TYPE"
```

X11 会话通常应存在：

```bash
echo "$DISPLAY"
```

Wayland 会话通常应存在：

```bash
echo "$WAYLAND_DISPLAY"
```

## 安装

### deb 包安装

如果已经获得 deb 安装包，可在安装包所在目录执行：

```bash
sudo apt update
sudo apt install ./kylin-cua_0.9.2_amd64.deb
```

如果安装包文件名不同，请替换为实际文件名。若系统不支持直接通过 `apt install ./xxx.deb` 安装，也可以使用：

```bash
sudo dpkg -i kylin-cua_0.9.2_amd64.deb
sudo apt -f install
```

deb 安装后的默认运行入口形态为：

```bash
python3 ~/.local/share/kylin-cua/run/kylin-cua.py "用户的任务请求"
```

如果安装包提供了桌面启动器或命令行包装脚本，以安装包实际入口为准。

### 源码安装

```bash
conda create -n desktop-agent python=3.12
conda activate desktop-agent
pip install -r requirements.txt
```

如需使用本地 Transformers 模型，请根据你的 CUDA、PyTorch 和量化方案确认 `torch`、`transformers`、`accelerate`、`bitsandbytes` 的版本是否匹配。

## Linux 桌面依赖

### X11 依赖

```bash
sudo apt update
sudo apt install -y \
  xclip xdotool wmctrl x11-utils x11-xserver-utils xdg-utils \
  libglib2.0-bin libgtk-3-bin scrot python3-tk
```

依赖说明：

- `xclip`：X11 剪贴板后端。
- `xdotool`、`wmctrl`：键盘输入、窗口激活和窗口控制。
- `x11-utils`、`x11-xserver-utils`：提供 `xprop`、`xrandr` 等工具。
- `libglib2.0-bin`、`libgtk-3-bin`、`xdg-utils`：提供应用启动和文件打开相关命令。
- `scrot`：截图后端。
- `python3-tk`：备用状态窗口依赖。

### Wayland 依赖

Wayland 下 PyAutoGUI 无法像 X11 一样直接截图和控制输入。本项目在 Wayland 会话中会自动切换到：

- `grim` 或 `gnome-screenshot`：截图后端。
- `ydotool`：鼠标移动、点击、滚动、键盘和快捷键后端。
- `wl-copy`、`wl-paste`：剪贴板读写和中文文本粘贴后端。
- `kscreen-doctor`：读取 Wayland/KDE 多屏逻辑坐标，缺失时会回退到截图尺寸。

建议安装：

```bash
sudo apt update
sudo apt install -y ydotool grim wl-clipboard kscreen
```

如果系统使用 GNOME，也可补充：

```bash
sudo apt install -y gnome-screenshot
```

确认命令是否可用：

```bash
command -v ydotool
command -v grim
command -v wl-copy
command -v wl-paste
command -v kscreen-doctor
```

`ydotool` 通常需要后台服务或足够的输入设备权限。如果鼠标键盘动作失败，请先确认 `ydotool` 能在当前会话中手动执行：

```bash
ydotool mousemove --absolute 100 100
ydotool click 0xC0
```

不同发行版对 `ydotool` 的服务名和权限组配置可能不同，请以系统包说明为准。

## Wayland 适配说明

### 会话检测

程序会根据 `XDG_SESSION_TYPE` 判断桌面会话：

- `XDG_SESSION_TYPE=wayland`：使用 Wayland 适配路径。
- `XDG_SESSION_TYPE=x11`：使用 X11/PyAutoGUI 适配路径。
- 未设置时，如果存在 `WAYLAND_DISPLAY`，也会按 Wayland 处理。

### 截图

Wayland 下截图流程为：

1. 优先使用 `grim` 截取目标屏幕或目标区域。
2. 如果 `grim` 不可用或失败，回退到 `gnome-screenshot`。
3. 如果两者都失败，任务会报错：`Wayland screenshot failed: neither grim nor gnome-screenshot succeeded.`

多屏环境下，程序会优先通过 `kscreen-doctor -o` 获取每个屏幕的逻辑坐标；如果无法获取，则回退到整张截图尺寸。

### 输入、点击和滚动

Wayland 下输入控制流程为：

- 鼠标移动、单击、双击、右键、拖拽、滚动：通过 `ydotool` 执行。
- 快捷键：通过 `ydotool key` 执行。
- 中文和长文本输入：优先写入 `wl-copy` 剪贴板，再通过 `Ctrl+V` 粘贴。
- 如果 `wl-copy` 不可用，会回退到 `ydotool type`，但中文输入稳定性可能下降。

当前限制：

- `ydotool` 不支持可靠读取当前鼠标位置，Wayland 下 `get_mouse_position` 会返回 `(0, 0)`。
- 坐标可能受系统缩放、合成器实现和多屏布局影响，需要按下文进行校准。

### Wayland 坐标校准

Wayland 的截图坐标和 `ydotool` 绝对坐标不一定一一对应。项目提供以下配置项进行校准：

```json
{
  "agent": {
    "target_screen": 1,
    "wayland_ydotool_factor": 2.4,
    "wayland_coord_scale_x": null,
    "wayland_coord_scale_y": null,
    "wayland_coord_offset_x": null,
    "wayland_coord_offset_y": null
  }
}
```

这些配置会在启动时映射为环境变量：

| 配置项 | 环境变量 | 说明 |
| --- | --- | --- |
| `target_screen` | `KYLIN_TARGET_SCREEN` | 目标屏幕编号，使用 1-based 索引，也可写成 `screen1`、`screen2`。 |
| `wayland_ydotool_factor` | `KYLIN_WAYLAND_YDOTOOL_FACTOR` | 截图逻辑坐标到 `ydotool` 坐标的缩放因子，默认 `2.4`。 |
| `wayland_coord_scale_x` | `KYLIN_WAYLAND_COORD_SCALE_X` | X 坐标微调比例，默认 `1.0`。 |
| `wayland_coord_scale_y` | `KYLIN_WAYLAND_COORD_SCALE_Y` | Y 坐标微调比例，默认 `1.0`。 |
| `wayland_coord_offset_x` | `KYLIN_WAYLAND_COORD_OFFSET_X` | X 坐标偏移，默认 `0.0`。 |
| `wayland_coord_offset_y` | `KYLIN_WAYLAND_COORD_OFFSET_Y` | Y 坐标偏移，默认 `0.0`。 |

校准建议：

1. 先保持默认 `wayland_ydotool_factor=2.4` 运行一个简单点击任务。
2. 如果点击整体偏大或偏小，优先调整 `wayland_ydotool_factor`。
3. 如果 X/Y 方向误差比例不同，再调整 `wayland_coord_scale_x` 和 `wayland_coord_scale_y`。
4. 如果所有点击都存在固定方向偏移，再调整 `wayland_coord_offset_x` 和 `wayland_coord_offset_y`。
5. 多屏环境先确认 `target_screen` 是否选中了目标显示器。

也可以临时通过环境变量覆盖：

```bash
export KYLIN_WAYLAND_YDOTOOL_FACTOR=2.4
export KYLIN_WAYLAND_COORD_SCALE_X=1.0
export KYLIN_WAYLAND_COORD_SCALE_Y=1.0
export KYLIN_WAYLAND_COORD_OFFSET_X=0
export KYLIN_WAYLAND_COORD_OFFSET_Y=0
```

## 配置

主配置文件位于：

```text
run/config.json
```

deb 包安装后通常位于：

```text
~/.local/share/kylin-cua/run/config.json
```

运行前至少需要配置：

- `agent.task`：本次要执行的桌面任务。
- `brain_llm`：负责观察、理解和决策的模型。
- `actor_llm`：负责生成具体动作的模型。
- `memory_llm`：负责记忆压缩和上下文整理的模型。
- `planner_llm`：开启规划时使用的模型。

### OpenAI 兼容接口示例

```json
{
  "brain_llm": {
    "provider": "openai_compatible",
    "model_name": "your-model",
    "base_url": "http://127.0.0.1:8080/v1",
    "api_key": "not-needed",
    "temperature": 0.0,
    "max_tokens": 2048,
    "timeout": 45
  },
  "actor_llm": {
    "provider": "openai_compatible",
    "model_name": "your-model",
    "base_url": "http://127.0.0.1:8080/v1",
    "api_key": "not-needed",
    "temperature": 0.0,
    "max_tokens": 1024,
    "timeout": 30
  }
}
```

`memory_llm` 和 `planner_llm` 可以使用相同格式配置。对于本地 OpenAI 兼容服务，请确认地址是否包含 `/v1`，以你的服务实现为准。不要把真实 API Key 提交到公开仓库。

### 任务示例

```json
{
  "agent": {
    "task": "打开浏览器，搜索 Python 官方文档，并找到 pathlib 的说明页面",
    "max_steps": 100,
    "max_actions_per_step": 5,
    "target_screen": 1,
    "wayland_ydotool_factor": 2.4,
    "wayland_coord_scale_x": null,
    "wayland_coord_scale_y": null,
    "wayland_coord_offset_x": null,
    "wayland_coord_offset_y": null,
    "use_plan": false,
    "use_skills": false,
    "resume": false
  }
}
```

建议把任务写清楚，包含目标、约束和停止条件。涉及账号、付款、下载、发送消息、删除文件等操作时，应在任务中明确是否允许执行。

## 运行

源码运行：

```bash
python3 run/kylin-cua.py
```

通过命令行覆盖任务：

```bash
python3 run/kylin-cua.py "打开浏览器，搜索 openKylin 官网"
```

指定配置文件：

```bash
python3 run/kylin-cua.py -c run/config.json "打开文件管理器，进入下载目录"
```

运行期间程序会读取 `config.json`，根据任务截图、规划并执行桌面动作。

可用的运行相关配置：

- `logging_level`：日志级别，常用值为 `INFO` 或 `DEBUG`。
- `output_dir`：输出文件目录，默认 `.kylin_tmp`。
- `cleanup_previous_runs`：是否整理上次运行产生的数据文件。
- `agent.status_window_enabled`：是否显示任务监视器。
- `agent.force_stop_hotkey`：强制停止热键，例如 `ctrl+shift+q`。
- `agent.save_*_conversation_path`：保存模型交互日志。

## 任务监视器、日志和状态文件

默认启用任务监视器：

```json
{
  "agent": {
    "status_window_enabled": true
  }
}
```

运行时会生成：

| 路径 | 说明 |
| --- | --- |
| `<output_dir>/<时间>_<任务>.log` | 本次任务主日志。 |
| `<output_dir>/step_log.jsonl` | 每一步 JSONL 记录。 |
| `<output_dir>/step_records/step_records.json` | 状态窗口保存的步骤记录。 |

状态窗口会自动启用 Qt HighDPI 缩放。跨系统部署时，可以通过环境变量指定字体：

```bash
export STATUS_WINDOW_FONT="Noto Sans CJK SC"
export STATUS_WINDOW_PRIMARY_FONT="Noto Sans CJK SC:style=Regular:lang=zh-cn"
export STATUS_WINDOW_EXTENDED_FONT="Noto Sans CJK SC:style=Regular:lang=zh-cn"
```

如果系统没有对应字体，请先安装 Noto Sans CJK、Source Han Sans、Microsoft YaHei、PingFang SC 或其他覆盖中文字符的字体。

## Skills

Skills 是可选的 Markdown 操作说明。当前项目内置的 Kylin-CUA Skill 位于：

```text
skill/kylin-cua/SKILL.md
```

它用于让上层智能体在用户需要操作真实 GUI、桌面应用、浏览器页面、系统窗口、文件选择器或弹窗时，优先调用 Kylin-CUA。

启用自定义 Skills 时，可在配置中设置：

```json
{
  "agent": {
    "use_plan": true,
    "use_skills": true,
    "skills_dir": "skills",
    "skills_max_chars": 4000
  }
}
```

## 恢复中断任务

如果任务中断后需要继续，设置稳定的 `agent_id` 并开启 `resume`：

```json
{
  "agent": {
    "resume": true,
    "agent_id": "my-task-001"
  }
}
```

注意事项：

- 继续运行时应保持相同的 `task`。
- 只有对应 `agent_id` 的历史记忆文件存在时才会恢复。
- 若要重新开始，请关闭 `resume`、更换 `agent_id`，或清理对应的历史输出目录。

## Wayland 常见问题

| 问题 | 处理方法 |
| --- | --- |
| 提示 `ydotool is required for Wayland actions` | 安装 `ydotool`，并确认服务和输入权限可用。 |
| 截图失败 | 安装 `grim`；GNOME 环境可安装 `gnome-screenshot` 作为回退。 |
| 中文输入失败或文本不完整 | 安装 `wl-clipboard`，确认 `wl-copy`、`wl-paste` 可用。 |
| 点击位置偏移 | 调整 `wayland_ydotool_factor`；若仍有固定偏移，再调 `wayland_coord_offset_x/y`。 |
| 多屏点击到错误屏幕 | 设置 `agent.target_screen` 或环境变量 `KYLIN_TARGET_SCREEN`。 |
| 滚动方向不符合预期 | 检查目标窗口是否获得焦点，并用简单滚动任务复测。 |
| 状态窗口显示异常 | 尝试设置 `status_window_enabled=false`，或调整 `STATUS_WINDOW_FONT`、`KYLIN_STATUS_BACKEND`。 |

## 开发

- 新增桌面动作时，优先放到对应平台目录，再通过 `controller` 注册。
- 新增模型服务时，优先扩展 `run/kylin-cua.py` 中的模型构建逻辑。
- 新增 Linux 桌面能力时，同时考虑 X11 和 Wayland 后端差异。
- 修改 Wayland 坐标逻辑后，至少验证截图区域、点击、输入、滚动和多屏场景。
- 提交前建议至少运行一次目标配置，并检查日志中是否有模型调用、截图或动作执行异常。

## 许可

本项目的许可信息见 [LICENSE](LICENSE)。
