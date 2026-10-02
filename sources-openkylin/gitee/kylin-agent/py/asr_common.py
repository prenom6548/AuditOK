#!/usr/bin/env python3
"""Shared helpers for realtime ASR probe scripts."""

from __future__ import annotations

import queue
import sys
import time
from pathlib import Path
from typing import Callable

import websocket


DEFAULT_SAMPLE_RATE = 16_000
DEFAULT_CHUNK_MS = 100


def stream_pcm_file(
    ws: websocket.WebSocketApp,
    pcm_file: Path,
    sample_rate: int,
    chunk_ms: int,
    stop_requested: Callable[[], bool],
    send_binary: Callable[[bytes], None] | None = None,
) -> None:
    chunk_size = sample_rate * 2 * chunk_ms // 1000
    sender = send_binary or (lambda chunk: ws.send(chunk, opcode=websocket.ABNF.OPCODE_BINARY))
    with pcm_file.open("rb") as audio:
        while not stop_requested():
            chunk = audio.read(chunk_size)
            if not chunk:
                break
            sender(chunk)
            time.sleep(chunk_ms / 1000)


def stream_microphone(
    ws: websocket.WebSocketApp,
    sample_rate: int,
    chunk_ms: int,
    device: str | None,
    stop_requested: Callable[[], bool],
    send_binary: Callable[[bytes], None] | None = None,
) -> None:
    try:
        import sounddevice as sd
    except ImportError:
        print("缺少 sounddevice，请执行: pip install -r py/requirements.txt", file=sys.stderr)
        ws.close()
        return

    sender = send_binary or (lambda chunk: ws.send(chunk, opcode=websocket.ABNF.OPCODE_BINARY))
    audio_queue: queue.Queue[bytes] = queue.Queue(maxsize=50)
    frames = sample_rate * chunk_ms // 1000

    def callback(indata: bytes, frame_count: int, time_info: object, status: object) -> None:
        if status:
            print(f"音频采集警告: {status}", file=sys.stderr)
        try:
            audio_queue.put_nowait(bytes(indata))
        except queue.Full:
            print("音频发送过慢，丢弃一个音频块", file=sys.stderr)

    with sd.RawInputStream(
        samplerate=sample_rate,
        blocksize=frames,
        device=device,
        channels=1,
        dtype="int16",
        callback=callback,
    ):
        while not stop_requested():
            try:
                chunk = audio_queue.get(timeout=0.2)
            except queue.Empty:
                continue
            sender(chunk)
