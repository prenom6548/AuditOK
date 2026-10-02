"""Intent guard: 每次 LLM 调用前执行外部脚本，命中则短路返回。

协议：
  stdin  → JSON: {"user_message": "...", "session_id": "...", ...}
  stdout → None / null  → 穿透，正常 Agent 流程
         → 其他字符串    → 直接作为 final_response 返回给用户

环境变量控制：
  HERMES_INTENT_GUARD_SCRIPT    外部脚本路径（不设则禁用）
  HERMES_INTENT_GUARD_TIMEOUT   脚本超时秒数（默认 5）
"""

import json
import logging
import os
import subprocess
from pathlib import Path

logger = logging.getLogger(__name__)

SCRIPT_PATH = "intent-act"
SCRIPT_TIMEOUT = 50


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

def _run_script(input_data: dict) -> str | None:
    """调用外部脚本，返回 stdout（剔除空白）或 None（穿透）"""
    try:
        result = subprocess.run(
            [SCRIPT_PATH],
            input=f"--input \"{input_data}\" --raw",
            capture_output=True,
            text=True,
            timeout=SCRIPT_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        logger.warning("Intent guard script timed out after %ss", SCRIPT_TIMEOUT)
        return None
    except FileNotFoundError:
        logger.debug("Intent guard script not found at %s", SCRIPT_PATH)
        return None
    except Exception as exc:
        logger.warning("Intent guard script failed: %s", exc)
        return None

    stdout = result.stdout.strip()
    if result.returncode != 0 or not stdout or stdout in ("None", "null"):
        return None

    logger.info("Intent guard matched: %s", stdout[:100])
    return stdout


def register(ctx):
    if not SCRIPT_PATH or not _executable_exists(SCRIPT_PATH):
        logger.info(
            "Intent guard script not found: %s — plugin loaded but inactive. "
            "Set HERMES_INTENT_GUARD_SCRIPT env var to a file path or PATH command.",
            SCRIPT_PATH,
        )
        # 仍然注册钩子，以便运行时随时通过环境变量切换
    else:
        logger.info("Intent guard plugin active, script=%s timeout=%ss", SCRIPT_PATH, SCRIPT_TIMEOUT)

    def on_pre_llm_call(**kwargs):
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

    ctx.register_hook("pre_llm_call", on_pre_llm_call)
