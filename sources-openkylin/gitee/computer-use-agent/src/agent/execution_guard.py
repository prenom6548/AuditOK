from __future__ import annotations

import json
import os
import tempfile
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any, Literal, Optional


CompletionDecision = Literal["continue", "verify", "confirmed"]


def _is_success(value: Any) -> bool:
    text = str(value or "").strip().lower()
    return text.startswith("success") or "成功" in text


@dataclass
class CompletionGuard:
    """Two-phase task completion gate.

    The first completion claim starts a verification round.  A later Brain
    observation must independently confirm the result before the run may end.
    Normal, non-completion steps do not pay any additional model-call cost.
    """

    enabled: bool = True
    required_confirmations: int = 2
    pending: bool = False
    confirmations: int = 0
    requested_step: Optional[int] = None
    verification_rounds: int = 0

    def evaluate(
        self,
        *,
        completion_requested: bool,
        step_evaluate: Any,
        step: int,
    ) -> CompletionDecision:
        if not self.enabled:
            return "confirmed" if completion_requested else "continue"

        if not completion_requested:
            # During a verification round, choosing another action means the
            # visual reviewer found unfinished work.  Clear the stale claim.
            if self.pending:
                self.reset()
            return "continue"

        if not self.pending:
            self.pending = True
            self.confirmations = 1
            self.requested_step = step
            self.verification_rounds += 1
            return "verify"

        if _is_success(step_evaluate):
            self.confirmations += 1
        else:
            self.reset()
            return "continue"

        if self.confirmations >= max(2, int(self.required_confirmations)):
            self.pending = False
            return "confirmed"
        return "verify"

    def reset(self) -> None:
        self.pending = False
        self.confirmations = 0
        self.requested_step = None

    def review_instruction(self) -> str:
        return (
            "COMPLETION VERIFICATION GATE IS ACTIVE. Independently inspect the newest "
            "screenshot and the recorded action results. Verify the user's full task outcome, "
            "not merely that an action was dispatched. Check for unfinished required actions, "
            "unsaved/unsubmitted state, error indicators, wrong target, and missing final-result "
            "evidence. For tasks that send, share, post, email, save, upload, download, create, "
            "or otherwise deliver something, require fresh evidence from this run; do not count "
            "pre-existing chat history, old records, old files, or stale UI state as completion "
            "unless the recorded actions from this run show the required content was inserted "
            "and dispatched. If the outcome is visibly confirmed, return the exact task-completion "
            "signal again with step_evaluate=Success. Otherwise provide the next corrective goal. "
            "Do not repeat a non-idempotent action until the screen has been checked for evidence "
            "that it may already have succeeded."
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    def restore(self, data: Optional[dict[str, Any]]) -> None:
        if not isinstance(data, dict):
            return
        self.pending = bool(data.get("pending", False))
        self.confirmations = int(data.get("confirmations", 0))
        self.requested_step = data.get("requested_step")
        self.verification_rounds = int(data.get("verification_rounds", 0))


@dataclass
class ExecutionMetrics:
    completion_requests: int = 0
    completion_claims_blocked: int = 0
    verified_completions: int = 0
    verification_rejections: int = 0
    visual_verification_rounds: int = 0
    checkpoints_written: int = 0
    recoveries: int = 0
    actions_dispatched: int = 0
    action_errors: int = 0
    text_focus_visual_checks: int = 0
    text_inputs_dispatched: int = 0
    text_input_failures: int = 0
    text_result_visual_checks: int = 0
    started_at: float = field(default_factory=time.time)


class ExecutionJournal:
    """Crash-tolerant checkpoint and append-only event journal."""

    def __init__(self, directory: str | Path, enabled: bool = True) -> None:
        self.enabled = enabled
        self.directory = Path(directory)
        self.checkpoint_path = self.directory / "execution_checkpoint.json"
        self.events_path = self.directory / "execution_events.jsonl"
        self.report_path = self.directory / "execution_report.json"
        self.metrics = ExecutionMetrics()

    def event(self, event_type: str, **payload: Any) -> None:
        if not self.enabled:
            return
        self.directory.mkdir(parents=True, exist_ok=True)
        record = {"time": time.time(), "type": event_type, **payload}
        with self.events_path.open("a", encoding="utf-8") as stream:
            stream.write(json.dumps(record, ensure_ascii=False, default=str) + "\n")

    def checkpoint(self, state: dict[str, Any]) -> None:
        if not self.enabled:
            return
        self.metrics.checkpoints_written += 1
        payload = {
            "updated_at": time.time(),
            "state": state,
            "metrics": asdict(self.metrics),
        }
        self._atomic_json_write(self.checkpoint_path, payload)

    def load(self) -> Optional[dict[str, Any]]:
        if not self.enabled or not self.checkpoint_path.exists():
            return None
        try:
            payload = json.loads(self.checkpoint_path.read_text(encoding="utf-8"))
            saved_metrics = payload.get("metrics", {})
            for name in asdict(self.metrics):
                if name in saved_metrics:
                    setattr(self.metrics, name, saved_metrics[name])
            self.metrics.recoveries += 1
            self.event("recovery_loaded", step=payload.get("state", {}).get("step"))
            return payload.get("state")
        except (OSError, ValueError, TypeError):
            return None

    def write_report(
        self,
        *,
        status: str,
        step: int,
        final_evidence: Optional[str] = None,
    ) -> None:
        if not self.enabled:
            return
        payload = {
            "status": status,
            "step": step,
            "duration_seconds": round(max(0.0, time.time() - self.metrics.started_at), 3),
            "metrics": asdict(self.metrics),
            "final_evidence": final_evidence,
        }
        self._atomic_json_write(self.report_path, payload)

    @staticmethod
    def _atomic_json_write(path: Path, payload: dict[str, Any]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        fd, tmp_name = tempfile.mkstemp(dir=path.parent, suffix=".tmp")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as stream:
                json.dump(payload, stream, ensure_ascii=False, indent=2, default=str)
                stream.flush()
                os.fsync(stream.fileno())
            os.replace(tmp_name, path)
        except Exception:
            try:
                os.unlink(tmp_name)
            except OSError:
                pass
            raise
