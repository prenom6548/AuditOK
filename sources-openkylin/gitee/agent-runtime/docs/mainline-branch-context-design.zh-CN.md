# 主干加多分支轻量上下文设计分析

## 1. 背景

Hermes Agent 当前的一次模型请求通常由主对话循环统一构建：系统提示词、工具 schema、历史消息、记忆、技能索引、平台提示和临时上下文都会进入请求。对于主干会话，这是合理的，因为主干需要理解用户偏好、会话历史、长期记忆、可用技能和平台约束。

但在复杂任务中，很多子任务并不需要完整上下文。例如读取一个文件、验证一个命令、检索一个网页、总结一个局部错误，通常只需要任务目标、少量背景、工作目录和少量工具。如果这些子任务也携带完整 system prompt、skills、memory 和大量工具 schema，就会造成 token 膨胀、延迟升高，并让主干上下文积累过多中间噪音。

因此可以考虑一种“主干 + 多轻量分支”的结构：

- 主干 agent 负责理解用户意图、拆解任务、调度分支、验证关键结果和最终回复。
- 分支 agent 只接收精简任务包，使用最小工具集完成局部任务。
- 分支中间过程不进入主干上下文，只把结构化摘要返回主干。

## 2. 当前代码基础

项目已经具备一部分基础能力。

### 2.1 `delegate_task` 已是分支雏形

`tools/delegate_tool.py` 中的 `delegate_task` 会创建子 `AIAgent`。当前设计已经包含：

- fresh conversation：子代理不继承父会话历史。
- isolated context：父上下文只看到最终 summary，不看到子代理中间工具结果。
- restricted toolsets：子代理可使用限制后的工具集。
- blocked tools：默认禁止 `delegate_task`、`clarify`、`memory`、`send_message` 等能力，
  但保留 `execute_code`。
- parallel batch：可并行运行多个子任务。

子代理创建时已经传入：

```text
skip_context_files=False
skip_memory=False
prompt_profile="branch"
enabled_toolsets=child_toolsets
ephemeral_system_prompt=child_prompt
```

当前实现选择“能力对齐分支”：除 focused identity、父会话历史和少数具有共享
副作用的工具外，尽量与主智能体保持一致。

### 2.2 当前子代理仍会构建较完整 system prompt

`agent/system_prompt.py` 将系统提示词分为三层：

- stable：身份、工具指导、Hermes 帮助提示、skills 索引、环境提示、平台提示等。
- context：调用方 system message、AGENTS.md、`.cursorrules` 等上下文文件。
- volatile：memory、USER profile、外部 memory provider、时间、session、model、provider。

当前 branch profile 设置 `skip_context_files=False` 和 `skip_memory=False`，因此会加载
项目上下文、memory、USER profile、外部 memory provider、工具指导、skills 索引和
环境提示。这会增加 token，但适合验证更完整系统提示词是否能提高分支质量。

### 2.3 `ephemeral_system_prompt` 是追加，不是替换

在 `agent/conversation_loop.py` 中，最终 system prompt 是：

```text
active_system_prompt + ephemeral_system_prompt
```

也就是说 `delegate_task` 构建的 child prompt 不是替代 Hermes 核心 system prompt，而是追加到核心 system prompt 后面。要真正降低 token，需要让子代理可以选择不同的 system prompt profile，而不仅是追加一个精简任务提示。

### 2.4 `/branch` 不是 token 优化机制

CLI 和 TUI 中的 `/branch` 是完整会话 fork：复制当前 conversation history 到新 session，适合探索另一条对话路径，但不会天然降低 token。它和这里讨论的“轻量任务分支”不是同一类机制。

## 3. 可行性判断

该方案可行，并且和现有架构较契合。原因是：

- `AIAgent` 已支持独立实例化。
- `delegate_task` 已支持 fresh conversation、toolset 限制、并行和摘要回传。
- `skip_memory`、`skip_context_files` 已存在。
- toolsets 已经是显式分组机制，可以天然用于分支能力收缩。
- 主干消息历史和子代理消息历史已经隔离。

需要补齐的是“轻量 prompt profile”和更明确的分支上下文策略。

## 4. 主要优势

### 4.1 降低 token 成本和延迟

许多子任务不需要完整 system prompt、memory、USER profile、skills catalog 和父会话历史。轻量分支只带任务目标、必要背景和最小工具 schema，可以显著减少输入 token。

这在多子任务并发时尤其明显。一个完整主干 prompt 如果很大，复制给每个子任务会造成线性放大；轻量分支可以避免这种放大。

### 4.2 保持主干上下文清洁

如果所有工具调用都由主干完成，长文件内容、网页内容、命令输出、失败尝试都会进入主干 messages，后续每次模型调用都要携带。

