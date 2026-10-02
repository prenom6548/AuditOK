"""
统一日志系统 - 简化版
功能：
1. 单个日志文件记录所有关键信息
2. 自动记录每步执行时间
3. 结构化输出：步骤、时间、LLM 回答、动作、总计
"""
from __future__ import annotations

import logging
import os
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any, Optional
from contextlib import contextmanager
import functools


class SessionLogger:
    """会话日志管理器 - 单例模式"""
    
    _instance: Optional["SessionLogger"] = None
    _initialized: bool = False
    
    def __new__(cls, *args, **kwargs) -> "SessionLogger":
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(
        self,
        log_file: Optional[str] = None,
        level: str = "INFO",
        console_output: bool = True,
    ):
        if SessionLogger._initialized:
            return
        
        self.start_time = datetime.now()
        self.step_timings: dict[str, float] = {}
        self.step_details: list[dict[str, Any]] = []
        self.current_step: Optional[dict[str, Any]] = None
        self.total_tokens_used = 0
        
        # 设置日志级别
        self.level = getattr(logging, level.upper(), logging.INFO)
        
        # 创建日志目录
        if log_file:
            self.log_path = Path(log_file).expanduser().resolve()
            self.log_path.parent.mkdir(parents=True, exist_ok=True)
        else:
            self.log_path = None
        
        # 使用 kylin 命名空间，由 src.logging_config 统一管理控制台和文件输出。
        self.logger = logging.getLogger("kylin.session")
        self.logger.handlers = []
        self.logger.setLevel(self.level)
        self.logger.propagate = True
        
        SessionLogger._initialized = True
    
    def reset(self):
        """重置日志状态（用于新会话）"""
        self.start_time = datetime.now()
        self.step_timings = {}
        self.step_details = []
        self.current_step = None
        self.total_tokens_used = 0
    
    def session_start(self, task: str, config: dict[str, Any]) -> None:
        """记录会话开始"""
        self.logger.info("")
        self.logger.info("╔" + "═" * 70 + "╗")
        self.logger.info("║  🚀 任务开始" + " " * 57 + "║")
        self.logger.info("╠" + "═" * 70 + "╣")
        self.logger.info(f"║  时间：{self.start_time.strftime('%Y-%m-%d %H:%M:%S')}{' ' * 47}║")
        task_short = task[:56]
        self.logger.info(f"║  任务：{task_short}{' ' * (56 - len(task_short))}║")
        self.logger.info("╚" + "═" * 70 + "╝")
        self.logger.info("")
    
    def step_start(self, step_num: int, goal: str) -> None:
        """开始一个新步骤"""
        self.current_step = {
            "step_num": step_num,
            "goal": goal,
            "start_time": time.time(),
            "llm_responses": [],
            "actions": [],
            "errors": [],
        }
        self.logger.info(f"┌── 步骤 {step_num:03d}: {goal[:60]}")
    
    def log_llm_response(self, role: str, content: str, tokens: Optional[tuple[int, int, int]] = None, elapsed: Optional[float] = None) -> None:
        """记录 LLM 响应"""
        pass
        # if self.current_step:
        #     self.current_step["llm_responses"].append({
        #         "role": role,
        #         "content": content,
        #         "tokens": tokens,
        #         "elapsed": elapsed,
        #     })
        #     if tokens:
        #         self.total_tokens_used += sum(tokens)
        
        # # 简化输出
        # emoji = "🧠" if role == "Brain" else "🎯" if role == "Actor" else "🤖"
        # if tokens:
        #     self.logger.info(f"   {emoji} {role}: {elapsed:.2f}s | Tokens: {tokens[0]:,}/{tokens[1]:,}")
        # else:
        #     self.logger.info(f"   {emoji} {role}: {elapsed:.2f}s" if elapsed else f"   {emoji} {role}")
    
    def log_action(self, action_name: str, action_params: dict[str, Any], result: str = "", elapsed: Optional[float] = None) -> None:
        """记录执行的动作"""
        pass
        # if self.current_step:
        #     self.current_step["actions"].append({
        #         "name": action_name,
        #         "params": action_params,
        #         "result": result,
        #         "elapsed": elapsed,
        #     })
        
        # # 简化动作图标
        # action_icons = {
        #     "click": "🖱️",
        #     "left_click": "🖱️",
        #     "right_click": "🖱️",
        #     "type": "⌨️",
        #     "input_text": "⌨️",
        #     "press": "🔑",
        #     "scroll": "📜",
        #     "move_mouse": "👆",
        #     "drag": "🖐️",
        #     "done": "✅",
        #     "open_app": "🚀",
        # }
        # icon = action_icons.get(action_name.lower())
        # if not icon:
        #     for key in action_icons:
        #         if key in action_name.lower():
        #             icon = action_icons[key]
        #             break
        # if not icon:
        #     icon = "▶️"
        
        # time_str = f" ({elapsed:.2f}s)" if elapsed else ""
        # self.logger.info(f"   {icon} {action_name}{time_str}")
    
    def log_error(self, error: str) -> None:
        """记录错误"""
        if self.current_step:
            self.current_step["errors"].append(error)
        self.logger.error(f"  ❌ [错误] {error}")
    
    def step_end(self, success: bool = True) -> float:
        """结束当前步骤，返回耗时（秒）"""
        if not self.current_step:
            return 0.0
        
        elapsed = time.time() - self.current_step["start_time"]
        step_num = self.current_step["step_num"]
        
        self.step_timings[f"step_{step_num}"] = elapsed
        self.step_details.append(self.current_step)
        
        # 简化步骤完成输出
        # status_icon = "✅" if success else "❌"
        # self.logger.info(f"   {status_icon} 完成：{elapsed:.2f}s")
        # self.logger.info("")
        
        self.current_step = None
        return elapsed
    
    def session_end(self, final_status: str = "success") -> dict[str, Any]:
        """结束会话，返回统计信息"""
        end_time = datetime.now()
        total_elapsed = (end_time - self.start_time).total_seconds()
        
        self.logger.info("═" * 72)
        status_icon = "✅" if final_status == "success" else "⚠️" if final_status == "error" else "🛑"
        self.logger.info(f"{status_icon} 会话结束：{final_status}")
        self.logger.info(f"📊 总步骤：{len(self.step_details)} | 总耗时：{total_elapsed:.1f}s ({total_elapsed/60:.2f}min)")
        self.logger.info(f"⚡ 平均每步：{total_elapsed / len(self.step_details) if self.step_details else 0:.2f}s")
        self.logger.info(f"💰 总 Token：约 {self.total_tokens_used:,}")
        self.logger.info("")
        
        return {
            "final_status": final_status,
            "total_steps": len(self.step_details),
            "total_elapsed_seconds": total_elapsed,
            "total_tokens_used": self.total_tokens_used,
            "step_timings": self.step_timings,
        }
    
    @contextmanager
    def step_context(self, step_num: int, goal: str):
        """上下文管理器用于步骤计时"""
        self.step_start(step_num, goal)
        try:
            yield self
            self.step_end(success=True)
        except Exception as e:
            self.log_error(str(e))
            self.step_end(success=False)
            raise
    
    def get_stats(self) -> dict[str, Any]:
        """获取当前统计信息"""
        total_elapsed = (datetime.now() - self.start_time).total_seconds()
        return {
            "total_steps": len(self.step_details),
            "total_elapsed_seconds": total_elapsed,
            "total_tokens_used": self.total_tokens_used,
            "step_timings": self.step_timings,
        }


