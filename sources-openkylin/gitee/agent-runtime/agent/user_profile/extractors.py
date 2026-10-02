"""Replaceable conversation-to-profile extraction adapters."""

from __future__ import annotations

import hashlib
import json
import os
from typing import Literal, Protocol, Sequence

from .models import ExtractionBatch, InteractionMessage, ProfileUpdate, UserProfile
from .schema import schema_for_prompt


DEFAULT_MAX_CHUNK_CHARS = 48_000
_MESSAGE_OVERHEAD_CHARS = 32
_CHUNK_OVERLAP_CHARS = 256
_CONVERSATION_OVERLAP_CHARS = 2_000
_MIN_CONSOLIDATION_CHARS = 4_000


class ProfileExtractor(Protocol):
    def extract(
        self,
        user_id: str,
        messages: Sequence[InteractionMessage],
        current_profile: UserProfile,
    ) -> ExtractionBatch: ...


SYSTEM_INSTRUCTIONS = """\
你是一个会话结束后运行的“模板驱动用户画像抽取器”，不是聊天助手。
你的唯一任务是：从用户消息中抽取用户画像更新。

你必须严格依据下面提供的允许字段 schema 工作。
schema 中每个字段的描述是最高优先级规则，不得自行扩展字段含义。

========================
核心原则
========================

1. 只允许输出 schema 中存在的 path。
禁止创建新字段、旧字段、近义字段。

2. 只能从用户本人消息中抽取信息。
禁止使用：
- Agent 的回答
- Agent 的推测
- 示例中的人物
- 用户引用的第三方信息
- 假设场景中的人物

必须区分：
用户本人：
“我是程序员”
可以记录

第三方：
“我的朋友是医生”
只能记录 life.relations（如果与任务相关），不能记录 occupation

假设：
“如果我是学生怎么办”
不能记录。

3. evidence_quote 必须来自用户原文。
不得改写，不得总结。

4. explicit 规则：

explicit=true:
- 用户直接陈述
- 用户明确确认
- 用户明确纠正
- 用户明确选择/拒绝

explicit=false:
- 只能通过上下文合理推断

对于 inferred 内容：
confidence 必须降低。

5. 不进行未经允许的人格、能力、身份推断。

禁止：
- 根据语言水平推断教育程度
- 根据代码问题推断职业
- 根据一次任务推断长期技能
- 根据一次购买咨询推断消费习惯
- 根据聊天风格推断人格

6. 用户删除或拒绝保存信息：

如果用户明确要求：
“不记住这个”
“删除这个”
“不要保存”

输出：

{{
 action:"delete",
 explicit:true
}}

7. 用户表达隐私边界：

例如：
“不要推测我的位置”
“不要保存健康信息”
“以后涉及付款必须确认”

只能写入：

boundaries.*

且：
- explicit 必须=true
- value 必须是 schema 中允许 path 数组或字段要求的结构

========================
字段选择规则
========================

按以下顺序判断：

------------------------
A. 基本事实
------------------------

长期稳定事实：

姓名、年龄、语言、教育：
basic.*

居住：
life.residence.*

职业：
occupation.*

不要把短期任务升级为职业。

例如：

“我正在学习Python”
不是：
occupation.title

可能是：
interests.short_term.topics


------------------------
B. 职业、技能、经验区分
------------------------

严格区分：

background.domains:
用户了解什么

例如：
“我研究机器学习”

background.skills:
用户能执行什么

例如：
“我会使用PyTorch训练模型”

background.experience:
用户做过什么

例如：
“我参与过自动驾驶项目”

不要混用。


------------------------
C. 兴趣与任务区分
------------------------

长期兴趣：

interests.long_term.*

需要：
- 多次出现
- 明确长期喜欢
- 用户主动表达长期关注


近期关注：

interests.short_term.*

用于：
- 当前研究主题
- 最近学习内容
- 最近准备购买
- 最近计划


一次性任务：

context.*

例如：
“帮我写一个论文摘要”

进入：
context.tasks

不要进入兴趣。


------------------------
D. 偏好判断
------------------------

preferences.items:

记录用户明确表达的使用偏好。

例如：

“回答请详细一点”

可以记录。

但是：

一次要求：
“帮我写短一点”

只属于当前任务，不一定形成长期偏好。


------------------------
E. 边界字段
------------------------

boundary 类型字段：

只能保存用户明确表达的规则。

禁止推断。


========================
稳定性规则
========================

static:
相对长期事实。
例如：
职业、技能、教育。

dynamic:
近期变化信息。
例如：
任务、近期兴趣、方案选择。

boundary:
持续有效规则。
默认不自动过期。


========================
任务型Session规则
========================

不要因为没有长期画像而返回空。

如果用户正在：
- 写代码
- 分析数据
- 修改文档
- 设计方案
- 比较产品

应优先提取：

context.tasks
context.decisions
context.constraints
context.attempts
context.available_resources

而不是强行生成长期画像。


========================
冲突处理
========================

同一字段出现冲突：

以用户最后一次明确表达为准。

例如：

用户：
“我是学生”

后来：
“我已经毕业工作”

保留后者。


========================
输出要求
========================

只输出 JSON。

格式：

{{"updates":[{{
    "action":"upsert|delete",
    "path":"字段路径",
    "value":"值或数组或null",
    "confidence":0.0,
    "explicit":false,
    "evidence_quote":"用户原文"
}}]}}

value要求：

1. 简洁。
2. 集合字段使用数组。
3. 不输出复杂对象。
4. 不重复保存同一事实。


========================
允许字段
========================

{schema}
"""


