#!/usr/bin/env python3
"""百度智能云实时语音识别 WebSocket 验证脚本。"""

from __future__ import annotations

import argparse
import json
import os
import sys
import threading
import uuid
from pathlib import Path
from typing import Any

import websocket

from asr_common import DEFAULT_CHUNK_MS, DEFAULT_SAMPLE_RATE, stream_microphone, stream_pcm_file


class BaiduASRClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.stop_audio = threading.Event()
        self.failed = False
        self.ws = websocket.WebSocketApp(
            f"wss://vop.baidu.com/realtime_asr?sn={uuid.uuid4()}",
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def run(self) -> int:
        print(f"连接地址: {self.ws.url}")
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        return 1 if self.failed else 0

    def _send_json(self, message: dict[str, Any]) -> None:
        self.ws.send(json.dumps(message, ensure_ascii=False))

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接，发送 START。")
        self._send_json({
            "type": "START",
            "data": {
                "appid": int(self.args.appid),
                "appkey": self.args.appkey,
                "dev_pid": self.args.dev_pid,
                "cuid": self.args.cuid,
                "format": "pcm",
                "sample": self.args.sample_rate,
            },
        })
        target = self._stream_file if self.args.pcm_file else self._stream_microphone
        threading.Thread(target=target, daemon=True).start()

    def _on_message(self, ws: websocket.WebSocketApp, raw: str) -> None:
        try:
            message = json.loads(raw)
        except json.JSONDecodeError:
            print(f"收到无法解析的消息: {raw!r}")
            return
        err_no = message.get("err_no", 0)
        if err_no:
            self.failed = True
            print(f"错误: {err_no} {message.get('err_msg', '')}", file=sys.stderr)
        text = message.get("result", "")
        if text:
            marker = "最终" if message.get("type") == "FIN_TEXT" else "实时"
            print(f"[{marker}] {text}")
        else:
            print(f"收到事件: {message}")

    def _on_error(self, ws: websocket.WebSocketApp, error: object) -> None:
        self.failed = True
        self.stop_audio.set()
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        self.stop_audio.set()
        print(f"连接已关闭: code={status_code}, reason={reason or '-'}")

    def _finish(self) -> None:
        if self.ws.sock and self.ws.sock.connected:
            self._send_json({"type": "FINISH"})

    def _stream_file(self) -> None:
        stream_pcm_file(self.ws, self.args.pcm_file, self.args.sample_rate,
                        self.args.chunk_ms, self.stop_audio.is_set)
        self._finish()

    def _stream_microphone(self) -> None:
        stream_microphone(self.ws, self.args.sample_rate, self.args.chunk_ms,
                          self.args.device, self.stop_audio.is_set)

    def stop(self) -> None:
        self.stop_audio.set()
        self._finish()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="百度实时语音识别 WebSocket 测试")
    parser.add_argument("--appid", default=os.getenv("BAIDU_APP_ID"))
    parser.add_argument("--appkey", default=os.getenv("BAIDU_API_KEY"))
    parser.add_argument("--dev-pid", type=int, default=15372)
    parser.add_argument("--cuid", default="kylin-agent-probe")
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--chunk-ms", type=int, default=160)
    parser.add_argument("--device")
    parser.add_argument("--pcm-file", type=Path)
    args = parser.parse_args()
    if not args.appid or not args.appkey:
        parser.error("请设置 BAIDU_APP_ID/BAIDU_API_KEY 或传入 --appid/--appkey")
    if args.sample_rate != 16000:
        parser.error("百度实时 ASR 当前示例固定使用 16000 Hz")
    if args.pcm_file and not args.pcm_file.is_file():
        parser.error(f"PCM 文件不存在: {args.pcm_file}")
    return args


def main() -> int:
    client = BaiduASRClient(parse_args())
    try:
        return client.run()
    except KeyboardInterrupt:
        client.stop()
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
