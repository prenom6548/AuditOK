# 主干-分支当前实现说明

> 状态说明：主动式主干—分支编排现已默认关闭。普通任务由主智能体在当前
> 上下文中连续执行，不会因为任务复杂、多步骤或可能并行而自动创建子智能体。
> `delegate_task` 能力仍然保留，仅用于用户明确要求委派、子智能体或
> 主干—分支工作流的场景。下文保留的是该可选能力的实现与启用说明。

## 1. 定位

当前 Hermes 的主干-分支能力是一个计划驱动的有界编排层：

- 主干 agent 仍然负责理解用户请求、维护完整上下文、做最终判断。
- 轻量 router 判断当前请求是否属于复杂任务。
- 如果命中，router 会在模型请求中追加临时编排要求。
- 主模型必须先调用 `todo` 建立非空、有序的执行计划；直接回答或先调用其他
  实质性工具会被运行时拒绝并重试。
- 只有共享同一 `parallel_group` 的独立 TODO 才批量委派；其余 TODO 按依赖顺序逐项完成。
- 实质性 TODO 默认 `executor: branch`；协调、父级验证、用户交互、简单直接动作和
  发送消息等外部副作用使用 `executor: mainline`。
- 缺少必要用户输入时允许先 `clarify`，得到回答后继续或调整原计划。
- 达到有限重试次数后仍无法规划时才降级执行，避免不支持工具调用的模型卡死。

因此，当前路径是“规则分类 + 运行时强制规划 + TODO 状态驱动执行 + 显式并行组分支”。

## 2. 实际执行链路

一次可能进入主干-分支的请求，大致流程如下：

```text
用户输入
  -> agent/conversation_loop.py 构建本轮主干请求
  -> agent/delegation_router.py 做轻量规则打分
  -> 命中则生成 mainline orchestration requirement
  -> 要求以临时上下文注入当前用户消息
  -> 主模型必须先调用 todo 创建有序计划
  -> 未规划则丢弃该响应并进行有限重试（clarify 除外）
  -> 顺序项逐项执行；显式 parallel_group 才调用 delegate_task(tasks=[...])
  -> tools/delegate_tool.py 按需创建一个或多个子 AIAgent
  -> 子 agent 使用 branch 系统提示词、完整 memory/profile 上下文和受限工具集执行任务
  -> 子 agent 只把摘要结果返回主干
  -> 主干综合、验证并回复用户
```

关键点：

- router 不调用模型。
- router 不拆任务。
- router 不拆任务，而是由运行时确保主模型先建立 TODO 计划。
- 编排要求和纠正反馈不持久化到会话历史。
- `delegate_task` 才是真正的分支入口。
- TODO 工具结果总会给出顺序执行策略；只有多个 `in_progress` 项显式共享同一
  `parallel_group` 时才追加 `delegation_required`。

## 3. 命名说明

`delegate_task` 是当前实现中的 canonical tool name。这个名字强调模型的调用意图：把一个或多个任务委派出去。实际执行机制则是创建隔离上下文的子 agent，因此在架构讨论中也可以把它称为“子代理分支入口”。

不要把 `delegate_task` 理解成普通函数式任务队列：它会启动新的 `AIAgent` 实例，使用 branch 系统提示词、显式传入的 task/context、受限 toolsets，并且只把最终摘要结果返回主干。虽然 `delegate_subagent` 这样的名字更贴近内部机制，但为了保持工具调用习惯、schema 兼容和已有文档/测试稳定，当前保留 `delegate_task` 作为正式名称。

## 4. 轻量 Router 在哪里

轻量 router 位于：

```text
agent/delegation_router.py
```

主循环调用位置在：

```text
agent/conversation_loop.py
```

核心逻辑是调用：

```python
decide_delegation_hint(
    user_message,
    valid_tool_names=agent.valid_tool_names,
    prompt_profile=agent.prompt_profile,
    config=delegation_config,
)
```

