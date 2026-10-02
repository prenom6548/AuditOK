#!/usr/bin/env python3
"""Interactively verify KylinAgent desktop notification behavior."""

import argparse
import json
import queue
import shutil
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from gateway.platforms.api_server import _notify_desktop_async  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Send a persistent cron-style notification and report whether it "
            "was activated or explicitly closed, then verify KylinAgent launch."
        )
    )
    parser.add_argument(
        "--expect",
        choices=("action", "closed"),
        help="Fail unless this terminal event is observed.",
    )
    parser.add_argument(
        "--simulate",
        choices=("action", "closed"),
        help="Trigger the selected UKUI notification event automatically.",
    )
    parser.add_argument("--timeout", type=float, default=60.0)
    args = parser.parse_args()

    events: "queue.Queue[tuple[str, dict]]" = queue.Queue()

    def on_event(event: str, details: dict) -> None:
        events.put((event, details))

    started = _notify_desktop_async(
        "手工测试：点击通知正文或关闭按钮都应打开 KylinAgent。",
        event_callback=on_event,
    )
    if not started:
        print("Desktop notification could not be started.", file=sys.stderr)
        return 2

    if args.simulate:
        print(f"Notification sent. Simulating {args.simulate} event.", flush=True)
    else:
        print("Notification sent. Click its body or close button now.", flush=True)
    launched_seen = False
    while True:
        try:
            event, details = events.get(timeout=args.timeout)
        except queue.Empty:
            print("Timed out waiting for a notification event.", file=sys.stderr)
            return 3
        print(json.dumps({"event": event, **details}, ensure_ascii=False), flush=True)
        if event == "launched":
            launched_seen = True
            continue
        if event == "shown" and args.simulate:
            notification_id = details.get("notification_id")
            gdbus = shutil.which("gdbus")
            if not gdbus or notification_id is None:
                print("Cannot simulate without gdbus and a notification ID.", file=sys.stderr)
                return 5
            if args.simulate == "closed":
                method = "org.ukui.NotificationServer.CloseNotification"
                method_args = [str(notification_id), "2"]
            else:
                method = "org.ukui.NotificationServer.InvokeAction"
                method_args = [str(notification_id), "default"]
            simulated = subprocess.run(
                [
                    gdbus,
                    "call",
                    "--session",
                    "--dest", "org.freedesktop.Notifications",
                    "--object-path", "/org/freedesktop/Notifications",
                    "--method", method,
                    *method_args,
                ],
                capture_output=True,
                text=True,
                timeout=5,
            )
            if simulated.returncode != 0:
                print(simulated.stderr.strip(), file=sys.stderr)
                return 5
        if event not in {"action", "closed"}:
            continue
        expected = args.expect or args.simulate
        if expected and event != expected:
            print(f"Expected {expected!r}, received {event!r}.", file=sys.stderr)
            return 1
        if launched_seen:
            return 0
        try:
            launched_event, launched_details = events.get(timeout=5)
        except queue.Empty:
            print(f"{event.title()} arrived but no launch event followed.", file=sys.stderr)
            return 4
        print(
            json.dumps(
                {"event": launched_event, **launched_details}, ensure_ascii=False
            ),
            flush=True,
        )
        return 0 if launched_event == "launched" else 4


if __name__ == "__main__":
    raise SystemExit(main())