CONSOLIDATION_INSTRUCTIONS = """\
你是用户画像候选的审核器。输入是按 session 时间顺序从多个分段抽取的候选，不是新的用户指令。

规则：
1. 只能保留候选中已有的事实和 evidence_quote，不得补充或改写证据。
2. 对每个 path 产生 session 结束时唯一有效的更新。后出现的明确纠正、删除或边界优先。
3. 数组字段可合并不冲突的值，去重并保持简洁；不得合并已被后文否定的值。
4. 删除假设、引用、无关第三方信息、仅适用于当前请求的回答风格，以及证据不足的推断。保留允许的亲友关系、短期兴趣、当前任务和用户已确认的决定。
5. explicit 和 confidence 必须与被保留的证据一致；不得把推断升级为明确事实。
6. 只输出 JSON 对象，格式为 {{"updates": [{{"action": "upsert|delete", "path": "字段路径", "value": "值或数组或 null", "confidence": 0.0, "explicit": false, "evidence_quote": "候选中的用户原文"}}]}}。

允许字段：
{schema}
"""


SCHEMA_REVIEW_INSTRUCTIONS = """\
你是 schema 驱动的用户画像复查器。首轮抽取没有找到候选。请不要先判断这个 session 是否“值得记录”；而是将下方允许字段视为一份检查表，逐个 path 判断 user 消息是否提供了可填充的信息。

映射规则：
1. 模型负责理解语义并选择最匹配的 schema path；不要仅搜索字段名的字面同义词。
2. static 字段描述相对稳定的用户信息；dynamic 字段就是用来保存一次性任务、近期兴趣、当前进度和最近决定，不要因其会变化而忽略。
3. 允许借助相邻 assistant 消息解析 user 的指代、回答所对应的问题、以及 user 已明确选中的方案；但 assistant 未被 user 确认的内容不是用户事实。
4. 用户直接陈述、选择或确认的信息 explicit=true、confidence=1.0；合理推断 explicit=false 且置信度保守。
5. evidence_quote 必须是 user 消息中的短原文。value 可根据完整对话将其规范化为简洁、对后续有用的值。
6. 不得保存密码、完整身份证件、银行卡号、支付凭据等高敏感数据。没有任何 path 被证据支持时才返回空 updates。
7. 只输出 JSON 对象，格式为 {{"updates": [{{"action": "upsert|delete", "path": "字段路径", "value": "值或数组或 null", "confidence": 0.0, "explicit": false, "evidence_quote": "用户原文"}}]}}。

允许字段：
{schema}
"""


