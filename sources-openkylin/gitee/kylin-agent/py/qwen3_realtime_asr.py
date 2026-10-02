#!/usr/bin/env python3
"""Qwen3-ASR-Flash-Realtime WebSocket 验证脚本。

Qwen3-ASR-Realtime 使用 /api-ws/v1/realtime?model=... 和 Realtime 事件协议，
不同于 qwen-audio-3.0-asr-flash-streaming / fun-asr-realtime 的 run-task 协议。
"""

from __future__ import annotations

import argparse
import base64
import json
import os
import queue
import sys
import threading
import time
import uuid
from pathlib import Path
from typing import Any

import websocket


DEFAULT_MODEL = "qwen3-asr-flash-realtime"
DEFAULT_SAMPLE_RATE = 16_000
DEFAULT_CHUNK_MS = 100


class Qwen3RealtimeASRClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.stop_audio = threading.Event()
        self.session_finished = threading.Event()
        self.failed = False
        self.ws = websocket.WebSocketApp(
            self._url(),
            header=[
                f"Authorization: Bearer {args.api_key}",
                "OpenAI-Beta: realtime=v1",
            ],
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _url(self) -> str:
        separator = "&" if "?" in self.args.url else "?"
        return f"{self.args.url}{separator}model={self.args.model}"

    def run(self) -> int:
        print(f"连接地址: {self._url()}")
        print(f"模型: {self.args.model}")
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        return 1 if self.failed or not self.session_finished.is_set() else 0

    def _send_event(self, event: dict[str, Any]) -> None:
        event.setdefault("event_id", f"event_{uuid.uuid4().hex}")
        self.ws.send(json.dumps(event, ensure_ascii=False))

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接，发送 session.update。")
        session: dict[str, Any] = {
            "modalities": ["text"],
            "input_audio_format": "pcm",
            "sample_rate": self.args.sample_rate,
            "input_audio_transcription": {
                "model": "qwen3-asr-flash-realtime",
            },
        }
        if self.args.manual:
            session["turn_detection"] = None
        else:
            session["turn_detection"] = {
                "type": "server_vad",
                "threshold": self.args.vad_threshold,
                "silence_duration_ms": self.args.silence_duration_ms,
            }
        if self.args.language:
            session["input_audio_transcription"]["language"] = self.args.language
        self._send_event({"type": "session.update", "session": session})

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
            print(
                f"错误: {error.get('code', '')} {error.get('message', '')}",
                file=sys.stderr,
            )
            ws.close()
            return

        if event_type == "session.updated":
            print("会话已更新，开始发送音频。")
            target = self._stream_file if self.args.pcm_file else self._stream_microphone
            threading.Thread(target=target, daemon=True).start()
            return

        if event_type in {
            "conversation.item.input_audio_transcription.text",
            "conversation.item.input_audio_transcription.completed",
        }:
            if event_type.endswith(".completed"):
                text = message.get("transcript", "")
            else:
                text = f"{message.get('text', '')}{message.get('stash', '')}"
            emotion = message.get("emotion", "")
            marker = "最终" if event_type.endswith(".completed") else "实时"
            if text:
                suffix = f" emotion={emotion}" if emotion else ""
                print(f"[{marker}] {text}{suffix}")
            return

        if event_type == "session.finished":
            print("会话完成。")
            self.session_finished.set()
            ws.close()
            return

        print(f"收到事件: {message}")

    def _on_error(self, ws: websocket.WebSocketApp, error: Any) -> None:
        self.failed = True
        self.stop_audio.set()
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        self.stop_audio.set()
        if not self.session_finished.is_set():
            print(f"连接已关闭: code={status_code}, reason={reason or '-'}")

    def _send_audio(self, chunk: bytes) -> None:
        self._send_event({
            "type": "input_audio_buffer.append",
            "audio": base64.b64encode(chunk).decode(),
        })

    def _finish(self) -> None:
        if not self.ws.sock or not self.ws.sock.connected:
            return
        if self.args.manual:
            self._send_event({"type": "input_audio_buffer.commit"})
        self._send_event({"type": "session.finish"})
        print("已发送 session.finish，等待服务端结束会话……")

    def _stream_file(self) -> None:
        chunk_size = self.args.sample_rate * 2 * self.args.chunk_ms // 1000
        try:
            with self.args.pcm_file.open("rb") as audio:
                while not self.stop_audio.is_set():
                    chunk = audio.read(chunk_size)
                    if not chunk:
                        break
                    self._send_audio(chunk)
                    time.sleep(self.args.chunk_ms / 1000)
        except OSError as exc:
            self.failed = True
            print(f"读取 PCM 文件失败: {exc}", file=sys.stderr)
        finally:
            self._finish()

    def _stream_microphone(self) -> None:
        try:
            import sounddevice as sd
        except ImportError:
            self.failed = True
            print("缺少 sounddevice，请执行: pip install -r py/requirements.txt", file=sys.stderr)
            self.ws.close()
            return

        audio_queue: queue.Queue[bytes] = queue.Queue(maxsize=50)
        frames = self.args.sample_rate * self.args.chunk_ms // 1000

        def callback(indata: bytes, frame_count: int, time_info: Any, status: Any) -> None:
            if status:
                print(f"音频采集警告: {status}", file=sys.stderr)
            try:
                audio_queue.put_nowait(bytes(indata))
            except queue.Full:
                print("音频发送过慢，丢弃一个音频块", file=sys.stderr)

        try:
            with sd.RawInputStream(
                samplerate=self.args.sample_rate,
                blocksize=frames,
                device=self.args.device,
                channels=1,
                dtype="int16",
                callback=callback,
            ):
                while not self.stop_audio.is_set():
                    try:
                        chunk = audio_queue.get(timeout=0.2)
                    except queue.Empty:
                        continue
                    self._send_audio(chunk)
        except Exception as exc:
            self.failed = True
            print(f"麦克风采集失败: {exc}", file=sys.stderr)
            self.ws.close()

    def stop(self) -> None:
        self.stop_audio.set()
        self._finish()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Qwen3-ASR-Flash-Realtime WebSocket 测试")
    parser.add_argument("--api-key", default=os.getenv("DASHSCOPE_API_KEY"))
    parser.add_argument("--workspace-id", default=os.getenv("DASHSCOPE_WORKSPACE_ID"))
    parser.add_argument("--url")
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--language")
    parser.add_argument("--manual", action="store_true", help="关闭服务端 VAD，发送完音频后手动 commit")
    parser.add_argument("--vad-threshold", type=float, default=0.2)
    parser.add_argument("--silence-duration-ms", type=int, default=400)
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--chunk-ms", type=int, default=DEFAULT_CHUNK_MS)
    parser.add_argument("--device")
    parser.add_argument("--pcm-file", type=Path)
    args = parser.parse_args()
    if not args.api_key:
        parser.error("请设置 DASHSCOPE_API_KEY 或传入 --api-key")
    if not args.url:
        if not args.workspace_id:
            parser.error("请设置 DASHSCOPE_WORKSPACE_ID、传入 --workspace-id 或 --url")
        args.url = f"wss://{args.workspace_id}.cn-beijing.maas.aliyuncs.com/api-ws/v1/realtime"
    if args.pcm_file and not args.pcm_file.is_file():
        parser.error(f"PCM 文件不存在: {args.pcm_file}")
    return args


def main() -> int:
    client = Qwen3RealtimeASRClient(parse_args())
    try:
        return client.run()
    except KeyboardInterrupt:
        client.stop()
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
