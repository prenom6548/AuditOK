#!/usr/bin/env python3
"""Control a running KylinAgent. Standard library only; also importable from Python."""

import argparse
import json
import os
from pathlib import Path
import socket
import sys
import time
import uuid


class ControlError(RuntimeError):
    def __init__(self, code, message):
        self.code = code
        super().__init__(message)


class KylinAgentClient:
    def __init__(self, socket_path=None, timeout=30.0):
        self.socket_path = str(socket_path or os.environ.get("KYLIN_AGENT_CONTROL_SOCKET")
                               or Path.home() / ".cache/kylin-agent-control.socket")
        self.timeout = timeout

    def request(self, command, **params):
        request_id = uuid.uuid4().hex
        payload = (json.dumps(dict(params, command=command, id=request_id),
                              ensure_ascii=False) + "\n").encode("utf-8")
        if len(payload) > 1024 * 1024:
            raise ControlError("request_too_large", "Request exceeds 1 MiB")
        deadline = time.monotonic() + self.timeout
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as connection:
            connection.settimeout(self.timeout)
            try:
                connection.connect(self.socket_path)
            except FileNotFoundError as exc:
                raise ControlError(
                    "control_unavailable",
                    "未找到 KylinAgent 控制 Socket：" + self.socket_path
                    + "。请先编译包含外部接口的新版程序，完全退出旧版后启动新版，"
                    "等待主窗口加载完成再重试。仅修改源码或启动客户端不会更新正在运行的程序。"
                    "若使用 --socket 或 KYLIN_AGENT_CONTROL_SOCKET，请检查路径。"
                ) from exc
            except ConnectionRefusedError as exc:
                raise ControlError(
                    "control_unavailable",
                    "KylinAgent 控制 Socket 无进程监听：" + self.socket_path
                    + "。请完全退出旧版后启动包含外部接口的新版程序。"
                ) from exc
            connection.sendall(payload)
            response = bytearray()
            while b"\n" not in response:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError("Timed out waiting for KylinAgent")
                connection.settimeout(remaining)
                part = connection.recv(65536)
                if not part:
                    raise ControlError("incomplete_response", "Server closed before a complete response")
                response.extend(part)
                if len(response) > 4 * 1024 * 1024:
                    raise ControlError("response_too_large", "Response exceeds 4 MiB")
        try:
            data = json.loads(response.split(b"\n", 1)[0])
        except (ValueError, UnicodeError) as exc:
            raise ControlError("invalid_response", "Server returned invalid JSON") from exc
        if not isinstance(data, dict) or type(data.get("ok")) is not bool:
            raise ControlError("invalid_response", "Response must contain a boolean ok field")
        # Transport-level errors can occur before a request ID is parsed.
        if data.get("id") != request_id and (data["ok"] or "id" in data):
            raise ControlError("invalid_response", "Response ID does not match request")
        if not data["ok"]:
            error = data.get("error")
            if not isinstance(error, dict):
                raise ControlError("invalid_response", "Missing error object")
            if command == "running_count" and error.get("code") == "unknown_command":
                # Older desktop builds already expose the running session IDs
                # through status, even though they lack this dedicated command.
                status = self.status()
                session_ids = status.get("running_session_ids")
                if (not isinstance(session_ids, list)
                        or any(not isinstance(value, str) or not value for value in session_ids)):
                    raise ControlError("invalid_response", "status 缺少有效的 running_session_ids，无法获取执行会话个数")
                session_ids = list(dict.fromkeys(session_ids))
                if status.get("running") and not session_ids:
                    raise ControlError("invalid_response", "status 显示任务执行中，但执行会话列表为空，请稍后重试")
                return {"running_count": len(session_ids), "running_session_ids": session_ids}
            raise ControlError(error.get("code", "unknown"), error.get("message", "Request failed"))
        if not isinstance(data.get("result"), dict):
            raise ControlError("invalid_response", "Missing result object")
        return data["result"]

    def status(self):
        return self.request("status")

    def running_count(self):
        """Number of desktop sessions executing a task, including background sessions."""
        return self.request("running_count")["running_count"]

    def new_session(self):
        """Open an untitled draft. The first send returns its persistent session ID."""
        return self.request("new_session")

    def set_input(self, text, session_id=None):
        return self.request("set_input", text=text, **self._guard(session_id))

    def send(self, text=None, session_id=None):
        params = self._guard(session_id)
        if text is not None:
            params["text"] = text
        return self.request("send", **params)

    def stop(self, session_id=None):
        return self.request("stop", **self._guard(session_id))

    @staticmethod
    def _guard(session_id):
        return {} if session_id is None else {"session_id": session_id}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--socket", help="Unix socket path")
    parser.add_argument("--timeout", type=float, default=30.0)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("status", help="Read input, current session and global desktop task status")
    commands.add_parser("running-count", help="Count desktop sessions with running tasks, including background sessions")
    commands.add_parser("new-session", help="Open an untitled draft; create the session on first send")
    for name in ("set-input", "send", "stop"):
        command = commands.add_parser(name)
        command.add_argument("--session-id", help="Fail if the UI has switched to another session")
        if name != "stop":
            text = command.add_mutually_exclusive_group(required=name == "set-input")
            text.add_argument("--text")
            text.add_argument("--stdin", action="store_true", help="Read UTF-8 text from stdin")
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    client = KylinAgentClient(args.socket, args.timeout)
    params = {}
    if getattr(args, "session_id", None) is not None:
        params["session_id"] = args.session_id
    if getattr(args, "stdin", False):
        params["text"] = sys.stdin.buffer.read().decode("utf-8")
    elif getattr(args, "text", None) is not None:
        params["text"] = args.text
    try:
        result = client.request(args.command.replace("-", "_"), **params)
        print(json.dumps({"ok": True, "result": result}, ensure_ascii=False))
        return 0
    except (ControlError, OSError) as exc:
        print(json.dumps({"ok": False, "error": {
            "code": getattr(exc, "code", "connection_error"), "message": str(exc)
        }}, ensure_ascii=False), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