PROFILE_PRECHECK_INSTRUCTIONS = """\
你是用户画像更新预判器。输入包含 current_profile（已有用户画像）和 messages（本次 session）。判断 messages 是否会使 current_profile 发生有效变化。

以下情况回答“是”：出现已有画像未包含的新信息；用户纠正或删除已有信息；用户设定新的隐私边界；已有推断被用户明确确认从而应提升来源或置信度；动态信息（当前任务、近期兴趣、已选方案等）确实发生变化。
以下情况回答“否”：session 只重复已有画像且没有增强证据；只有寒暄、确认收到；只是查询外部信息且没有提供用户自身的新信息；消息内容与已有画像相比不会产生任何新增、修改、删除或置信度升级。

只输出一个字：是 或 否。不要输出解释、标点或其他内容。
"""


class OpenAICompatibleProfileExtractor:
    """Provider-neutral adapter for APIs compatible with the OpenAI Python SDK."""

    def __init__(
        self,
        model: str,
        client: object | None = None,
        base_url: str | None = None,
        proxy_url: str | None = None,
        trust_env: bool = True,
        api_mode: Literal["auto", "responses", "chat_completions"] = "chat_completions",
        api_key: str | None = None,
        max_chunk_chars: int = DEFAULT_MAX_CHUNK_CHARS,
    ) -> None:
        if not model.strip():
            raise ValueError("LLM model must be configured")
        if api_mode not in {"auto", "responses", "chat_completions"}:
            raise ValueError(f"unsupported OpenAI API mode: {api_mode}")
        if max_chunk_chars < 1:
            raise ValueError("max_chunk_chars must be positive")
        if client is None:
            import httpx
            from openai import OpenAI

            try:
                http_client = (
                    httpx.Client(proxy=proxy_url, trust_env=False)
                    if proxy_url
                    else httpx.Client(trust_env=trust_env)
                )
            except ImportError as exc:
                if proxy_url and proxy_url.lower().startswith("socks"):
                    raise RuntimeError(
                        "SOCKS 代理需要额外依赖，请运行 pip install 'httpx[socks]'，"
                        "或将 KYLIN_PROFILE_LLM_PROXY_URL 改为 HTTP 代理地址"
                    ) from exc
                raise
            client = OpenAI(api_key=api_key, base_url=base_url, http_client=http_client)
        self.model = model
        self.client = client
        self.api_mode = api_mode
        self.max_chunk_chars = max_chunk_chars

    @classmethod
    def from_environment(cls) -> "OpenAICompatibleProfileExtractor":
        api_key = _environment_value("KYLIN_PROFILE_LLM_API_KEY", "OPENAI_API_KEY")
        if not api_key:
            raise RuntimeError(
                "missing KYLIN_PROFILE_LLM_API_KEY (or legacy OPENAI_API_KEY)"
            )
        model = _environment_value("KYLIN_PROFILE_LLM_MODEL", "OPENAI_PROFILE_MODEL")
        if not model:
            raise RuntimeError(
                "missing KYLIN_PROFILE_LLM_MODEL (or legacy OPENAI_PROFILE_MODEL)"
            )
        base_url = _environment_value("KYLIN_PROFILE_LLM_BASE_URL", "OPENAI_BASE_URL")
        trust_env = _environment_boolean_alias(
            "KYLIN_PROFILE_LLM_TRUST_ENV", "OPENAI_TRUST_ENV", default=True
        )
        proxy_url = _configured_proxy() if trust_env else None
        api_mode = (
            _environment_value("KYLIN_PROFILE_LLM_API_MODE", "OPENAI_API_MODE")
            or "chat_completions"
        ).strip().lower()
        if api_mode not in {"auto", "responses", "chat_completions"}:
            raise RuntimeError(
                "KYLIN_PROFILE_LLM_API_MODE must be auto, responses, or "
                "chat_completions"
            )
        max_chunk_chars = _environment_integer(
            "KYLIN_PROFILE_LLM_CHUNK_CHARS", default=DEFAULT_MAX_CHUNK_CHARS
        )
        return cls(
            model=model,
            api_key=api_key,
            base_url=base_url,
            proxy_url=proxy_url,
            trust_env=trust_env and proxy_url is None,
            api_mode=api_mode,  # type: ignore[arg-type]
            max_chunk_chars=max_chunk_chars,
        )

    def extract(
        self,
        user_id: str,
        messages: Sequence[InteractionMessage],
        current_profile: UserProfile,
    ) -> ExtractionBatch:
        user_messages = [message for message in messages if message.role == "user"]
        if not user_messages:
            return ExtractionBatch()

        user_texts = [message.content for message in user_messages]
        chunks = [
            chunk
            for chunk in _chunk_messages(messages, self.max_chunk_chars)
            if any(message.role == "user" for message in chunk)
        ]
        candidates = []
        recovery_inputs = []
        for chunk_index, chunk in enumerate(chunks):
            payload = {
                "session_part": {
                    "index": chunk_index + 1,
                    "total": len(chunks),
                    "is_final": chunk_index == len(chunks) - 1,
                },
                "current_profile": _profile_payload(current_profile),
                "messages": [message.model_dump() for message in chunk],
            }
            parsed = self._request_extraction(user_id, payload, SYSTEM_INSTRUCTIONS)
            if parsed is None:
                raise RuntimeError("LLM returned no structured profile extraction")
            chunk_user_texts = [
                message.content for message in chunk if message.role == "user"
            ]
            recovery_inputs.append((payload, chunk_user_texts))
            validated = _validated_updates(parsed, chunk_user_texts)
            candidates.extend(validated)

        # A second, task-focused pass is only used when the complete first pass
        # found nothing, avoiding extra calls for normally productive sessions.
        if not candidates and len(user_messages) > 1:
            for payload, chunk_user_texts in recovery_inputs:
                recovered = self._request_extraction(
                    user_id, payload, SCHEMA_REVIEW_INSTRUCTIONS
                )
                if recovered is None:
                    raise RuntimeError("LLM returned no task profile review")
                candidates.extend(_validated_updates(recovered, chunk_user_texts))

        candidates = _deduplicate_updates(candidates)
        if len(chunks) == 1 or not candidates:
            return ExtractionBatch(updates=candidates)

        return ExtractionBatch(
            updates=self._consolidate_candidates(
                user_id, candidates, current_profile, user_texts
            )
        )

    def should_extract(
        self,
        user_id: str,
        messages: Sequence[InteractionMessage],
        current_profile: UserProfile | None = None,
    ) -> bool | None:
        """Run a one-token gate before the more expensive structured extraction.

        ``None`` means the model response was malformed. Callers should fail
        open in that case so a transient provider/model behavior cannot lose a
        potentially useful profile update.
        """

        user_messages = [message for message in messages if message.role == "user"]
        if not user_messages:
            return False
        payload = json.dumps(
            {
                "current_profile": (
                    _profile_payload(current_profile) if current_profile else {}
                ),
                "messages": [message.model_dump() for message in messages],
            },
            ensure_ascii=False,
        )
        if self.api_mode == "chat_completions":
            answer = self._request_chat_precheck(payload)
        else:
            try:
                answer = self._request_responses_precheck(user_id, payload)
            except Exception as exc:
                if self.api_mode != "auto" or not _is_unsupported_endpoint(exc):
                    raise
                answer = self._request_chat_precheck(payload)
        return _parse_precheck_answer(answer)

    def _request_chat_precheck(self, payload: str) -> str:
        kwargs = {
            "model": self.model,
            "messages": [
                {"role": "system", "content": PROFILE_PRECHECK_INSTRUCTIONS},
                {"role": "user", "content": payload},
            ],
            **_precheck_token_limit(self.model),
        }
        completion = self.client.chat.completions.create(**kwargs)  # type: ignore[attr-defined]
        content = completion.choices[0].message.content
        if not content:
            raise RuntimeError("Chat Completions returned empty profile precheck")
        return str(content)

    def _request_responses_precheck(self, user_id: str, payload: str) -> str:
        response = self.client.responses.create(  # type: ignore[attr-defined]
            model=self.model,
            instructions=PROFILE_PRECHECK_INSTRUCTIONS,
            input=payload,
            max_output_tokens=1,
            store=False,
            safety_identifier=hashlib.sha256(user_id.encode("utf-8")).hexdigest(),
        )
        content = getattr(response, "output_text", None)
        if not content:
            raise RuntimeError("Responses returned empty profile precheck")
        return str(content)

    def _consolidate_candidates(
        self,
        user_id: str,
        candidates: list[ProfileUpdate],
        current_profile: UserProfile,
        user_texts: Sequence[str],
    ) -> list[ProfileUpdate]:
        consolidation_budget = max(
            self.max_chunk_chars, _MIN_CONSOLIDATION_CHARS
        )
        candidates_by_path: dict[str, list[ProfileUpdate]] = {}
        for candidate in candidates:
            candidates_by_path.setdefault(candidate.path, []).append(candidate)

        path_groups = []
        for path_candidates in candidates_by_path.values():
            while _updates_size(path_candidates) > consolidation_budget:
                reduced = []
                for group in _chunk_updates(path_candidates, consolidation_budget):
                    reviewed = self._review_candidates(
                        user_id, group, current_profile, user_texts
                    )
                    reduced.extend(_latest_update_per_path(reviewed))
                if len(reduced) >= len(path_candidates):
                    # A model may echo every same-path candidate; retain the latest
                    # validated result so hierarchical review always converges.
                    reduced = reduced[-1:]
                path_candidates = reduced
                if not path_candidates:
                    break
            if path_candidates:
                path_groups.append(path_candidates)

        final_updates = []
        for group in _pack_path_groups(path_groups, consolidation_budget):
            final_updates.extend(
                self._review_candidates(
                    user_id, group, current_profile, user_texts
                )
            )
        return _latest_update_per_path(final_updates)

    def _review_candidates(
        self,
        user_id: str,
        candidates: list[ProfileUpdate],
        current_profile: UserProfile,
        user_texts: Sequence[str],
    ) -> list[ProfileUpdate]:
        payload = {
            "current_profile": _profile_payload(current_profile),
            "candidates": [
                {"sequence": index + 1, **update.model_dump()}
                for index, update in enumerate(candidates)
            ],
        }
        consolidated = self._request_extraction(
            user_id, payload, CONSOLIDATION_INSTRUCTIONS
        )
        if consolidated is None:
            raise RuntimeError("LLM returned no consolidated profile extraction")
        return _validated_consolidated_updates(
            consolidated, candidates, user_texts
        )

    def _request_extraction(
        self,
        user_id: str,
        payload: dict[str, object],
        instructions: str,
    ) -> ExtractionBatch | None:
        if self.api_mode == "chat_completions":
            return self._request_chat_completions(payload, instructions)
        try:
            return self._request_responses(user_id, payload, instructions)
        except Exception as exc:
            if self.api_mode != "auto" or not _is_unsupported_endpoint(exc):
                raise
            return self._request_chat_completions(payload, instructions)

    def _request_responses(
        self,
        user_id: str,
        payload: dict[str, object],
        instructions: str,
    ) -> ExtractionBatch | None:
        response = self.client.responses.parse(  # type: ignore[attr-defined]
            model=self.model,
            instructions=instructions.format(schema=schema_for_prompt()),
            input=json.dumps(payload, ensure_ascii=False),
            text_format=ExtractionBatch,
            store=False,
            safety_identifier=hashlib.sha256(user_id.encode("utf-8")).hexdigest(),
        )
        return response.output_parsed

    def _request_chat_completions(
        self, payload: dict[str, object], instructions: str
    ) -> ExtractionBatch:
        completion = self.client.chat.completions.create(  # type: ignore[attr-defined]
            model=self.model,
            messages=[
                {
                    "role": "system",
                    "content": instructions.format(schema=schema_for_prompt()),
                },
                {
                    "role": "user",
                    "content": json.dumps(payload, ensure_ascii=False),
                },
            ],
            response_format={"type": "json_object"},
        )
        content = completion.choices[0].message.content
        if not content:
            raise RuntimeError("Chat Completions returned empty profile extraction")
        try:
            return ExtractionBatch.model_validate_json(content)
        except ValueError as exc:
            raise RuntimeError("Chat Completions returned invalid profile JSON") from exc


