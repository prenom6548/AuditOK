"""Intent guard: 每次 LLM 调用前执行外部脚本，命中则短路返回。

协议：
  stdin  → JSON: {"user_message": "...", "session_id": "...", ...}
  stdout → None / null  → 穿透，正常 Agent 流程
         → 其他字符串    → 直接作为 final_response 返回给用户

环境变量控制：
  HERMES_INTENT_GUARD_SCRIPT    外部脚本路径（不设则禁用）
  HERMES_INTENT_GUARD_TIMEOUT   脚本超时秒数（默认 5）
"""

import argparse
import json
import logging
import os
import subprocess
from pathlib import Path

logger = logging.getLogger(__name__)

SCRIPT_PATH = "intent-act"
SCRIPT_TIMEOUT = float(os.environ.get("HERMES_INTENT_GUARD_TIMEOUT", "50"))


def _executable_exists(path_or_cmd: str) -> bool:
    """判断给定的路径字符串是否可执行。
    如果包含 / 则当作文件路径检查，否则在 PATH 中查找。"""
    if "/" in path_or_cmd or "\\" in path_or_cmd:
        return Path(path_or_cmd).is_file() and os.access(path_or_cmd, os.X_OK)
    import shutil
    return shutil.which(path_or_cmd) is not None


def _call_actuator(input: str) -> str | None:
    try:
        result = subprocess.run(
            [SCRIPT_PATH, "--input", input, "--raw"],
            capture_output=True,
            text=True,
            timeout=SCRIPT_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        print(f"执行超时：{SCRIPT_PATH} 超过 {SCRIPT_TIMEOUT}s 未返回")
        return None
    except Exception as exc:
        print(f"执行失败：{exc}")
        return None

    if result.returncode != 0:
        print(f"执行失败：returncode={result.returncode}, stderr={result.stderr.strip()}")
        return None

    return result.stdout.strip()

def llm_call(**kwargs):
    if not SCRIPT_PATH or not _executable_exists(SCRIPT_PATH):
        logger.info(
        "Intent guard script not found: %s — plugin loaded but inactive. "
        "Set HERMES_INTENT_GUARD_SCRIPT env var to a file path or PATH command.",
        SCRIPT_PATH,
    )
        print("执行文件未找到")
        return None

    user_message = kwargs.get("user_message", "")
    if not user_message:
        print("没有用户输入信息")
        return None

    print(user_message)
    response = _call_actuator(user_message)
    if response is None or response == "":
        print("没有反馈")
        return None  # 穿透 → 正常 Agent 流程

    return {"final_response": response}  # 短路 → 直接返回


def main() -> None:
    parser = argparse.ArgumentParser(description="测试 intent-guard llm_call")
    parser.add_argument("--user-message", "-u", help="传入 user_message", default="")
    parser.add_argument("--script", "-s", help="覆盖 HERMES_INTENT_GUARD_SCRIPT，指定守卫脚本路径或命令")
    args = parser.parse_args()

    if args.script:
        global SCRIPT_PATH
        SCRIPT_PATH = args.script

    user_message = args.user_message or "请帮我打开浏览器"
    if not user_message:
        user_message = input("请输入用户消息: ").strip()

    result = llm_call(user_message=user_message)
    print("llm_call 返回:", result)


if __name__ == "__main__":
    main()

