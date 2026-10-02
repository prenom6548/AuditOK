#!/usr/bin/env python3
"""科大讯飞实时语音转写 WebSocket 验证脚本。"""

from __future__ import annotations

import argparse
import base64
import hashlib
import hmac
import json
import os
import sys
import threading
import time
from pathlib import Path
from urllib.parse import urlencode

import websocket

from asr_common import DEFAULT_SAMPLE_RATE, stream_microphone, stream_pcm_file


class IflytekRTASRClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.stop_audio = threading.Event()
        self.failed = False
        self.ws = websocket.WebSocketApp(
            self._signed_url(),
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _signed_url(self) -> str:
        ts = str(int(time.time()))
        base = hashlib.md5((self.args.appid + ts).encode()).hexdigest()
        signa = base64.b64encode(
            hmac.new(self.args.api_key.encode(), base.encode(), hashlib.sha1).digest()
        ).decode()
        query = urlencode({"appid": self.args.appid, "ts": ts, "signa": signa})
        return "wss://rtasr.xfyun.cn/v1/ws?" + query

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
        action = message.get("action")
        code = message.get("code")
        if action == "error" or (code and code != "0"):
            self.failed = True
            print(f"错误: {code} {message.get('desc', '')}", file=sys.stderr)
            ws.close()
            return
        if action == "result":
            data = json.loads(message.get("data", "{}"))
            text = extract_iflytek_text(data)
            if text:
                print(f"[识别] {text}")
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
            self.ws.send(json.dumps({"end": True}), opcode=websocket.ABNF.OPCODE_BINARY)

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


def extract_iflytek_text(data: dict) -> str:
    if "src" in data:
        return str(data.get("src") or "")
    st = data.get("cn", {}).get("st", {})
    parts: list[str] = []
    for rt in st.get("rt", []):
        for ws in rt.get("ws", []):
            for cw in ws.get("cw", []):
                parts.append(cw.get("w", ""))
    return "".join(parts)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="科大讯飞实时语音转写 WebSocket 测试")
    parser.add_argument("--appid", default=os.getenv("XFYUN_APP_ID"))
    parser.add_argument("--api-key", default=os.getenv("XFYUN_API_KEY"))
    parser.add_argument("--sample-rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--chunk-ms", type=int, default=40)
    parser.add_argument("--device")
    parser.add_argument("--pcm-file", type=Path)
    args = parser.parse_args()
    if not args.appid or not args.api_key:
        parser.error("请设置 XFYUN_APP_ID/XFYUN_API_KEY 或传入参数")
    if args.sample_rate != 16000:
        parser.error("讯飞实时转写当前示例固定使用 16000 Hz")
    if args.pcm_file and not args.pcm_file.is_file():
        parser.error(f"PCM 文件不存在: {args.pcm_file}")
    return args


def main() -> int:
    client = IflytekRTASRClient(parse_args())
    try:
        return client.run()
    except KeyboardInterrupt:
        client.stop()
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