如果返回 `should_hint=True`，主循环会把要求放进当前用户消息的临时 API
消息里。第一次请求必定注入；模型未按要求委派时，重试请求还会同时注入
纠正反馈：

```python
if _delegation_router_hint and (
    api_call_count == 1 or _delegation_enforcement_feedback
):
    _injections.append(_delegation_router_hint)
```

这些内容只存在于 API 请求副本，不会污染持久会话消息。

## 5. Router 的实现逻辑

Router 是纯规则打分器，主要检查这些条件。

### 5.1 配置开关

配置位于 `config.yaml` 的 `delegation` 段：

```yaml
delegation:
  mainline_branch_enabled: false
  router_enabled: false
  router_min_score: 3
  orchestrate_uncertain_tasks: false
  enforce_complex_tasks: false
  enforcement_max_retries: 2
  todo_fanout_enabled: false
```

如果 `router_enabled: false`，router 不分类也不强制。设置
`enforce_complex_tasks: false` 可以恢复为仅提示模式。

### 5.2 必须有 `delegate_task`

如果当前 agent 可用工具里没有 `delegate_task`，router 不会提示。这样可以避免提示模型使用不存在的工具。

### 5.3 子 agent 不再被提示

如果当前 agent 的 `prompt_profile` 是 `lean`，router 不提示。普通 leaf 分支虽然使用
`branch` profile，但没有 `delegate_task` 工具，因此也不会递归委派；只有显式启用的
orchestrator 分支可以在深度限制内继续创建子分支。

### 5.4 规则打分

Router 会按文本信号加分：

- 明确提到 `delegate_task`、`subagent`、`子代理`、`主干分支` 等。
- 出现“并行、同时、分头、分别、split、fan-out”等并发或拆分词。
- 出现“多个角度、多个方向、分别分析、分别检查”等多视角词。
- 出现“调研、研究、对比、review、debug、排查、根因、架构、性能、安全”等重任务词。
- 较长且包含多个分隔片段的请求会额外加少量分。

如果分数达到 `router_min_score`，默认注入强制编排要求。实现、开发、
修改、重构、修复和迁移等实质性工程任务也会计入复杂度；明确的拼写、
错别字、标点和纯格式调整仍按简单任务处理。

为避免纯正则关键词漏掉复杂任务，默认采用三态路由：明确简单的短问答、
寒暄、单步动作和微小文本修改直达；明确复杂的任务进入编排；无法确定的
任务在 `orchestrate_uncertain_tasks: true` 时也进入编排。这样分类错误更
倾向于多一次分支调用，而不是让复杂任务漏过强制编排。

### 5.5 简单任务抑制

简单动作类请求会被压低，例如：

```text
看下、查看、打开、搜索、运行、执行、读取、保存、记住
```

如果只是简单的一步工具动作，router 不鼓励分支，避免多一次模型调用带来的延迟。

## 6. 分支如何创建

真正创建分支的是：

```text
tools/delegate_tool.py
```

核心函数是：

```text
_build_child_agent()
```

子 agent 创建时会使用隔离的分支运行策略：

```python
skip_context_files=False
skip_memory=False
prompt_profile="branch"
enabled_toolsets=child_toolsets
ephemeral_system_prompt=child_prompt
quiet_mode=True
```

这表示子 agent 默认不加载父会话历史，但会加载与主智能体相同的 memory、
用户画像和外部 memory provider 上下文，使用 focused subagent identity，并保留
完整工具指导、技能目录、环境信息和项目 context files。`memory` 写入工具仍被禁用，
避免分支直接修改共享长期记忆。父上下文只看到
`delegate_task` 调用和子 agent 的最终摘要结果。该组合用于验证完整分支系统提示词
是否比极简 lean prompt 带来更好的执行质量。

## 7. TODO Planning 与顺序/并行执行

复杂任务第一步必须使用 `todo`。默认执行模型是顺序的：只允许一个 TODO 为
`in_progress`，完成并验证后标记 `completed`，再推进下一项。执行发现新情况时可以
增加、重排、取消或修改 TODO。

