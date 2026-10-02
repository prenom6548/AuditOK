#!/usr/bin/env python3
"""腾讯云语音合成 REST API 验证脚本。"""

from __future__ import annotations

import argparse
import base64
import datetime as dt
import hashlib
import hmac
import json
import os
import sys
import urllib.request
from pathlib import Path


DEFAULT_TEXT = "你好，我是 KylinAgent。腾讯云语音合成正在测试中。"
SERVICE = "tts"
HOST = "tts.tencentcloudapi.com"
ENDPOINT = f"https://{HOST}"
VERSION = "2019-08-23"
ACTION = "TextToVoice"


def sign(key: bytes, message: str) -> bytes:
    return hmac.new(key, message.encode("utf-8"), hashlib.sha256).digest()


def tc3_headers(args: argparse.Namespace, payload: str) -> dict[str, str]:
    now = dt.datetime.utcnow()
    timestamp = int(now.timestamp())
    date = now.strftime("%Y-%m-%d")
    canonical_request = "\n".join([
        "POST",
        "/",
        "",
        f"content-type:application/json; charset=utf-8\nhost:{HOST}\n",
        "content-type;host",
        hashlib.sha256(payload.encode()).hexdigest(),
    ])
    credential_scope = f"{date}/{SERVICE}/tc3_request"
    string_to_sign = "\n".join([
        "TC3-HMAC-SHA256",
        str(timestamp),
        credential_scope,
        hashlib.sha256(canonical_request.encode()).hexdigest(),
    ])
    secret_date = sign(("TC3" + args.secret_key).encode(), date)
    secret_service = hmac.new(secret_date, SERVICE.encode(), hashlib.sha256).digest()
    secret_signing = hmac.new(secret_service, b"tc3_request", hashlib.sha256).digest()
    signature = hmac.new(secret_signing, string_to_sign.encode(), hashlib.sha256).hexdigest()
    authorization = (
        "TC3-HMAC-SHA256 "
        f"Credential={args.secret_id}/{credential_scope}, "
        "SignedHeaders=content-type;host, "
        f"Signature={signature}"
    )
    return {
        "Authorization": authorization,
        "Content-Type": "application/json; charset=utf-8",
        "Host": HOST,
        "X-TC-Action": ACTION,
        "X-TC-Timestamp": str(timestamp),
        "X-TC-Version": VERSION,
        "X-TC-Region": args.region,
    }


def synthesize(args: argparse.Namespace) -> bytes:
    payload = json.dumps({
        "Text": args.text,
        "SessionId": args.session_id,
        "ModelType": args.model_type,
        "VoiceType": args.voice_type,
        "Codec": args.codec,
        "SampleRate": args.sample_rate,
        "Speed": args.speed,
        "Volume": args.volume,
        "PrimaryLanguage": args.primary_language,
    }, ensure_ascii=False, separators=(",", ":"))
    request = urllib.request.Request(
        ENDPOINT,
        data=payload.encode(),
        headers=tc3_headers(args, payload),
    )
    with urllib.request.urlopen(request, timeout=40) as response:
        body = json.loads(response.read().decode())
    if "Error" in body.get("Response", {}):
        raise RuntimeError(f"腾讯云 TTS 失败: {body['Response']['Error']}")
    audio = body.get("Response", {}).get("Audio")
    if not audio:
        raise RuntimeError(f"腾讯云 TTS 未返回 Audio: {body}")
    return base64.b64decode(audio)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="腾讯云 TTS REST 测试")
    parser.add_argument("--secret-id", default=os.getenv("TENCENTCLOUD_SECRET_ID"))
    parser.add_argument("--secret-key", default=os.getenv("TENCENTCLOUD_SECRET_KEY"))
    parser.add_argument("--region", default=os.getenv("TENCENTCLOUD_REGION", "ap-beijing"))
    parser.add_argument("--text", default=DEFAULT_TEXT)
    parser.add_argument("--session-id", default="kylin-agent-tts-probe")
    parser.add_argument("--model-type", type=int, default=1)
    parser.add_argument("--voice-type", type=int, default=101001)
    parser.add_argument("--codec", choices=["wav", "mp3", "pcm"], default="wav")
    parser.add_argument("--sample-rate", type=int, default=16000)
    parser.add_argument("--speed", type=float, default=0)
    parser.add_argument("--volume", type=float, default=0)
    parser.add_argument("--primary-language", type=int, default=1)
    parser.add_argument("--output", type=Path, default=Path("py/tencent_tts_result.wav"))
    args = parser.parse_args()
    if not args.secret_id or not args.secret_key:
        parser.error("请设置 TENCENTCLOUD_SECRET_ID/TENCENTCLOUD_SECRET_KEY 或传入参数")
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
