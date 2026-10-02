#!/usr/bin/env python3
"""通过阿里云百炼 WebSocket API 进行实时语音识别。

默认从麦克风采集 16 kHz、16 bit、单声道 PCM 音频；也可以用裸 PCM 文件
模拟实时输入。凭据仅从环境变量或命令行读取，不会写入源码。
"""

from __future__ import annotations

import argparse
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


DEFAULT_MODEL = "qwen-audio-3.0-asr-flash-streaming"
DEFAULT_SAMPLE_RATE = 16_000
DEFAULT_CHUNK_MS = 100


class RealtimeASRClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.task_id = uuid.uuid4().hex
        self.task_started = threading.Event()
        self.task_finished = threading.Event()
        self.stop_audio = threading.Event()
        self.failed = False

        self.ws = websocket.WebSocketApp(
            args.url,
            header=[f"Authorization: Bearer {args.api_key}"],
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def run(self) -> int:
        print(f"连接地址: {self.args.url}")
        print(f"模型: {self.args.model}")
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        return 1 if self.failed or not self.task_finished.is_set() else 0

    def _send_json(self, message: dict[str, Any]) -> None:
        self.ws.send(json.dumps(message, ensure_ascii=False))

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接，正在启动识别任务……")
        parameters: dict[str, Any] = {
            "format": "pcm",
            "sample_rate": self.args.sample_rate,
            "language_hints": self.args.language,
            "max_sentence_silence": self.args.max_sentence_silence,
        }
        self._send_json(
            {
                "header": {
                    "action": "run-task",
                    "task_id": self.task_id,
                    "streaming": "duplex",
                },
                "payload": {
                    "task_group": "audio",
                    "task": "asr",
                    "function": "recognition",
                    "model": self.args.model,
                    "parameters": parameters,
                    "input": {},
                },
            }
        )

    def _on_message(self, ws: websocket.WebSocketApp, raw: str) -> None:
        try:
            message = json.loads(raw)
            header = message.get("header", {})
            event = header.get("event", "")
        except (json.JSONDecodeError, AttributeError):
            print(f"收到无法解析的消息: {raw!r}", file=sys.stderr)
            return

        if event == "task-started":
            print("任务已启动，请开始说话（按 Ctrl+C 结束）。")
            self.task_started.set()
            target = self._stream_file if self.args.pcm_file else self._stream_microphone
            threading.Thread(target=target, daemon=True).start()
        elif event == "result-generated":
            sentence = (
                message.get("payload", {}).get("output", {}).get("sentence", {})
            )
            text = sentence.get("text", "")
            if text:
                marker = "最终" if sentence.get("sentence_end") else "实时"
                print(f"[{marker}] {text}")
        elif event == "task-finished":
            usage = message.get("payload", {}).get("usage", {})
            if usage:
                print(f"任务完成，计费音频时长: {usage.get('duration', '?')} 秒")
            else:
                print("任务完成。")
            self.task_finished.set()
            ws.close()
        elif event == "task-failed":
            self.failed = True
            self.stop_audio.set()
            print(
                f"任务失败: {header.get('error_code', '')} "
                f"{header.get('error_message', '未知错误')}",
                file=sys.stderr,
            )
            ws.close()
        else:
            print(f"收到事件: {event or message}")

    def _on_error(self, ws: websocket.WebSocketApp, error: Any) -> None:
        self.failed = True
        self.stop_audio.set()
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(
        self,
        ws: websocket.WebSocketApp,
        status_code: int | None,
        reason: str | None,
    ) -> None:
        self.stop_audio.set()
        if not self.task_finished.is_set():
            print(f"连接已关闭: code={status_code}, reason={reason or '-'}")

    def _finish_task(self) -> None:
        if self.ws.sock and self.ws.sock.connected:
            self._send_json(
                {
                    "header": {
                        "action": "finish-task",
                        "task_id": self.task_id,
                        "streaming": "duplex",
                    },
                    "payload": {"input": {}},
                }
            )
            print("已发送 finish-task，等待服务端返回最终结果……")

    def _stream_file(self) -> None:
        chunk_size = self.args.sample_rate * 2 * self.args.chunk_ms // 1000
        try:
            with self.args.pcm_file.open("rb") as audio:
                while not self.stop_audio.is_set():
                    chunk = audio.read(chunk_size)
                    if not chunk:
                        break
                    self.ws.send(chunk, opcode=websocket.ABNF.OPCODE_BINARY)
                    time.sleep(self.args.chunk_ms / 1000)
        except OSError as exc:
            self.failed = True
            print(f"读取 PCM 文件失败: {exc}", file=sys.stderr)
        finally:
            self._finish_task()

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
                    self.ws.send(chunk, opcode=websocket.ABNF.OPCODE_BINARY)
        except Exception as exc:
            self.failed = True
            print(f"麦克风采集失败: {exc}", file=sys.stderr)
            self.ws.close()

    def stop(self) -> None:
        self.stop_audio.set()
        if self.task_started.is_set() and not self.task_finished.is_set():
            self._finish_task()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Qwen 实时语音识别 WebSocket 测试")
    parser.add_argument(
        "--api-key",
        default=os.getenv("DASHSCOPE_API_KEY"),
        help="百炼 API Key，默认读取 DASHSCOPE_API_KEY",
    )
    parser.add_argument(
        "--workspace-id",
        default=os.getenv("DASHSCOPE_WORKSPACE_ID"),
        help="百炼 Workspace ID，默认读取 DASHSCOPE_WORKSPACE_ID",
    )
    parser.add_argument("--url", help="自定义完整 WebSocket 地址")
    parser.add_argument("--model", default=DEFAULT_MODEL, help=f"模型 ID，默认 {DEFAULT_MODEL}")
    parser.add_argument("--language", nargs="+", default=["zh"], help="语言提示，最多 4 个")
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--chunk-ms", type=int, default=DEFAULT_CHUNK_MS)
    parser.add_argument("--max-sentence-silence", type=int, default=800)
    parser.add_argument("--device", help="sounddevice 麦克风设备名称或编号")
    parser.add_argument(
        "--pcm-file",
        type=Path,
        help="用裸 PCM（16 bit、单声道）文件模拟实时输入；不传则使用麦克风",
    )
    args = parser.parse_args()

    if not args.api_key:
        parser.error("请设置 DASHSCOPE_API_KEY 或传入 --api-key")
    if not args.url:
        if not args.workspace_id:
            parser.error("请设置 DASHSCOPE_WORKSPACE_ID、传入 --workspace-id 或 --url")
        args.url = (
            f"wss://{args.workspace_id}.cn-beijing.maas.aliyuncs.com"
            "/api-ws/v1/inference"
        )
    if not 1 <= len(args.language) <= 4:
        parser.error("--language 需提供 1 到 4 个语言代码")
    if args.sample_rate <= 0 or args.chunk_ms <= 0:
        parser.error("采样率和音频块时长必须大于 0")
    if args.pcm_file and not args.pcm_file.is_file():
        parser.error(f"PCM 文件不存在: {args.pcm_file}")
    return args


def main() -> int:
    args = parse_args()
    client = RealtimeASRClient(args)
    try:
        return client.run()
    except KeyboardInterrupt:
        print("\n正在结束识别……")
        client.stop()
        client.task_finished.wait(timeout=10)
        client.ws.close()
        return 1 if client.failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
