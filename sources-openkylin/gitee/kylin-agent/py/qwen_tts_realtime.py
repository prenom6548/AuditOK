#!/usr/bin/env python3
"""Qwen-TTS-Realtime WebSocket 语音合成验证脚本。

默认使用阿里云百炼 qwen3-tts-flash-realtime，将文本合成为 24kHz/16bit/mono PCM，
并额外写出 WAV 文件方便直接播放。
"""

from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import threading
import time
import uuid
import wave
from pathlib import Path
from typing import Any

import websocket


DEFAULT_MODEL = "qwen3-tts-flash-realtime"
DEFAULT_URL = "wss://dashscope.aliyuncs.com/api-ws/v1/realtime"
DEFAULT_TEXT = "你好，我是 KylinAgent。语音播报功能正在测试中。"
DEFAULT_VOICE = "Cherry"
DEFAULT_SAMPLE_RATE = 24_000


class QwenTTSRealtimeClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.done = threading.Event()
        self.session_updated = threading.Event()
        self.failed = False
        self.first_audio_at: float | None = None
        self.started_at = 0.0
        self.audio_bytes = bytearray()
        self.ws = websocket.WebSocketApp(
            self._url(),
            header=self._headers(),
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _url(self) -> str:
        separator = "&" if "?" in self.args.url else "?"
        return f"{self.args.url}{separator}model={self.args.model}"

    def _headers(self) -> list[str]:
        headers = [
            f"Authorization: Bearer {self.args.api_key}",
            "user-agent: kylin-agent-tts-probe",
        ]
        if self.args.workspace_id:
            headers.append(f"X-DashScope-WorkSpace: {self.args.workspace_id}")
        return headers

    def run(self) -> int:
        print(f"连接地址: {self._url()}")
        print(f"模型: {self.args.model}")
        print(f"音色: {self.args.voice}")
        self.started_at = time.monotonic()
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        if self.audio_bytes:
            self._write_outputs()
        return 1 if self.failed or not self.done.is_set() else 0

    def _send_event(self, event: dict[str, Any]) -> None:
        event.setdefault("event_id", f"event_{uuid.uuid4().hex}")
        self.ws.send(json.dumps(event, ensure_ascii=False))

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接，等待 session.created。")

    def _on_message(self, ws: websocket.WebSocketApp, raw: str) -> None:
        try:
            message = json.loads(raw)
        except json.JSONDecodeError:
            print(f"收到无法解析的消息: {raw!r}", file=sys.stderr)
            return

        event_type = message.get("type", "")
        if event_type == "error":
            self.failed = True
            error = message.get("error", {})
            print(f"错误: {error.get('code', '')} {error.get('message', '')}", file=sys.stderr)
            ws.close()
            return

        if event_type == "session.created":
            session = message.get("session", {})
            print(f"会话已创建: {session.get('id', '-')}")
            self._update_session()
            return

        if event_type == "session.updated":
            print("会话配置已更新，发送文本。")
            self.session_updated.set()
            threading.Thread(target=self._send_text_and_finish, daemon=True).start()
            return

        if event_type == "response.audio.delta":
            delta = message.get("delta", "")
            if delta:
                if self.first_audio_at is None:
                    self.first_audio_at = time.monotonic()
                    print(f"首包音频延迟: {(self.first_audio_at - self.started_at) * 1000:.0f} ms")
                chunk = base64.b64decode(delta)
                self.audio_bytes.extend(chunk)
                print(f"收到音频分片: {len(chunk)} bytes，总计 {len(self.audio_bytes)} bytes")
            return

        if event_type in {"response.audio.done", "response.done"}:
            print(f"收到事件: {event_type}")
            return

        if event_type == "session.finished":
            print("会话完成。")
            self.done.set()
            ws.close()
            return

        print(f"收到事件: {message}")

    def _update_session(self) -> None:
        session: dict[str, Any] = {
            "voice": self.args.voice,
            "response_format": self.args.format,
            "sample_rate": self.args.sample_rate,
            "mode": self.args.mode,
        }
        if self.args.language_type:
            session["language_type"] = self.args.language_type
        if self.args.instructions:
            session["instructions"] = self.args.instructions
            session["optimize_instructions"] = self.args.optimize_instructions

        self._send_event({
            "type": "session.update",
            "session": session,
        })

    def _send_text_and_finish(self) -> None:
        chunks = self.args.text
        for chunk in chunks:
            print(f"发送文本: {chunk}")
            self._send_event({
                "type": "input_text_buffer.append",
                "text": chunk,
            })
            time.sleep(self.args.chunk_delay_ms / 1000)

        if self.args.mode == "commit":
            self._send_event({"type": "input_text_buffer.commit"})
            print("已发送 input_text_buffer.commit。")

        self._send_event({"type": "session.finish"})
        print("已发送 session.finish，等待音频和结束事件。")

    def _on_error(self, ws: websocket.WebSocketApp, error: Any) -> None:
        self.failed = True
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        if not self.done.is_set():
            print(f"连接已关闭: code={status_code}, reason={reason or '-'}")

    def _write_outputs(self) -> None:
        pcm_path = self.args.output
        pcm_path.write_bytes(self.audio_bytes)
        print(f"已写出 PCM: {pcm_path} ({len(self.audio_bytes)} bytes)")

        if self.args.format == "pcm" and self.args.wav_output:
            with wave.open(str(self.args.wav_output), "wb") as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(self.args.sample_rate)
                wav.writeframes(self.audio_bytes)
            print(f"已写出 WAV: {self.args.wav_output}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Qwen TTS Realtime WebSocket 测试")
    parser.add_argument("--api-key", default=os.getenv("DASHSCOPE_API_KEY"))
    parser.add_argument("--workspace-id", default=os.getenv("DASHSCOPE_WORKSPACE_ID"))
    parser.add_argument("--url", default=os.getenv("DASHSCOPE_TTS_URL", DEFAULT_URL))
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--voice", default=DEFAULT_VOICE)
    parser.add_argument("--text", nargs="+", default=[DEFAULT_TEXT])
    parser.add_argument("--mode", choices=["server_commit", "commit"], default="commit")
    parser.add_argument("--format", choices=["pcm", "wav", "mp3", "opus"], default="pcm")
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--language-type", default="Chinese")
    parser.add_argument("--instructions")
    parser.add_argument("--optimize-instructions", action="store_true")
    parser.add_argument("--chunk-delay-ms", type=int, default=100)
    parser.add_argument("--output", type=Path, default=Path("py/qwen_tts_result_24k.pcm"))
    parser.add_argument("--wav-output", type=Path, default=Path("py/qwen_tts_result_24k.wav"))
    args = parser.parse_args()

    if not args.api_key:
        parser.error("请设置 DASHSCOPE_API_KEY 或传入 --api-key")
    if args.sample_rate <= 0:
        parser.error("--sample-rate 必须大于 0")
    if args.chunk_delay_ms < 0:
        parser.error("--chunk-delay-ms 不能小于 0")
    if args.format != "pcm" and args.wav_output:
        print("提示：非 PCM 格式不会额外写 WAV 封装文件。", file=sys.stderr)
    return args


def main() -> int:
    return QwenTTSRealtimeClient(parse_args()).run()


if __name__ == "__main__":
    raise SystemExit(main())