分支模式下，子代理消化中间过程，主干只接收摘要、结论、证据和风险。主干更像协调器，不容易被中间细节淹没。

### 4.3 适合并行处理

复杂任务常常能拆成多个独立部分。例如：

- 分支 A 检查代码路径。
- 分支 B 查找相关测试。
- 分支 C 阅读文档或网页。
- 分支 D 验证某个命令。

这些分支可以并行运行，主干等待结果后综合判断。当前 `delegate_task` 的 batch 模式已经支持这一方向。

### 4.4 失败隔离更好

分支失败不会污染主干上下文。子代理如果跑错命令、读错文件或探索失败，主干可以重新派发、忽略或进行验证，而不是让失败轨迹长期留在主干会话中。

### 4.5 权限和能力更可控

分支可以只获得任务所需工具：

- 代码读取：`file`
- 本地验证：`terminal,file`
- 网页研究：`web`
- 浏览器交互：`browser`
- 纯推理：不传 tools

这比让所有任务都使用主干完整工具集更安全，也更节省工具 schema token。

### 4.6 有利于任务 DAG 化

主干可以拆任务，分支可以执行局部工作，orchestrator 分支可以进一步拆解，最终由主干汇总和验证。这比单线 conversation loop 更适合长任务、宽任务和多来源任务。

## 5. 主要缺点和风险

### 5.1 信息丢失

这是最大风险。子代理只返回摘要，主干看不到完整过程。如果子代理遗漏关键细节，主干可能基于不完整信息做错判断。

高风险任务尤其不能只依赖子代理自述，例如：

- 代码审查。
- 安全分析。
- 法律、医学、金融类判断。
- 需要精确引用的研究。
- 需要逐行理解的复杂 bug。

缓解方式是要求子代理返回可验证证据，如文件路径、行号、命令、URL、测试结果和关键输出片段，并由主干复核关键事实。

### 5.2 子代理缺少用户偏好

如果轻量分支完全不带 memory 和 USER profile，可能不知道用户偏好和环境事实，例如：

- 用户偏好中文。
- 用户使用 openKylin/UKUI。
- 文档目录是 `/home/ok/文档`。
- 当前项目工作目录。

缓解方式不是全量注入 memory，而是构建一个很小的 `branch essentials`，只包含任务必要事实。

### 5.3 拆任务本身有额外成本

多分支不适合所有任务。简单任务如果拆分，会多一次或多次模型调用，反而更慢更贵。

适合分支的任务通常具备以下特征：

- 上下文很大。
- 中间过程很长。
- 子任务边界清楚。
- 可以并行。
- 最终只需要摘要或证据。

不适合分支的任务包括：

- 一步工具调用。
- 小修小改。
- 需要连续互动的探索。
- 需要主干全程保持细腻语境的对话。

### 5.4 协调复杂度上升

主干需要判断：

- 是否拆分。
- 拆成几个分支。
- 每个分支拿什么上下文。
- 每个分支用哪些工具。
- 分支结果冲突时如何处理。
- 哪些结果需要主干验证。

如果策略不当，容易出现过度委派：本来两步能完成，却创建多个子代理。

### 5.5 轻量 prompt 可能丢失关键约束

Hermes 当前完整 system prompt 中包含很多行为约束，例如必须实际使用工具、不要只描述计划、路径和环境提示、skills 使用规则、memory 使用规则等。轻量分支如果删得太狠，模型可能退回到“口头承诺、不执行”的坏习惯。

因此 lean prompt 不能只是短，还必须保留最关键的执行纪律。

### 5.6 工具 schema 仍可能是大头

如果子代理仍继承大量工具，即使 system prompt 变短，工具 schema 也会继续消耗大量 token。真正节省 token 需要同时做：

- 精简 system prompt。
- 精简 toolsets。

只做 prompt profile，不限制工具，收益有限。

### 5.7 调试难度增加

多分支后，需要能追踪：

- 主干请求。
- 分支请求。
- 分支父子关系。
- 每个分支的 prompt profile。
- 每个分支的工具集。
- 每个分支 token 估算。
- 分支结果如何被主干使用。

否则问题定位会更困难。现有 session detail log 和 subagent progress 机制可以继续扩展。

### 5.8 memory 和 skill 写入要谨慎

分支默认不应写 memory 或 skills。多个子代理并发写长期记忆或技能，可能导致：

- 重复记忆。
- 错误技能沉淀。
- 并发写冲突。
- 把临时任务过程写成长期知识。

建议分支默认禁止写 memory/skill，由主干在验证后决定是否写入。

## 6. 推荐设计

### 6.1 增加 prompt profile

给 `AIAgent` 增加类似 `prompt_profile` 或 `context_policy` 的参数。

建议 profile：

