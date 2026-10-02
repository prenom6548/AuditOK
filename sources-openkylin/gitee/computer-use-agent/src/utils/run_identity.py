from __future__ import annotations

import re
import uuid
from datetime import datetime
from typing import Optional


RUN_ID_SLUG_MAX_LEN = 60


def _to_slug(value: str, max_len: int = RUN_ID_SLUG_MAX_LEN) -> str:
    """Convert a task or configured ID into a path-safe, readable slug."""
    slug = re.sub(r"[^a-z0-9]+", "-", str(value or "").strip().lower())
    slug = slug.strip("-")
    return (slug or "task")[:max_len]


def create_run_id(
    task: str,
    *,
    now: Optional[datetime] = None,
    base_id: Optional[str] = None,
    nonce: Optional[str] = None,
) -> str:
    """Return a unique, path-safe ID for one non-resumed execution.

    Microseconds make IDs easy to correlate with logs, while a UUID prevents
    collisions between concurrent launches and between launches whose clocks
    have the same value. ``nonce`` is injectable only to make tests deterministic.
    """
    started_at = now or datetime.now()
    timestamp = started_at.strftime("%Y-%m-%d_%H%M%S_%f")
    slug = _to_slug(base_id if base_id else task)
    unique_part = _to_slug(nonce, max_len=32) if nonce else uuid.uuid4().hex
    return f"{timestamp}_{slug}_{unique_part}"
