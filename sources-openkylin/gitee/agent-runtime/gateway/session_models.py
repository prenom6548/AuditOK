"""Session-scoped model identity and model changes at execution boundaries."""
from __future__ import annotations

import copy


def selection_provider(value):
    # kylin-local-selection-provider-v1: accept legacy built-in local aliases.
    provider = str(value or "custom")
    return "custom" if provider.startswith("kylin-local-") else provider


def public_selection(entry: dict) -> dict:
    return {"model": str(entry.get("model") or ""),
            "provider": str(entry.get("provider") or "custom"),
            "base_url": str(entry.get("baseUrl") or entry.get("base_url") or "").rstrip("/"),
            "name": str(entry.get("name") or "")}


def find_model_entry(entries: list[dict], selection: dict) -> dict:
    """Only resolve configured endpoints; duplicate model IDs need a qualifier."""
    if not isinstance(selection, dict) or not isinstance(selection.get("model"), str):
        raise ValueError("model_selection must contain a model")
    matches = []
    for entry in entries:
        public = public_selection(entry)
        if selection["model"] not in {public["model"], str(entry.get("id") or "")}:
            continue
        if selection.get("provider") and selection_provider(selection["provider"]) != selection_provider(public["provider"]):
            continue
        if selection.get("base_url") and str(selection["base_url"]).rstrip("/") != public["base_url"]:
            continue
        matches.append(entry)
    if not matches:
        raise ValueError("Selected model is no longer configured")
    identities = {(public_selection(e)["provider"], public_selection(e)["base_url"]) for e in matches}
    if len(identities) > 1:
        raise ValueError("Model ID is ambiguous; specify its provider and base_url")
    return matches[0]


def resolve_entry_runtime(entry: dict) -> dict:
    from kylin_agent_runtime_cli.runtime_provider import resolve_runtime_provider
    from kylin_agent_runtime_cli.auth import AuthError, resolve_provider
    identity = public_selection(entry)
    requested = identity["provider"]
    key = entry.get("apiKey") or entry.get("api_key") or None
    try:
        resolve_provider(requested, explicit_base_url=identity["base_url"] or None, explicit_api_key=key)
    except AuthError:
        # Desktop model rows may use a display/provider alias absent from the
        # built-in registry. The endpoint was matched against configured rows,
        # so use its own OpenAI-compatible runtime, never the global default.
        if identity["base_url"] and key:
            requested = "custom"
        else:
            raise
    runtime = resolve_runtime_provider(
        requested=requested,
        explicit_base_url=identity["base_url"] or None,
        explicit_api_key=key,
        target_model=identity["model"],
    )
    return {key: runtime.get(key) for key in
            ("provider", "api_key", "base_url", "api_mode", "credential_pool", "command", "args")}


def switch_at_boundary(agent, runtime: dict, model: str) -> None:
    """Prepare on a shallow runtime copy, then commit without touching task state.

    Requests/tools have already finished. A failed client construction must not
    leave half of the old provider paired with half of the new provider.
    """
    from agent.agent_runtime_helpers import switch_model
    pending = copy.copy(agent)
    pending._transport_cache = {}
    pending.context_compressor = copy.copy(getattr(agent, "context_compressor", None))
    pending._credential_pool = runtime.get("credential_pool")
    # Do not let switch_model's legacy empty-value fallback reuse another
    # provider's key or endpoint.
    pending.api_key = runtime.get("api_key") or ""
    pending.base_url = runtime.get("base_url") or ""
    switch_model(pending, model, runtime["provider"], pending.api_key,
                 pending.base_url, runtime.get("api_mode") or "")
    if agent._interrupt_requested:
        return
    # Only fields changed by the runtime swap are committed. In particular,
    # concurrent cancellation, budgets, tools, TODOs and callbacks stay intact.
    for key, value in pending.__dict__.items():
        if key in {"_cached_system_prompt", "_interrupt_requested", "_interrupt_message"}:
            continue
        if value is not agent.__dict__.get(key):
            agent.__dict__[key] = value


class SessionModelController:
    def __init__(self, db, session_id, entries, initial_selection):
        self.db, self.session_id, self.entries = db, session_id, entries
        self.revision = (initial_selection or {}).get("revision", 0)

    def __call__(self, agent):
        if agent._interrupt_requested:
            return
        selection = self.db.get_session_model_selection(self.session_id)
        if not selection or selection.get("revision", 0) == self.revision:
            return
        from agent.session_detail_logger import write_session_detail
        self.revision = selection["revision"]
        try:
            entry = find_model_entry(self.entries(), selection)
            runtime = resolve_entry_runtime(entry)
            switch_at_boundary(agent, runtime, selection["model"])
            if agent._interrupt_requested:
                return
            write_session_detail(self.session_id, "model.switch.applied", {
                **selection, "api_mode": agent.api_mode,
            })
            agent._emit_status("已切换当前会话模型：" + selection["model"])
        except Exception as exc:
            # Keep the previous runtime; don't turn a failed switch into an
            # automatic task restart or reveal credential-bearing exceptions.
            write_session_detail(self.session_id, "model.switch.failed", {
                **selection, "error_type": type(exc).__name__,
                "reason": "模型切换失败，本轮继续使用原模型；请检查所选模型配置。",
            })
            agent._emit_status("模型切换失败，本轮继续使用原模型。")
