#!/usr/bin/env python3
"""腾讯云实时语音识别 WebSocket 验证脚本。"""

from __future__ import annotations

import argparse
import base64
import hashlib
import hmac
import json
import os
import random
import sys
import threading
import time
import uuid
from pathlib import Path
from urllib.parse import quote

import websocket

from asr_common import DEFAULT_CHUNK_MS, DEFAULT_SAMPLE_RATE, stream_microphone, stream_pcm_file


class TencentASRClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.voice_id = str(uuid.uuid4())
        self.stop_audio = threading.Event()
        self.finished = threading.Event()
        self.failed = False
        self.ws = websocket.WebSocketApp(
            self._signed_url(),
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _signed_url(self) -> str:
        now = int(time.time())
        params = {
            "engine_model_type": self.args.engine_model_type,
            "expired": now + self.args.expire_seconds,
            "filter_dirty": 0,
            "filter_modal": 0,
            "filter_punc": 0,
            "needvad": 1,
            "nonce": random.randint(100000, 999999999),
            "secretid": self.args.secret_id,
            "timestamp": now,
            "voice_format": 1,
            "voice_id": self.voice_id,
        }
        query = "&".join(f"{key}={params[key]}" for key in sorted(params))
        raw = f"asr.cloud.tencent.com/asr/v2/{self.args.appid}?{query}"
        signature = base64.b64encode(
            hmac.new(self.args.secret_key.encode(), raw.encode(), hashlib.sha1).digest()
        ).decode()
        return f"wss://{raw}&signature={quote(signature, safe='')}"

    def run(self) -> int:
        print(f"连接地址: {self.ws.url}")
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        return 1 if self.failed else 0

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接，开始发送音频。")
        target = self._stream_file if self.args.pcm_file else self._stream_microphone
        threading.Thread(target=target, daemon=True).start()

    def _on_message(self, ws: websocket.WebSocketApp, raw: str) -> None:
        try:
            message = json.loads(raw)
        except json.JSONDecodeError:
            print(f"收到无法解析的消息: {raw!r}")
            return
        code = message.get("code", 0)
        if code != 0:
            self.failed = True
            print(f"错误: {code} {message.get('message', '')}", file=sys.stderr)
            ws.close()
            return
        if "result" in message:
            print(f"[识别] {message.get('result')}")
        else:
            print(f"收到事件: {message}")

    def _on_error(self, ws: websocket.WebSocketApp, error: object) -> None:
        self.failed = True
        self.stop_audio.set()
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        self.stop_audio.set()
        self.finished.set()
        print(f"连接已关闭: code={status_code}, reason={reason or '-'}")

    def _stream_file(self) -> None:
        stream_pcm_file(self.ws, self.args.pcm_file, self.args.sample_rate,
                        self.args.chunk_ms, self.stop_audio.is_set)
        self.ws.close()

    def _stream_microphone(self) -> None:
        stream_microphone(self.ws, self.args.sample_rate, self.args.chunk_ms,
                          self.args.device, self.stop_audio.is_set)

    def stop(self) -> None:
        self.stop_audio.set()
        self.ws.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="腾讯云实时语音识别 WebSocket 测试")
    parser.add_argument("--appid", default=os.getenv("TENCENTCLOUD_APPID"))
    parser.add_argument("--secret-id", default=os.getenv("TENCENTCLOUD_SECRET_ID"))
    parser.add_argument("--secret-key", default=os.getenv("TENCENTCLOUD_SECRET_KEY"))
    parser.add_argument("--engine-model-type", default="16k_zh")
    parser.add_argument("--expire-seconds", type=int, default=3600)
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--chunk-ms", type=int, default=DEFAULT_CHUNK_MS)
    parser.add_argument("--device")
    parser.add_argument("--pcm-file", type=Path)
    args = parser.parse_args()
    if not args.appid or not args.secret_id or not args.secret_key:
        parser.error("请设置 TENCENTCLOUD_APPID/TENCENTCLOUD_SECRET_ID/TENCENTCLOUD_SECRET_KEY 或传入参数")
    if args.pcm_file and not args.pcm_file.is_file():
        parser.error(f"PCM 文件不存在: {args.pcm_file}")
    return args


def main() -> int:
    client = TencentASRClient(parse_args())
    try:
        return client.run()
    except KeyboardInterrupt:
        client.stop()
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
