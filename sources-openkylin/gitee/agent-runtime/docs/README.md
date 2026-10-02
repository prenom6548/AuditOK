# Hermes Agent 中文技术文档索引

本文档组基于当前仓库代码、`README.md`、`README.zh-CN.md`、`AGENTS.md` 以及 `website/docs/` 下的开发者文档整理，面向需要快速理解 Hermes Agent 架构、运行机制和功能边界的开发者。

## 文档列表

- [架构设计](./architecture-design.zh-CN.md)：项目分层、核心模块、数据流和扩展点。
- [工作原理](./working-principles.zh-CN.md)：代理循环、提示词构建、工具调用、会话、记忆、压缩和安全审批的原理。
- [工作流程](./workflows.zh-CN.md)：CLI、TUI、网关、Cron、ACP、插件、技能等主路径流程。
- [详细工作流程](./detailed-workflows.zh-CN.md)：从启动到模型调用、工具调度、消息平台处理、会话保存的细粒度步骤。
- [功能清单](./features.zh-CN.md)：按核心、自动化、媒体 Web、集成、定制、安全和开发能力整理项目功能。
- [关键技术文档](./key-technical-docs.zh-CN.md)：仓库内关键源文件和上游文档的阅读路线。
- [主干加多分支轻量上下文设计分析](./mainline-branch-context-design.zh-CN.md)：分析用轻量子分支降低 token、隔离上下文和并行执行的可行性、优缺点与方案。
- [主干-分支当前实现说明](./mainline-branch-current-implementation.zh-CN.md)：说明当前 router hint、`delegate_task`、lean 子代理的实际触发链路和优势边界。

## 推荐阅读顺序

1. 先读 [架构设计](./architecture-design.zh-CN.md)，建立项目全局视图。
2. 再读 [工作原理](./working-principles.zh-CN.md)，理解一次对话如何完成。
3. 根据入口类型阅读 [工作流程](./workflows.zh-CN.md) 和 [详细工作流程](./detailed-workflows.zh-CN.md)。
4. 查能力边界时阅读 [功能清单](./features.zh-CN.md)。
5. 需要继续深挖代码时阅读 [关键技术文档](./key-technical-docs.zh-CN.md)。
