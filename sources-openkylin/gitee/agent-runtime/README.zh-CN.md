# Kylin Agent Runtime


**Kylin Agent Runtime 是基于 Hermes 改进的自进化智能体运行时。** 本项目在 Hermes 基础上继续改进主干与分支协同方法、记忆能力、自进化约束、过程详细显示等核心体验，后续也将持续演进。它内置学习闭环——从经验中创建技能，在使用中改进技能，主动持久化知识，搜索过往对话，并在跨会话中逐步构建对你的深度理解。

## 安装

推荐使用 `uv` 创建虚拟环境并以可编辑模式安装本项目。

```bash
git clone https://gitee.com/openkylin/agent-runtime.git
cd agent-runtime

curl -LsSf https://astral.sh/uv/install.sh | sh
uv venv .venv --python 3.11
source .venv/bin/activate
uv pip install -e ".[all]"
```

如果你已经有 `uv` 和 Python 3.11，可以直接从创建虚拟环境开始。

## 运行 Agent Runtime

首次运行前，先激活虚拟环境并完成模型提供商、API Key 等基础配置：

```bash
source .venv/bin/activate
kylin-agent-runtime setup
```

配置完成后，在项目目录中启动交互式 Agent：

```bash
kylin-agent-runtime
```

也可以按需使用其他运行方式：

```bash
# 启动现代终端界面（TUI）
kylin-agent-runtime --tui

# 执行一次性任务并仅输出最终结果
kylin-agent-runtime -z "分析当前项目并给出改进建议"

# 启动 Web 控制台（默认端口 9119）
kylin-agent-runtime dashboard
```

### 运行消息网关

在 Linux 上，网关使用 `kylin-agent-runtime-gateway.service` systemd 用户服务，也可以直接通过 `systemctl` 管理：

```bash
# 启动、停止或重启服务
systemctl --user start kylin-agent-runtime-gateway.service
systemctl --user stop kylin-agent-runtime-gateway.service
systemctl --user restart kylin-agent-runtime-gateway.service

# 查看服务状态
systemctl --user status kylin-agent-runtime-gateway.service

# 启用或禁用自动启动
systemctl --user enable kylin-agent-runtime-gateway.service
systemctl --user disable kylin-agent-runtime-gateway.service

# 实时查看服务日志
journalctl --user -u kylin-agent-runtime-gateway.service -f
```

对于系统级服务，请去掉 `--user`，并使用 `sudo` 执行相应的 `systemctl` 命令。

查看全部命令及参数：

```bash
kylin-agent-runtime --help
```

## 开发安装

开发或运行测试时安装开发依赖：

```bash
git clone https://gitee.com/openkylin/agent-runtime.git
cd agent-runtime

curl -LsSf https://astral.sh/uv/install.sh | sh
uv venv .venv --python 3.11
source .venv/bin/activate
uv pip install -e ".[all,dev]"
```

## 更新

进入项目目录后拉取最新代码并重新安装：

```bash
git pull
source .venv/bin/activate
uv pip install -e ".[all]"
```

如果依赖锁文件发生变化，建议同步锁定依赖：

```bash
uv sync --all-extras
```

## 测试

运行默认测试：

```bash
scripts/run_tests.sh
```

或在已激活虚拟环境中直接运行：

```bash
python -m pytest tests/ -q
```

## 许可证

MIT — 详见 [LICENSE](LICENSE)。