每项还可以指定执行者：

- `executor: branch`：默认值。当前顺序项必须调用单任务
  `delegate_task(goal=...)`，同步等待结果，然后更新 TODO，才能推进下一项。
- `executor: mainline`：用于协调、父级验证、`clarify`、非常小的直接动作，以及
  发送消息等必须由主干承担的外部副作用。

触发条件：

- 当前工具是 `todo`。
- 本次调用写入了 `todos`。
- 至少两个 `in_progress` 项共享同一个非空 `parallel_group`。
- 当前 agent 不是受限的 `lean` agent。
- 当前 agent 可用工具中存在 `delegate_task`。

命中后，`agent/tool_executor.py` 会在 TODO JSON 结果中追加：

```json
{
  "delegation_required": {
    "required": true,
    "reason": "todo_plan_has_explicit_parallel_group",
    "parallel_group": "review",
    "instruction": "On the next step, call delegate_task with a tasks array..."
  }
}
```

这不是在工具层偷偷执行子 agent，而是给下一轮模型并行委派建议。没有共同
`parallel_group` 的 TODO 不会扇出，必须等待当前项完成后再执行下一项。

对于不需要计划的单步明确动作（例如“打开 WPS”或“用微信给张三发送‘会议改到
三点’”），router 返回 `direct_single_action`，主干直接执行，不创建 TODO 或分支。
如果联系人、内容或作用范围存在实质歧义，则先 `clarify`。

这样做的原因是：

- 保持 OpenAI 工具调用协议简单，不在一个 tool result 内额外伪造另一个 tool call。
- 保持主模型仍能决定 task context、toolsets、role 等委派参数。
- 普通 leaf 分支没有 `delegate_task`，不会递归触发；orchestrator 分支只会在配置的
  深度上限内继续委派。

## 8. Toolset 策略

`delegate_task` 可以显式传入子 agent 的 toolsets。如果没有显式指定，会根据配置决定：

```yaml
delegation:
  default_tool_policy: inherit
```

当前支持的方向：

- `inherit`：继承父 agent 可用工具集，再移除危险或不适合子 agent 的工具。
- `minimal`：根据任务内容推断更小的工具集，并且仍与父 agent 可用工具求交集。

无论哪种策略，子 agent 默认会剥离这些工具：

```text
delegate_task, clarify, memory, send_message
```

目的分别是：

- 避免无限递归委派。
- 避免子 agent 直接向用户追问。
- 避免子 agent 写长期记忆。
- 避免子 agent 向外部平台发消息。
`execute_code` 不再被裁剪，分支可以像主智能体一样执行需要编程编排的多步操作。

## 9. 当前实现的优势

### 9.1 降低主干上下文污染

主干不需要保存子 agent 的全部中间工具输出、失败尝试和推理过程。主干只收到子任务摘要、证据和结论，后续模型调用不必反复携带大量中间噪音。

### 9.2 降低部分 token 成本

子 agent 使用 `lean` prompt，且跳过 memory 和 context files。对于局部任务，它不需要完整主干上下文，可以减少输入 token。

真正节省多少取决于子 agent 的工具集是否也被缩小。如果子 agent 仍继承大量工具 schema，收益会被工具 schema 抵消。

### 9.3 支持并行子任务

`delegate_task` 支持 batch 形式，可以同时运行多个子任务。适合：

- 多文件并行检查。
- 多来源资料调研。
- 多方案对比。
- 日志、代码、测试分别排查。

主干可以等待多个分支结果后再综合判断。

### 9.4 失败隔离

子 agent 失败不会把失败轨迹全部写入主干消息历史。主干可以忽略失败分支、重新派发或做关键验证。

### 9.5 权限更可控

分支可以只拿局部任务需要的工具。比如：

- 只读代码：`file`
- 本地验证：`terminal`
- 网页调研：`web`
- 浏览器任务：`browser`

这比让所有子任务都使用完整 Hermes 工具集更安全。

### 9.6 更接近长任务 DAG

