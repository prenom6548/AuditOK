#!/usr/bin/env python3
"""百度千帆/智能云短文本在线语音合成验证脚本。

默认使用新版 API Key 鉴权：
    Authorization: Bearer <BAIDU_API_KEY>

如需兼容旧 OAuth 流程，也可以传入 --legacy-oauth，并提供 Secret Key 换取 access_token。
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.parse
import urllib.request
from pathlib import Path


DEFAULT_TEXT = "你好，我是 KylinAgent。百度语音合成正在测试中。"


def fetch_access_token(api_key: str, secret_key: str) -> str:
    data = urllib.parse.urlencode({
        "grant_type": "client_credentials",
        "client_id": api_key,
        "client_secret": secret_key,
    }).encode()
    request = urllib.request.Request(
        "https://aip.baidubce.com/oauth/2.0/token",
        data=data,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
    )
    with urllib.request.urlopen(request, timeout=20) as response:
        payload = json.loads(response.read().decode())
    token = payload.get("access_token")
    if not token:
        raise RuntimeError(f"获取百度 access_token 失败: {payload}")
    return token


def synthesize(args: argparse.Namespace) -> bytes:
    data = urllib.parse.urlencode({
        "tex": args.text,
        "cuid": args.cuid,
        "ctp": 1,
        "lan": "zh",
        "spd": args.speed,
        "pit": args.pitch,
        "vol": args.volume,
        "per": args.voice,
        "aue": args.audio_format,
    }).encode()
    headers = {"Content-Type": "application/x-www-form-urlencoded"}
    if args.legacy_oauth:
        token = args.token or fetch_access_token(args.api_key, args.secret_key)
        data += f"&tok={urllib.parse.quote(token)}".encode()
    else:
        headers["Authorization"] = f"Bearer {args.api_key}"
    request = urllib.request.Request(
        "https://tsn.baidu.com/text2audio",
        data=data,
        headers=headers,
    )
    with urllib.request.urlopen(request, timeout=40) as response:
        content_type = response.headers.get("Content-Type", "")
        body = response.read()
    if "application/json" in content_type or body.startswith(b"{"):
        raise RuntimeError(f"百度 TTS 失败: {body.decode(errors='replace')}")
    return body


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="百度智能云 TTS REST 测试")
    parser.add_argument("--api-key", default=os.getenv("BAIDU_API_KEY"))
    parser.add_argument("--secret-key", default=os.getenv("BAIDU_SECRET_KEY"))
    parser.add_argument("--token", default=os.getenv("BAIDU_ACCESS_TOKEN"))
    parser.add_argument("--legacy-oauth", action="store_true", help="使用旧 OAuth token 流程")
    parser.add_argument("--text", default=DEFAULT_TEXT)
    parser.add_argument("--voice", type=int, default=0, help="发音人 per，0 普通女声，1 普通男声等")
    parser.add_argument("--speed", type=int, default=5)
    parser.add_argument("--pitch", type=int, default=5)
    parser.add_argument("--volume", type=int, default=5)
    parser.add_argument("--audio-format", type=int, default=6, help="aue，6=wav，3=mp3 等")
    parser.add_argument("--cuid", default="kylin-agent-tts-probe")
    parser.add_argument("--output", type=Path, default=Path("py/baidu_tts_result.wav"))
    args = parser.parse_args()
    if not args.api_key:
        parser.error("请设置 BAIDU_API_KEY 或传入 --api-key")
    if args.legacy_oauth and not args.token and not args.secret_key:
        parser.error("--legacy-oauth 需要 BAIDU_SECRET_KEY，或提供 BAIDU_ACCESS_TOKEN/--token")
    return args


def main() -> int:
    args = parse_args()
    try:
        audio = synthesize(args)
    except Exception as exc:
        print(exc, file=sys.stderr)
        return 1
    args.output.write_bytes(audio)
    print(f"合成成功: {args.output} ({len(audio)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