def _profile_payload(profile: UserProfile) -> dict[str, object]:
    return {
        path: {
            "value": entry.value,
            "source": entry.source,
            "confidence": entry.confidence,
        }
        for path, entry in profile.entries.items()
    }


def _parse_precheck_answer(answer: str) -> bool | None:
    """Accept only an unambiguous yes/no token from the precheck model."""

    normalized = answer.strip().casefold().strip("`'\"。.!！?？")
    if normalized in {"是", "yes", "y"}:
        return True
    if normalized in {"否", "no", "n"}:
        return False
    return None


def _precheck_token_limit(model: str) -> dict[str, int]:
    """Use the parameter required by newer reasoning-model Chat APIs."""

    model_name = model.rsplit("/", 1)[-1].casefold()
    if model_name.startswith(("gpt-5", "o1", "o3", "o4")):
        return {"max_completion_tokens": 1}
    return {"max_tokens": 1}


def _chunk_messages(
    messages: Sequence[InteractionMessage], max_chars: int
) -> list[list[InteractionMessage]]:
    """Split a session without silently dropping oversized individual messages."""

    pieces: list[InteractionMessage] = []
    content_budget = max(1, max_chars - _MESSAGE_OVERHEAD_CHARS)
    overlap = min(_CHUNK_OVERLAP_CHARS, content_budget // 8)
    step = max(1, content_budget - overlap)
    for message in messages:
        if len(message.content) <= content_budget:
            pieces.append(message)
            continue
        start = 0
        while start < len(message.content):
            pieces.append(
                InteractionMessage(
                    role=message.role,
                    content=message.content[start : start + content_budget],
                )
            )
            if start + content_budget >= len(message.content):
                break
            start += step

    chunks: list[list[InteractionMessage]] = []
    current: list[InteractionMessage] = []
    current_chars = 0
    for message in pieces:
        message_chars = len(message.content) + _MESSAGE_OVERHEAD_CHARS
        if current and current_chars + message_chars > max_chars:
            chunks.append(current)
            current = _conversation_tail(current, message_chars, max_chars)
            current_chars = sum(
                len(item.content) + _MESSAGE_OVERHEAD_CHARS for item in current
            )
        current.append(message)
        current_chars += message_chars
    if current:
        chunks.append(current)
    return chunks


def _conversation_tail(
    messages: Sequence[InteractionMessage], next_message_chars: int, max_chars: int
) -> list[InteractionMessage]:
    """Carry nearby turns forward when they fit, preserving question/answer context."""

    overlap_budget = min(_CONVERSATION_OVERLAP_CHARS, max_chars // 5)
    tail: list[InteractionMessage] = []
    tail_chars = 0
    for message in reversed(messages):
        message_chars = len(message.content) + _MESSAGE_OVERHEAD_CHARS
        if (
            tail_chars + message_chars > overlap_budget
            or tail_chars + message_chars + next_message_chars > max_chars
        ):
            break
        tail.insert(0, message)
        tail_chars += message_chars
    return tail


def _validated_updates(
    batch: ExtractionBatch, user_texts: Sequence[str]
) -> list[ProfileUpdate]:
    return [
        update
        for update in batch.updates
        if update.evidence_quote.strip()
        and any(update.evidence_quote.strip() in text for text in user_texts)
    ]


def _deduplicate_updates(updates: Sequence[ProfileUpdate]) -> list[ProfileUpdate]:
    deduplicated: list[ProfileUpdate] = []
    seen: set[str] = set()
    for update in updates:
        key = update.model_dump_json()
        if key not in seen:
            deduplicated.append(update)
            seen.add(key)
    return deduplicated


def _chunk_updates(
    updates: Sequence[ProfileUpdate], max_chars: int
) -> list[list[ProfileUpdate]]:
    chunks: list[list[ProfileUpdate]] = []
    current: list[ProfileUpdate] = []
    current_chars = 0
    for update in updates:
        update_chars = len(update.model_dump_json()) + _MESSAGE_OVERHEAD_CHARS
        if current and current_chars + update_chars > max_chars:
            chunks.append(current)
            current = []
            current_chars = 0
        current.append(update)
        current_chars += update_chars
    if current:
        chunks.append(current)
    return chunks


def _updates_size(updates: Sequence[ProfileUpdate]) -> int:
    return sum(
        len(update.model_dump_json()) + _MESSAGE_OVERHEAD_CHARS for update in updates
    )


def _pack_path_groups(
    path_groups: Sequence[list[ProfileUpdate]], max_chars: int
) -> list[list[ProfileUpdate]]:
    packed: list[list[ProfileUpdate]] = []
    current: list[ProfileUpdate] = []
    current_chars = 0
    for path_group in path_groups:
        group_chars = _updates_size(path_group)
        if current and current_chars + group_chars > max_chars:
            packed.append(current)
            current = []
            current_chars = 0
        current.extend(path_group)
        current_chars += group_chars
    if current:
        packed.append(current)
    return packed


def _latest_update_per_path(updates: Sequence[ProfileUpdate]) -> list[ProfileUpdate]:
    latest_indexes: dict[str, int] = {}
    for index, update in enumerate(updates):
        latest_indexes[update.path] = index
    return [
        update
        for index, update in enumerate(updates)
        if latest_indexes[update.path] == index
    ]


def _validated_consolidated_updates(
    batch: ExtractionBatch,
    candidates: Sequence[ProfileUpdate],
    user_texts: Sequence[str],
) -> list[ProfileUpdate]:
    """Reject facts or evidence introduced only by the consolidation call."""

    validated = []
    for update in _validated_updates(batch, user_texts):
        supporting = [
            candidate
            for candidate in candidates
            if candidate.path == update.path and candidate.action == update.action
        ]
        if not supporting:
            continue
        evidence_supporting = [
            candidate
            for candidate in supporting
            if candidate.evidence_quote.strip() == update.evidence_quote.strip()
        ]
        if not evidence_supporting:
            continue
        contributors = supporting
        if update.action == "upsert":
            contributors = _value_contributors(update.value, supporting)
            if not contributors or not _value_supported(update.value, contributors):
                continue
        if update.explicit and not all(candidate.explicit for candidate in contributors):
            continue
        if update.confidence > min(candidate.confidence for candidate in contributors):
            continue
        validated.append(update)
    return validated


def _value_supported(
    value: object, candidates: Sequence[ProfileUpdate]
) -> bool:
    candidate_values = [candidate.value for candidate in candidates]
    if isinstance(value, list):
        allowed_items = {
            item
            for candidate_value in candidate_values
            for item in (candidate_value if isinstance(candidate_value, list) else [])
        }
        return all(item in allowed_items for item in value)
    return value in candidate_values


def _value_contributors(
    value: object, candidates: Sequence[ProfileUpdate]
) -> list[ProfileUpdate]:
    if not isinstance(value, list):
        return [candidate for candidate in candidates if candidate.value == value]
    requested = set(value)
    return [
        candidate
        for candidate in candidates
        if isinstance(candidate.value, list) and requested.intersection(candidate.value)
    ]


def _environment_boolean(name: str, *, default: bool) -> bool:
    value = os.environ.get(name)
    if value is None:
        return default
    normalized = value.strip().lower()
    if normalized in {"1", "true", "yes", "on"}:
        return True
    if normalized in {"0", "false", "no", "off"}:
        return False
    raise RuntimeError(f"{name} must be true or false")


def _environment_integer(name: str, *, default: int) -> int:
    value = os.environ.get(name)
    if value is None:
        return default
    try:
        parsed = int(value)
    except ValueError as exc:
        raise RuntimeError(f"{name} must be an integer") from exc
    if parsed < 1:
        raise RuntimeError(f"{name} must be positive")
    return parsed


def _environment_value(primary: str, legacy: str) -> str | None:
    return os.environ.get(primary) or os.environ.get(legacy)


def _environment_boolean_alias(primary: str, legacy: str, *, default: bool) -> bool:
    if primary in os.environ:
        return _environment_boolean(primary, default=default)
    return _environment_boolean(legacy, default=default)


def _configured_proxy() -> str | None:
    for name in (
        "KYLIN_PROFILE_LLM_PROXY_URL",
        "OPENAI_PROXY_URL",
        "HTTPS_PROXY",
        "https_proxy",
        "HTTP_PROXY",
        "http_proxy",
        "ALL_PROXY",
        "all_proxy",
    ):
        value = os.environ.get(name)
        if not value:
            continue
        # HTTPX recognizes socks5://, while many desktop proxy tools export socks://.
        if value.lower().startswith("socks://"):
            value = f"socks5://{value[len('socks://') :]}"
        return value
    return None


def _is_unsupported_endpoint(exc: Exception) -> bool:
    return getattr(exc, "status_code", None) in {404, 405}


# Backward-compatible public name used by the first prototype revision.
OpenAIProfileExtractor = OpenAICompatibleProfileExtractor
