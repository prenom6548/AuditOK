#!/usr/bin/env python3
"""Send a raw OpenAI-compatible chat request with top-level priority.

This bypasses the OpenAI SDK entirely, so it is useful for checking whether a
local model service can read ``body["priority"]`` from the raw HTTP JSON body.
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.error
import urllib.request


DEFAULT_MESSAGE = "详细介绍北京的各个方面，包括历史、文化、旅游景点、美食等，不少于3000字。"


def build_body(args: argparse.Namespace) -> dict:
    return {
        "messages": [
            {
                "content": args.message,
                "role": "user",
            }
        ],
        "model": args.model,
        "priority": args.priority,
    }


def send_request(args: argparse.Namespace) -> int:
    url = args.base_url.rstrip("/") + "/chat/completions"
    body = build_body(args)
    payload = json.dumps(body, ensure_ascii=False).encode("utf-8")

    headers = {
        "Content-Type": "application/json",
    }
    if args.api_key:
        headers["Authorization"] = f"Bearer {args.api_key}"

    print("== request url ==")
    print(url)
    print("\n== request body ==")
    print(json.dumps(body, ensure_ascii=False, indent=2, sort_keys=True))
    print(f"\nbody.priority: {body.get('priority')!r}")

    if args.dry_run:
        print("\nDry-run only. Remove --dry-run to send.")
        return 0

    request = urllib.request.Request(
        url,
        data=payload,
        headers=headers,
        method="POST",
    )

    try:
        with urllib.request.urlopen(request, timeout=args.timeout) as response:
            response_text = response.read().decode("utf-8", errors="replace")
            print("\n== response status ==")
            print(response.status)
            print("\n== response body ==")
            print(response_text)
            return 0
    except urllib.error.HTTPError as exc:
        print("\n== response status ==")
        print(exc.code)
        print("\n== error body ==")
        print(exc.read().decode("utf-8", errors="replace"))
        return 1
    except urllib.error.URLError as exc:
        print("\n== request failed ==")
        print(exc)
        return 1


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="POST a raw chat/completions request with top-level priority.",
    )
    parser.add_argument("--base-url", default="http://localhost:8000/v1")
    parser.add_argument("--model", default="qwen3.6-27b")
    parser.add_argument("--priority", type=int, default=2)
    parser.add_argument("--message", default=DEFAULT_MESSAGE)
    parser.add_argument("--timeout", type=float, default=300.0)
    parser.add_argument("--api-key", default="")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    return send_request(parse_args(argv or sys.argv[1:]))


if __name__ == "__main__":
    raise SystemExit(main())
