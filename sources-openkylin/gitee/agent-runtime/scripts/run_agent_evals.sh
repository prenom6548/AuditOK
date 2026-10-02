#!/usr/bin/env bash
# Run deterministic agent behavior evals.
#
# This runner is intentionally model-free: it validates recorded/synthetic
# traces for agent behavior contracts such as TODO completion, pip index usage,
# delegation ordering, gateway delivery, and command safety.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PYTHON=""
for candidate in "$REPO_ROOT/.venv/bin/python" "$REPO_ROOT/venv/bin/python"; do
  if [ -x "$candidate" ]; then
    PYTHON="$candidate"
    break
  fi
done

if [ -z "$PYTHON" ]; then
  PYTHON="$(command -v python3 || command -v python)"
fi

cd "$REPO_ROOT"
exec "$PYTHON" agent_eval_runner.py "$@"
