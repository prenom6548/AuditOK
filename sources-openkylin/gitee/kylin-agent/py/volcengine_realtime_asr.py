#!/usr/bin/env python3
"""火山引擎 / 豆包流式语音识别 WebSocket 鉴权验证脚本。

说明：
    火山「大模型流式语音识别」不是简单的 WebSocket 连接后直接发送 PCM。
    它使用 X-Api-* Header 做握手鉴权，并在连接建立后使用自定义二进制
    协议帧传输请求和音频。这个脚本先用于验证 endpoint、Header 和账号权限
    是否有效；完整音频发送需要按官方二进制帧协议继续实现。
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import uuid

import websocket


DEFAULT_URL = "wss://openspeech.bytedance.com/api/v3/sauc/bigmodel"
DEFAULT_RESOURCE_ID = "volc.bigasr.sauc.duration"


class VolcengineASRProbe:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.failed = False
        self.ws = websocket.WebSocketApp(
            args.url,
            header=self._headers(),
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

    def _headers(self) -> list[str]:
        return [
            f"X-Api-App-Key: {self.args.app_key}",
            f"X-Api-Access-Key: {self.args.access_key}",
            f"X-Api-Resource-Id: {self.args.resource_id}",
            f"X-Api-Connect-Id: {self.args.connect_id}",
        ]

    def run(self) -> int:
        print(f"连接地址: {self.args.url}")
        print(f"Resource ID: {self.args.resource_id}")
        print(f"Connect ID: {self.args.connect_id}")
        self.ws.run_forever(ping_interval=20, ping_timeout=10)
        return 1 if self.failed else 0

    def _on_open(self, ws: websocket.WebSocketApp) -> None:
        print("WebSocket 已连接：鉴权/握手通过。")
        if self.args.send_empty_request:
            # 这里只发送一个便于观察服务端错误格式的 JSON 文本，不代表正式音频协议。
            # 正式接入应改成火山官方要求的二进制帧。
            ws.send(json.dumps({"note": "kylin-agent volcengine asr probe"}, ensure_ascii=False))
        else:
            print("未发送音频：火山 ASR 后续需要按官方二进制帧协议封包。")
            ws.close()

    def _on_message(self, ws: websocket.WebSocketApp, raw: str | bytes) -> None:
        if isinstance(raw, bytes):
            print(f"收到二进制消息: {raw[:80]!r} ... 共 {len(raw)} 字节")
            return
        try:
            message = json.loads(raw)
        except json.JSONDecodeError:
            print(f"收到文本消息: {raw}")
            return
        print(f"收到事件: {json.dumps(message, ensure_ascii=False)}")

    def _on_error(self, ws: websocket.WebSocketApp, error: object) -> None:
        self.failed = True
        print(f"WebSocket 错误: {error}", file=sys.stderr)

    def _on_close(self, ws: websocket.WebSocketApp, status_code: int | None, reason: str | None) -> None:
        print(f"连接已关闭: code={status_code}, reason={reason or '-'}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="火山引擎 / 豆包流式 ASR WebSocket 鉴权测试")
    parser.add_argument("--app-key", default=os.getenv("VOLCENGINE_APP_KEY"))
    parser.add_argument("--access-key", default=os.getenv("VOLCENGINE_ACCESS_KEY"))
    parser.add_argument("--resource-id", default=os.getenv("VOLCENGINE_RESOURCE_ID", DEFAULT_RESOURCE_ID))
    parser.add_argument("--connect-id", default=str(uuid.uuid4()))
    parser.add_argument("--url", default=os.getenv("VOLCENGINE_ASR_URL", DEFAULT_URL))
    parser.add_argument(
        "--send-empty-request",
        action="store_true",
        help="连接成功后发送一条 JSON 探针消息，仅用于观察服务端错误格式，不是正式音频协议",
    )
    args = parser.parse_args()
    if not args.app_key or not args.access_key:
        parser.error("请设置 VOLCENGINE_APP_KEY/VOLCENGINE_ACCESS_KEY 或传入 --app-key/--access-key")
    return args


def main() -> int:
    return VolcengineASRProbe(parse_args()).run()


if __name__ == "__main__":
    raise SystemExit(main())
