#!/usr/bin/env python3
"""科大讯飞在线语音合成 WebSocket 验证脚本。"""

from __future__ import annotations

import argparse
import base64
import datetime as dt
import hashlib
import hmac
import json
import os
import ssl
import sys
from email.utils import format_datetime
from pathlib import Path
from urllib.parse import quote, urlparse

import websocket


DEFAULT_TEXT = "你好，我是 KylinAgent。科大讯飞语音合成正在测试中。"
DEFAULT_URL = "wss://tts-api.xfyun.cn/v2/tts"


class IflytekTTSClient:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.audio = bytearray()
        self.failed = False
        self.done = False
        self.ws = websocket.WebSocketApp(
            self._signed_url(),
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _signed_url(self) -> str:
        parsed = urlparse(self.args.url)
        date = format_datetime(dt.datetime.now(dt.timezone.utc), usegmt=True)
        signature_origin = f"host: {parsed.netloc}\ndate: {date}\nGET {parsed.path} HTTP/1.1"
        signature = base64.b64encode(
            hmac.new(self.args.api_secret.encode(), signature_origin.encode(), hashlib.sha256).digest()
        ).decode()
        authorization_origin = (
            f'api_key="{self.args.api_key}", algorithm="hmac-sha256", '
            f'headers="host date request-line", signature="{signature}"'
        )
        authorization = base64.b64encode(authorization_origin.encode()).decode()
        return (
            f"{self.args.url}?authorization={quote(authorization)}"
            f"&date={quote(date)}&host={quote(parsed.netloc)}"
        )

    def run(self) -> int:
        print(f"连接地址: {self.args.url}")
        print(f"音色: {self.args.voice}")
        self.ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE}, ping_interval=20, ping_timeout=10)
        if self.audio:
            self.args.output.write_bytes(self.audio)
            print(f"合成成功: {self.args.output} ({len(self.audio)} bytes)")
        return 1 if self.failed or not self.done else 0

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        payload = {
            "common": {"app_id": self.args.appid},
            "business": {
                "aue": self.args.aue,
                "auf": self.args.auf,
                "vcn": self.args.voice,
                "tte": "UTF8",
                "speed": self.args.speed,
                "volume": self.args.volume,
                "pitch": self.args.pitch,
            },
            "data": {
                "status": 2,
                "text": base64.b64encode(self.args.text.encode("utf-8")).decode(),
            },
        }
        ws.send(json.dumps(payload, ensure_ascii=False))
        print("已发送合成请求。")

    def _on_message(self, ws: websocket.WebSocketApp, raw: str) -> None:
        try:
            message = json.loads(raw)
        except json.JSONDecodeError:
            print(f"收到无法解析的消息: {raw!r}", file=sys.stderr)
            return
        code = message.get("code", 0)
        if code != 0:
            self.failed = True
            print(f"错误: {code} {message.get('message', '')}", file=sys.stderr)
            ws.close()
            return
        audio = message.get("data", {}).get("audio", "")
        if audio:
            chunk = base64.b64decode(audio)
            self.audio.extend(chunk)
            print(f"收到音频分片: {len(chunk)} bytes，总计 {len(self.audio)} bytes")
        if message.get("data", {}).get("status") == 2:
            self.done = True
            ws.close()

    def _on_error(self, ws: websocket.WebSocketApp, error: object) -> None:
        self.failed = True
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        if not self.done:
            print(f"连接已关闭: code={status_code}, reason={reason or '-'}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="科大讯飞在线 TTS WebSocket 测试")
    parser.add_argument("--appid", default=os.getenv("XFYUN_APP_ID"))
    parser.add_argument("--api-key", default=os.getenv("XFYUN_API_KEY"))
    parser.add_argument("--api-secret", default=os.getenv("XFYUN_API_SECRET"))
    parser.add_argument("--url", default=DEFAULT_URL)
    parser.add_argument("--text", default=DEFAULT_TEXT)
    parser.add_argument("--voice", default="xiaoyan")
    parser.add_argument("--aue", default="lame", help="lame=mp3, raw=pcm")
    parser.add_argument("--auf", default="audio/L16;rate=16000")
    parser.add_argument("--speed", type=int, default=50)
    parser.add_argument("--volume", type=int, default=50)
    parser.add_argument("--pitch", type=int, default=50)
    parser.add_argument("--output", type=Path, default=Path("py/iflytek_tts_result.mp3"))
    args = parser.parse_args()
    if not args.appid or not args.api_key or not args.api_secret:
        parser.error("请设置 XFYUN_APP_ID/XFYUN_API_KEY/XFYUN_API_SECRET 或传入参数")
    return args


def main() -> int:
    return IflytekTTSClient(parse_args()).run()


if __name__ == "__main__":
    raise SystemExit(main())