# 便捷函数
def time_step(func):
    """装饰器：自动记录步骤执行时间"""
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        step_num = kwargs.get("step_num", args[0].n_steps if hasattr(args[0], "n_steps") else 1)
        goal = kwargs.get("goal", "未知目标")
        
        logger = get_session_logger()
        with logger.step_context(step_num, goal):
            return func(*args, **kwargs)
    return wrapper


# 全局实例
_session_logger: Optional[SessionLogger] = None


def init_session_logger(
    log_file: Optional[str] = None,
    level: str = "INFO",
    console_output: bool = True,
) -> SessionLogger:
    """初始化会话日志器"""
    global _session_logger
    _session_logger = SessionLogger(
        log_file=log_file,
        level=level,
        console_output=console_output,
    )
    return _session_logger


def get_session_logger() -> SessionLogger:
    """获取会话日志器实例"""
    global _session_logger
    if _session_logger is None:
        _session_logger = SessionLogger()
    return _session_logger


# 兼容旧的 logging setup
def setup_logging():
    """兼容旧的 logging 接口"""
    import logging
    
    try:
        addLoggingLevel('RESULT', 35)
    except AttributeError:
        pass

    log_type = os.getenv('kylin_LOGGING_LEVEL', 'info').lower()

    root = logging.getLogger()
    for handler in list(root.handlers):
        root.removeHandler(handler)
        try:
            handler.close()
        except Exception:
            pass

    class BrowserUseFormatter(logging.Formatter):
        def format(self, record):
            if record.name.startswith('kylin.'):
                record.name = record.name.split('.')[-2]
            return super().format(record)

    if sys.platform == 'win32':
        import io
        utf8_stdout = io.TextIOWrapper(
            sys.stdout.buffer,
            encoding='utf-8',
            errors='replace'
        )
        console = logging.StreamHandler(utf8_stdout)
    else:
        console = logging.StreamHandler(sys.stdout)

    if log_type == 'result':
        console.setLevel('RESULT')
        console.setFormatter(BrowserUseFormatter('%(message)s'))
    else:
        console.setFormatter(BrowserUseFormatter('%(levelname)-8s [%(name)s] %(message)s'))

    root.addHandler(console)

    if log_type == 'result':
        root.setLevel('RESULT')
    elif log_type == 'debug':
        root.setLevel(logging.DEBUG)
    else:
        root.setLevel(logging.INFO)

    kylin_logger = logging.getLogger('kylin')
    for handler in list(kylin_logger.handlers):
        kylin_logger.removeHandler(handler)
        try:
            handler.close()
        except Exception:
            pass
    kylin_logger.propagate = False
    kylin_logger.addHandler(console)
    kylin_logger.setLevel(root.level)

    logger = logging.getLogger('kylin')
    logger.info('kylin logging setup complete with level %s', log_type)
    
    for logger_name in [
        'WDM', 'httpx', 'selenium', 'playwright', 'urllib3',
        'asyncio', 'langchain', 'openai', 'httpcore',
        'charset_normalizer', 'anthropic._base_client',
        'PIL.PngImagePlugin',
    ]:
        third_party = logging.getLogger(logger_name)
        third_party.setLevel(logging.ERROR)
        third_party.propagate = False


def addLoggingLevel(levelName, levelNum, methodName=None):
    """动态添加日志级别"""
    if not methodName:
        methodName = levelName.lower()

    if hasattr(logging, levelName):
        raise AttributeError('{} already defined in logging module'.format(levelName))
    if hasattr(logging, methodName):
        raise AttributeError('{} already defined in logging module'.format(methodName))
    if hasattr(logging.getLoggerClass(), methodName):
        raise AttributeError('{} already defined in logger class'.format(methodName))

    def logForLevel(self, message, *args, **kwargs):
        if self.isEnabledFor(levelNum):
            self._log(levelNum, message, args, **kwargs)

    def logToRoot(message, *args, **kwargs):
        logging.log(levelNum, message, *args, **kwargs)

    logging.addLevelName(levelNum, levelName)
    setattr(logging, levelName, levelNum)
    setattr(logging.getLoggerClass(), methodName, logForLevel)
    setattr(logging, methodName, logToRoot)
