#!/usr/bin/env python3
"""Quick check for Hermes extra_body.priority propagation.

Default mode does not call the model. It builds the same OpenAI-compatible
kwargs Hermes uses and prints the final extra_body.

For a real local-model request:

    python quick-test/check_model_priority.py --live \
      --base-url http://localhost:8000/v1 \
      --model your-local-model \
      --priority 2
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))


def _jsonable(value: Any) -> Any:
    try:
        json.dumps(value)
        return value
    except TypeError:
        return repr(value)


def _print_request(label: str, kwargs: dict[str, Any]) -> None:
    extra_body = kwargs.get("extra_body") or {}
    print(f"\n== {label} ==")
    print(f"keys: {sorted(kwargs.keys())}")
    print(f"extra_body.priority: {extra_body.get('priority')!r}")
    print("extra_body:")
    print(json.dumps(_jsonable(extra_body), ensure_ascii=False, indent=2, sort_keys=True))


def _wire_body_from_kwargs(kwargs: dict[str, Any]) -> dict[str, Any]:
    """Approximate the JSON body the OpenAI SDK sends on the wire.

    ``extra_body`` is an SDK-only kwarg. Its fields are merged into the
    top-level HTTP JSON body, while ``timeout`` stays client-side.
    """
    body = {
        key: value
        for key, value in kwargs.items()
        if key not in {"extra_body", "timeout"}
    }
    extra_body = kwargs.get("extra_body")
    if isinstance(extra_body, dict):
        body.update(extra_body)
    return body


def _print_wire_body(kwargs: dict[str, Any]) -> dict[str, Any]:
    body = _wire_body_from_kwargs(kwargs)
    print("\n== HTTP JSON body your model service should receive ==")
    print(json.dumps(_jsonable(body), ensure_ascii=False, indent=2, sort_keys=True))
    print(f"\nbody.priority: {body.get('priority')!r}")
    return body


def build_hermes_kwargs(args: argparse.Namespace) -> dict[str, Any]:
    import agent.transports.chat_completions  # noqa: F401
    from agent.request_priority import (
        reset_current_request_priority,
        set_current_request_priority_from_overrides,
    )
    from agent.transports import get_transport

    request_overrides = {"extra_body": {"priority": args.priority}}
    token = set_current_request_priority_from_overrides(request_overrides)
    try:
        transport = get_transport("chat_completions")
        return transport.build_kwargs(
            model=args.model,
            messages=[{"role": "user", "content": args.message}],
            tools=[],
            base_url=args.base_url,
            timeout=args.timeout,
            request_overrides=request_overrides,
            provider_name="custom",
            is_custom_provider=True,
        )
    finally:
        reset_current_request_priority(token)


def dry_run(args: argparse.Namespace) -> int:
    kwargs = build_hermes_kwargs(args)
    _print_request("Hermes-built chat.completions kwargs", kwargs)
    _print_wire_body(kwargs)
    print("\nDry-run only. Add --live to send this to your local model service.")
    return 0


def raw_live_run(args: argparse.Namespace) -> int:
    kwargs = build_hermes_kwargs(args)
    _print_request("Hermes-built chat.completions kwargs", kwargs)
    body = _print_wire_body(kwargs)

    url = args.base_url.rstrip("/") + "/chat/completions"
    headers = {"Content-Type": "application/json"}
    if args.api_key:
        headers["Authorization"] = f"Bearer {args.api_key}"
    payload = json.dumps(body, ensure_ascii=False).encode("utf-8")
    request = urllib.request.Request(url, data=payload, headers=headers, method="POST")

    print(f"\nSending raw HTTP request to {url} ...")
    try:
        with urllib.request.urlopen(request, timeout=args.timeout) as response:
            response_body = response.read().decode("utf-8", errors="replace")
            print(f"\n== raw response status ==\n{response.status}")
            print("\n== raw response body ==")
            print(response_body)
    except urllib.error.HTTPError as exc:
        print(f"\n== raw response status ==\n{exc.code}")
        print("\n== raw error body ==")
        print(exc.read().decode("utf-8", errors="replace"))
        return 1
    return 0


def live_run(args: argparse.Namespace) -> int:
    from openai import OpenAI

    kwargs = build_hermes_kwargs(args)
    _print_request("Hermes-built chat.completions kwargs", kwargs)
    _print_wire_body(kwargs)

    client = OpenAI(
        base_url=args.base_url,
        api_key=args.api_key or "local-no-api-key-required",
        timeout=args.timeout,
    )

    print("\nSending live request...")
    response = client.chat.completions.create(**kwargs)
    message = response.choices[0].message
    print("\n== response ==")
    print(getattr(message, "content", "") or "")
    return 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check whether Hermes sends extra_body.priority.",
    )
    parser.add_argument("--base-url", default="http://localhost:8000/v1")
    parser.add_argument("--model", default="local-model")
    parser.add_argument("--priority", type=int, default=2)
    parser.add_argument("--message", default="详细介绍北京的各个方面，包括历史、文化、旅游景点、美食等，不少于3000字。")
    parser.add_argument("--timeout", type=float, default=60.0)
    parser.add_argument("--api-key", default="")
    parser.add_argument("--live", action="store_true")
    parser.add_argument("--raw-live", action="store_true", help="POST the printed JSON body directly, without OpenAI SDK.")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    if args.raw_live:
        return raw_live_run(args)
    if args.live:
        return live_run(args)
    return dry_run(args)


if __name__ == "__main__":
    raise SystemExit(main())
