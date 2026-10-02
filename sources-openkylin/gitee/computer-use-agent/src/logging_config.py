"""
简化的日志配置
主要变更：
1. 移除冗余的日志处理器设置
2. 统一使用新的 SessionLogger 进行会话日志管理
3. 保留基础的 logging setup 用于第三方库
"""
import logging
import os
import sys
from pathlib import Path
from typing import Optional

from src.logger import get_session_logger, init_session_logger

__all__ = ['setup_logging', 'get_session_logger', 'init_session_logger']


def addLoggingLevel(levelName, levelNum, methodName=None):
    """动态添加日志级别（保留用于兼容）"""
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


def setup_logging(log_file: Optional[str] = None, level: str = "INFO"):
    """
    简化的日志设置
    
    Args:
        log_file: 日志文件路径（可选），不指定则只输出到控制台
        level: 日志级别 (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    """
    try:
        addLoggingLevel('RESULT', 35)
    except AttributeError:
        pass

    log_type = os.getenv('kylin_LOGGING_LEVEL', level).lower()
    
    # 映射日志级别
    level_map = {
        'result': 'RESULT',
        'debug': logging.DEBUG,
        'info': logging.INFO,
        'warning': logging.WARNING,
        'error': logging.ERROR,
        'critical': logging.CRITICAL,
    }
    log_level = level_map.get(log_type, logging.INFO)

    # Always reconfigure. src.__init__ installs an early console logger before
    # run/kylin-cua.py knows the per-session log file, so returning here would
    # silently drop file logging and hide later diagnostics.
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

    # 控制台处理器
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
        console.setLevel(log_level)

    root.addHandler(console)

    file_handler = None
    if log_file:
        log_path = Path(log_file).expanduser().resolve()
        log_path.parent.mkdir(parents=True, exist_ok=True)
        file_handler = logging.FileHandler(log_path, encoding="utf-8", mode="w")
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(
            logging.Formatter(
                "%(asctime)s | %(levelname)-8s | [%(name)s] %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S",
            )
        )
        root.addHandler(file_handler)

    # 设置日志级别
    if log_type == 'result':
        root.setLevel('RESULT')
    else:
        root.setLevel(log_level)

    kylin_logger = logging.getLogger('kylin')
    for handler in list(kylin_logger.handlers):
        kylin_logger.removeHandler(handler)
        try:
            handler.close()
        except Exception:
            pass
    kylin_logger.propagate = True
    kylin_logger.setLevel(root.level)

    logger = logging.getLogger('kylin')
    logger.info('kylin logging setup complete with level %s', log_type)
    
    # 静默第三方库日志
    for logger_name in [
        'WDM',
        'httpx',
        'selenium',
        'playwright',
        'urllib3',
        'asyncio',
        'langchain',
        'openai',
        'httpcore',
        'charset_normalizer',
        'anthropic._base_client',
        'PIL.PngImagePlugin',
    ]:
        third_party = logging.getLogger(logger_name)
        third_party.setLevel(logging.ERROR)
        third_party.propagate = False

    # 初始化新的 SessionLogger（如果指定了日志文件）
    if log_file:
        init_session_logger(
            log_file=log_file,
            level=log_type if log_type != 'result' else 'INFO',
            console_output=False,
        )
        logger.info('Session logger initialized with log file: %s', log_file)
