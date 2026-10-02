"""Routing decisions for mainline-to-branch orchestration.

This module deliberately avoids model calls.  It detects tasks that are likely
to benefit from ``delegate_task`` and tells the conversation loop whether the
decision is advisory or must be enforced before normal tool execution begins.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Any, Dict, Iterable, Optional


_EXPLICIT_DELEGATION_RE = re.compile(
    r"(delegate_task|subagent|子\s*agent|子代理|子智能体|主干[-—]?分支|主干分支|分支方式|分支并行)",
    re.IGNORECASE,
)

_PARALLEL_RE = re.compile(
    r"(并行|同时|分头|分开|分别|多路|parallel|in parallel|split|fan[- ]?out)",
    re.IGNORECASE,
)

_MULTI_ANGLE_RE = re.compile(
    r"(多个方向|多个角度|几个方向|几个角度|从.+角度|分别.+(?:分析|检查|调研|研究|对比|比较|review|审查))",
    re.IGNORECASE,
)

_HEAVY_TASK_RE = re.compile(
    r"(调研|研究|对比|比较|审查|review|debug|排查|定位|分析原因|根因|方案|架构|性能|安全|测试失败|日志|多次会话)",
    re.IGNORECASE,
)

_IMPLEMENTATION_TASK_RE = re.compile(
    r"(实现|开发|修改|改造|新增|添加|重构|修复|迁移|升级|集成|部署|编写|"
    r"implement|develop|modify|refactor|fix|migrat(?:e|ion)|upgrade|integrat(?:e|ion)|deploy)",
    re.IGNORECASE,
)

_COMPOUND_TASK_RE = re.compile(
    r"(并且|同时|然后|以及|还要|并验证|并测试|并修复|and\s+(?:then|also)|as\s+well\s+as)",
    re.IGNORECASE,
)

_TRIVIAL_CHANGE_RE = re.compile(
    r"((?:一个|1\s*个)?(?:拼写|错别字|标点)(?:错误)?|纯?格式化|"
    r"formatting\s+only|typo|whitespace\s+only)",
    re.IGNORECASE,
)

_SIMPLE_TASK_RE = re.compile(
    r"^\s*(?:(?:看下|查看|打开|搜索|运行|执行|读取|列出|保存|记住|更新记忆|帮我记住|帮我更新)"
    r"|(?:open|read|list|run|save|remember)\b)",
    re.IGNORECASE,
)

_DIRECT_APP_ACTION_RE = re.compile(
    r"^\s*(?:(?:请|帮我|麻烦你?)\s*)?"
    r"(?:打开|启动|关闭|退出|切换到|open|launch|start|close|quit)",
    re.IGNORECASE,
)

_DIRECT_MESSAGE_ACTION_RE = re.compile(
    r"^\s*(?:(?:请|帮我|麻烦你?)\s*)?"
    r"(?:(?:使用|通过)\s*(?:微信|WeChat|企业微信|钉钉|飞书|QQ|短信|邮件)\s*)?"
    r"(?:(?:给|向)\s*[^，。,；;:\n]{1,48}\s*)?"
    r"(?:发送消息|发消息|发送一条消息|发一条消息|发送|send\s+(?:a\s+)?message)",
    re.IGNORECASE,
)

_SIMPLE_QA_RE = re.compile(
    r"^\s*(?:(?:你好|您好|嗨|谢谢|多谢|再见|什么是|谁是|哪里是|何时|多少|"
    r"怎么读|如何理解|请解释|解释一下)|(?:hello|hi|thanks|thank\s+you|bye|"
    r"what\s+is|who\s+is|where\s+is|when\s+is|how\s+many|define|explain)\b)",
    re.IGNORECASE,
)


@dataclass(frozen=True)
class DelegationRouterDecision:
    should_hint: bool
    score: int
    reason: str
    hint: str = ""
    must_delegate: bool = False


def _cfg_bool(cfg: Dict[str, Any], key: str, default: bool) -> bool:
    raw = cfg.get(key, default)
    if isinstance(raw, bool):
        return raw
    if isinstance(raw, str):
        return raw.strip().lower() in {"1", "true", "yes", "on", "always"}
    return bool(raw)


def _cfg_int(cfg: Dict[str, Any], key: str, default: int) -> int:
    try:
        return int(cfg.get(key, default))
    except (TypeError, ValueError):
        return default


def _has_delegate_tool(valid_tool_names: Iterable[str]) -> bool:
    return "delegate_task" in set(valid_tool_names or [])


def _looks_like_simple_qa(text: str) -> bool:
    """Conservatively recognize short conversational or factual questions.

    This is intentionally a narrow allow-list. Unknown requests default to
    orchestration so missing a complexity keyword cannot silently bypass the
    branch path.
    """
    if len(text) > 160:
        return False
    if _PARALLEL_RE.search(text) or _COMPOUND_TASK_RE.search(text):
        return False
    if _IMPLEMENTATION_TASK_RE.search(text):
        return False
    if _SIMPLE_QA_RE.search(text):
        return True
    return bool(
        text.rstrip().endswith(("?", "？"))
        and not _HEAVY_TASK_RE.search(text)
        and not _MULTI_ANGLE_RE.search(text)
        and not _EXPLICIT_DELEGATION_RE.search(text)
    )


def _looks_like_direct_single_action(text: str) -> bool:
    """Recognize short, explicit one-action UI/messaging requests."""
    if len(text) > 300:
        return False
    if (
        _PARALLEL_RE.search(text)
        or _COMPOUND_TASK_RE.search(text)
        or _MULTI_ANGLE_RE.search(text)
        or _HEAVY_TASK_RE.search(text)
        or _IMPLEMENTATION_TASK_RE.search(text)
    ):
        return False
    return bool(
        _DIRECT_APP_ACTION_RE.search(text)
        or _DIRECT_MESSAGE_ACTION_RE.search(text)
    )


def decide_delegation_hint(
    user_message: str,
    *,
    valid_tool_names: Iterable[str],
    prompt_profile: str = "full",
    config: Optional[Dict[str, Any]] = None,
) -> DelegationRouterDecision:
    """Return an API-time routing hint when delegation is likely useful."""
    cfg = config or {}
    if not _cfg_bool(cfg, "enabled", True):
        return DelegationRouterDecision(False, 0, "disabled")
    router_enabled = _cfg_bool(cfg, "router_enabled", False)
    if not router_enabled:
        return DelegationRouterDecision(False, 0, "disabled")
    # An explicit router_enabled=true remains a complete opt-in for older
    # configs/tests that predate mainline_branch_enabled. Shipped defaults set
    # both flags false, so ordinary installations never enter this path.
    if not _cfg_bool(cfg, "mainline_branch_enabled", router_enabled):
        return DelegationRouterDecision(False, 0, "mainline_branch_disabled")
    if str(prompt_profile or "full").strip().lower() == "lean":
        return DelegationRouterDecision(False, 0, "lean_profile")
    if not _has_delegate_tool(valid_tool_names):
        return DelegationRouterDecision(False, 0, "delegate_tool_unavailable")
    if not isinstance(user_message, str) or not user_message.strip():
        return DelegationRouterDecision(False, 0, "empty_message")

    text = user_message.strip()
    if _looks_like_direct_single_action(text):
        return DelegationRouterDecision(False, 0, "direct_single_action")
    if (
        _TRIVIAL_CHANGE_RE.search(text)
        and not _PARALLEL_RE.search(text)
        and not _COMPOUND_TASK_RE.search(text)
        and not _EXPLICIT_DELEGATION_RE.search(text)
    ):
        return DelegationRouterDecision(False, 0, "simple_task")

    score = 0
    reasons: list[str] = []

    if _EXPLICIT_DELEGATION_RE.search(text):
        score += 4
        reasons.append("explicit_delegation")
    if _PARALLEL_RE.search(text):
        score += 2
        reasons.append("parallel_language")
    if _MULTI_ANGLE_RE.search(text):
        score += 2
        reasons.append("multi_angle")
    if _HEAVY_TASK_RE.search(text):
        score += 1
        reasons.append("heavy_task")

    if _IMPLEMENTATION_TASK_RE.search(text):
        score += 3
        reasons.append("implementation_task")
    if _COMPOUND_TASK_RE.search(text):
        score += 1
        reasons.append("compound_task")

    if len(text) >= 80 and ("、" in text or "," in text or "，" in text):
        score += 1
        reasons.append("long_multi_part")

    if (
        _SIMPLE_TASK_RE.search(text)
        and not _PARALLEL_RE.search(text)
        and not _MULTI_ANGLE_RE.search(text)
        and not _HEAVY_TASK_RE.search(text)
        and not _IMPLEMENTATION_TASK_RE.search(text)
        and not _COMPOUND_TASK_RE.search(text)
        and not _EXPLICIT_DELEGATION_RE.search(text)
    ):
        return DelegationRouterDecision(False, score, "simple_task")

    min_score = max(1, _cfg_int(cfg, "router_min_score", 3))
    if score < min_score:
        if _looks_like_simple_qa(text):
            return DelegationRouterDecision(False, score, "simple_qa")
        if not _cfg_bool(
            cfg,
            "orchestrate_uncertain_tasks",
            router_enabled,
        ):
            return DelegationRouterDecision(False, score, "score_below_threshold")
        reasons.append("uncertain_defaults_to_orchestration")

    reason = ",".join(reasons) or "matched"
    must_delegate = _cfg_bool(cfg, "enforce_complex_tasks", router_enabled)
    if must_delegate:
        hint = (
            "[Mainline orchestration requirement: This request was classified "
            f"as a complex task ({reason}). Before doing substantive work, you "
            "MUST call `todo` and create an ordered execution plan. Keep one "
            "item in_progress unless independent items share a parallel_group. "
            "Assign substantive items to executor=branch; reserve "
            "executor=mainline for coordination, parent verification, user "
            "interaction, direct actions, and external side effects. "
            "Then execute and update the plan until every required item is "
            "completed. Use a batch `delegate_task(tasks=[...])` only for "
            "independent parallel items; execute dependent items sequentially.]"
        )
    else:
        hint = (
            "[Delegation router hint: This user request appears suitable for the "
            f"`delegate_task` subagent path ({reason}). If the work can be split "
            "into independent subtasks, call `delegate_task` early with a concise "
            "goal or a small `tasks` array. Pass all needed context explicitly. "
            "If it is actually a simple single-tool action, ignore this hint and "
            "proceed directly.]"
        )
    return DelegationRouterDecision(True, score, reason, hint, must_delegate)