主干保留完整语境和最终决策权，分支负责局部探索。这个结构更容易扩展成多子任务、并行、多阶段的任务图。

## 10. 当前实现的边界

### 10.1 强制规划不等于完整 DAG 调度

复杂任务的直接回答和规划前的实质性工具调用会被拒绝，运行时要求模型先调用
`todo`。具体拆成几个任务、依赖关系和委派参数仍由主模型决定。

对不能可靠产生工具调用的模型，达到 `enforcement_max_retries` 后会降级
到主干执行。这是兼容性逃生口，因此“强制”是有界强制，而不是无限重试。

显式并行组的 `delegation_required` 会给模型提供建议任务数组，但不会伪造工具调用。

### 10.2 规划由模型生成

当前没有独立 planner 模型；TODO 内容由主模型生成，但运行时强制规划发生在执行前。

### 10.3 简单任务可能不触发

为了避免过度委派，简单任务会被 router 抑制。这是有意设计，因为简单任务拆分后通常更慢。

### 10.4 子 agent 摘要可能丢信息

主干只收到摘要，不自动看到完整过程。如果任务需要高准确性，主干仍应复核关键证据，例如文件路径、行号、命令输出、URL 或测试结果。

### 10.5 子 agent 不应写长期状态

当前默认禁用 `memory`，这是合理的。长期 memory、skill 写入应该由主干验证后执行，避免子 agent 把临时探索写成长期知识。

## 11. 适合触发的任务

更可能用到主干-分支的任务：

- “并行检查这几个模块的性能瓶颈。”
- “分别从代码、测试、日志三个方向排查原因。”
- “对比三种实现方案，并给出优缺点。”
- “调研多个来源，总结证据和风险。”
- “审查这个改动可能影响哪些路径。”
- “主干-分支方式分析这个问题。”

不太适合的任务：

- “打开某个网页。”
- “运行一个命令。”
- “读取一个文件。”
- “把这句话保存到记忆。”
- “改一个很小的拼写错误。”

## 12. 调试和观测

当前实现已经写入一些会话详细过程记录和终端 print。

主干 router 决策事件：

```text
delegation.router.decision
```

子 agent 构建和运行事件包括：

```text
delegate.child.build.start
delegate.child.llm.submit
```

TODO planning 要求委派的事件：

```text
delegation.todo.required
delegation.todo_completion.retry
```

终端中也可能看到类似：

```text
====delegation router.hint score=...
====delegation todo.required active_todos=...
====delegate child.build.start ...
```

如果用户任务明显适合分支但没有触发，可以检查：

- 当前工具列表里是否有 `delegate_task`。
- `delegation.router_enabled` 是否为 `true`。
- `delegation.router_min_score` 是否过高。
- 用户请求是否被简单任务规则压制。
- 是否出现 `delegation.planning.retry` 或最终的兼容性降级事件。
- `delegate_task` 所在 toolset 是否被平台或配置禁用。

## 13. 总结

当前主干-分支实现是一个有界强制编排层：

- 主干仍是完整 agent。
- Router 对任务复杂度做轻量分类。
- 复杂任务默认必须先调用 `todo` 创建计划。
- 未遵守的响应不会执行普通工具或写入会话历史。
- 顺序项完成后才推进下一项；只有显式 `parallel_group` 批量委派。
- `clarify` 可以暂停计划，回答后继续或调整。
- 有限重试失败后降级主干，保证会话可继续。
- `delegate_task` 负责真正创建轻量子 agent。
- 子 agent 默认使用 branch 系统提示词和完整 memory 上下文、无父历史，并限制
  面向用户或共享长期状态的工具能力。

这种方式保持了现有对话循环和 `delegate_task` 执行引擎，同时让命中的
复杂任务确定性地优先进入分支。当前主要限制仍是规则分类可能存在误判，
以及任务拆解和依赖调度尚未结构化为 DAG。

如果后续想进一步强化，可以考虑增加显式 planner、确定性任务拆分、分支结果结构化 schema，以及主干自动验证策略。