| Profile | 用途 | 内容 |
|---|---|---|
| `full` | 主干 agent | 当前完整行为，包含 memory、skills、平台提示等 |
| `lean` | 默认子分支 | 简短身份、任务、工作目录、工具纪律、输出格式 |
| `tool_only` | 工具型子任务 | 任务、工具使用规则、工作目录、最小环境提示 |
| `no_tools` | 纯推理子任务 | 任务和输出格式，不传工具 |

主干默认 `full`，`delegate_task` 默认 `lean`。

### 6.2 增加分支上下文策略

子代理不继承父历史，而是接收上下文包：

```text
goal: 子任务目标
constraints: 约束
artifacts: 相关文件、URL、错误日志、命令输出摘要
workspace: 工作目录
return_contract: 期望返回字段
```

可配置策略：

| 参数 | 选项 | 说明 |
|---|---|---|
| `context_mode` | `none`, `summary`, `selected`, `inherited` | 控制分支上下文来源 |
| `memory_policy` | `off`, `essentials`, `readonly` | 控制记忆注入 |
| `skills_policy` | `off`, `selected`, `index_only` | 控制技能注入 |
| `tool_policy` | `minimal`, `explicit`, `inherit` | 控制工具集 |

默认建议：

```text
context_mode=selected
memory_policy=essentials
skills_policy=off
tool_policy=minimal
prompt_profile=lean
```

### 6.3 子代理返回结构化结果

不要只返回自然语言总结。建议返回：

```text
status
summary
facts
evidence
files_changed
commands_run
risks
needs_parent_verification
```

主干根据 `needs_parent_verification` 决定是否亲自读取文件、运行命令或复查来源。

### 6.4 分支默认最小工具集

不要默认继承父工具集。可以按任务类型推断，也可以由主干显式指定。

常见映射：

| 子任务类型 | 推荐 toolsets |
|---|---|
| 代码阅读 | `file` |
| 本地构建/测试 | `terminal,file` |
| 网页研究 | `web` |
| 浏览器交互 | `browser` |
| 文档/PPT 生成 | `terminal,file`，必要时加 `skills` |
| 纯分析 | 无工具 |

### 6.5 主干负责验证和最终写入

分支可以发现事实，但主干应该负责：

- 验证关键证据。
- 判断冲突结果。
- 决定是否修改文件。
- 决定是否写 memory。
- 决定是否沉淀 skill。
- 最终回复用户。

## 7. 分阶段落地建议

### 阶段 1：观测和指标

先不改行为，增强 detail log：

- 记录每次请求的 system prompt 估算 token。
- 记录工具 schema 估算 token。
- 记录消息历史估算 token。
- 记录子代理 toolsets 和 prompt 大小。

目标是确认 token 大头在哪里。

### 阶段 2：实现 lean prompt profile

在 `AIAgent` 增加 `prompt_profile`，在 `agent/system_prompt.py` 中按 profile 控制注入项。`delegate_task` 子代理默认使用 `lean`。

此阶段应保持主干行为不变。

### 阶段 3：收紧子代理默认工具集

调整 `delegate_task` 默认策略：不再无条件继承父工具集，而是使用更小的默认 toolsets 或要求主干显式指定。

为了兼容，可以配置开关：

```text
delegation.default_tool_policy: inherit|minimal
```

初期可默认保持 `inherit`，提供 `minimal` 试验。

### 阶段 4：结构化分支返回

让子代理 prompt 明确要求返回结构化摘要。后续可进一步让 `delegate_task` 对返回内容做解析和标准化。

### 阶段 5：DAG 和 orchestrator 策略

在已有 `role=orchestrator` 的基础上，加入更明确的策略：

- 限制最大深度。
- 限制总子任务数。
- 限制总 token 预算。
- 分支结果冲突时要求主干验证。

## 8. 适用场景和不适用场景

### 适用场景

- 大型代码库调研。
- 多文件审查。
- 并行网页研究。
- 长文档处理。
- 多步骤生成任务。
- 需要隔离中间噪音的任务。

### 不适用场景

- 一步能完成的小任务。
- 用户正在实时互动确认细节的任务。
- 需要主干完整语境连续推理的小范围问题。
- 高风险判断且无法由主干验证证据的任务。

## 9. 总体结论

“主干 + 多轻量分支”适合 Hermes Agent 当前架构，且已经有 `delegate_task` 作为基础。它的最大收益是降低 token、减少主干上下文污染、提升并行能力和隔离失败；最大风险是信息丢失、协调复杂度上升和轻量 prompt 过度删减导致行为退化。

本文最初建议分支默认轻量、无 memory、无父历史和最小工具集。当前实验实现保留
“无父历史、只回传结构化结果”，但改为 branch 完整系统提示词、
`skip_memory=False`，并允许 `execute_code`，用于验证更完整能力上下文是否能提升
分支质量。主干仍负责计划、顺序推进、关键事实验证和最终决策。
