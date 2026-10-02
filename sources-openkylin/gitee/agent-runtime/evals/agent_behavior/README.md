# Agent Behavior Evals

This directory contains deterministic behavior evals for Hermes/Kylin agent
workflows.  The eval runner reads JSONL cases and validates recorded or
synthetic traces without calling an LLM or mutating the local workspace.

Run:

```bash
scripts/run_agent_evals.sh
scripts/run_agent_evals.sh --json
scripts/run_agent_evals.sh --output eval-results/agent_behavior.json
scripts/run_agent_evals.sh --trajectory trajectory_samples.jsonl
scripts/run_agent_evals.sh --trace-events ~/.kylin-agent-runtime/eval_traces/agent_events.jsonl
```

Each JSONL line is one case:

```json
{
  "id": "pip_install_tsinghua",
  "name": "pip 安装必须使用清华源",
  "task": "安装一个 Python 依赖",
  "trace": [
    {
      "type": "tool_call",
      "name": "terminal",
      "args": {
        "command": "python -m pip install rich",
        "env": {
          "PIP_INDEX_URL": "https://pypi.tuna.tsinghua.edu.cn/simple"
        }
      },
      "duration_ms": 1200
    }
  ],
  "criteria": {
    "require_pip_tsinghua_index": true,
    "max_tool_calls": 3,
    "max_seconds": 30
  }
}
```

Supported criteria currently include:

- `expected_tools_ordered`: required tool subsequence.
- `required_tools`: tools that must appear.
- `forbidden_tools`: tools that must not appear.
- `forbidden_shell_regex`: unsafe command patterns.
- `required_event_types`: non-tool events that must appear.
- `require_todo_complete`: final TODO state must have no active items.
- `require_pip_tsinghua_index`: every pip install must use the Tsinghua index.
- `require_pip_tsinghua_index_if_used`: pip installs must use the Tsinghua
  index when the trace contains pip installs.
- `require_delegate_after_branch_todo`: branch TODO `in_progress` must be followed
  by `delegate_task`.
- `max_tool_calls`: upper bound for tool calls.
- `max_seconds`: upper bound for summed trace duration.

## Real traces

There are two non-invasive ways to evaluate real agent runs.

### Existing trajectory files

Hermes already supports trajectory saving.  Those files are ShareGPT/XML-style
training traces, so the runner converts their `<tool_call>` and
`<tool_response>` blocks into eval events:

```bash
python run_agent.py --save_trajectories --query "安装 rich 并说明结果"
scripts/run_agent_evals.sh --trajectory trajectory_samples.jsonl
```

Trajectory files do not currently include per-tool `duration_ms`, so they are
best for checking tool order, tool arguments, pip-index policy, and dangerous
command patterns.

### Opt-in event recorder

For duration-aware event traces, enable the bundled standalone plugin and opt in
with an environment variable:

```bash
kylin-agent-runtime plugins enable agent-eval-recorder
HERMES_AGENT_EVAL_RECORD=1 kylin-agent-runtime -z "执行一个需要工具的任务"
scripts/run_agent_evals.sh --trace-events ~/.kylin-agent-runtime/eval_traces/agent_events.jsonl
```

Optionally override the event file:

```bash
HERMES_AGENT_EVAL_RECORD=1 \
HERMES_AGENT_EVAL_TRACE_FILE=/tmp/agent-events.jsonl \
kylin-agent-runtime -z "执行一个需要工具的任务"
```

The recorder is inert unless both the plugin is enabled and
`HERMES_AGENT_EVAL_RECORD` is truthy.  Hook errors are swallowed so recording
cannot alter the agent's normal task behavior.

## Test-set policy

Keep `cases.jsonl` as the small, stable regression set.  Treat real traces as a
candidate pool: after a real bug or behavior regression is reproduced, minimize
the trace, remove sensitive content, and then add the minimized version to
`cases.jsonl`.
