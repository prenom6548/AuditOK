import logging
import threading

_logger_lock = threading.Lock()

def get_logger(name='kylin-ai-document-service'):

    logger = logging.getLogger(name) #全局变量 是线程安全的

    # 双检锁
    if logger.handlers: #读是安全的
        return logger

    with _logger_lock:
        # 添加 handler 不安全 需要加锁
        if not logger.handlers:
            logger.setLevel(logging.INFO)
            formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
            console_handler = logging.StreamHandler()
            console_handler.setLevel(logging.INFO)
            console_handler.setFormatter(formatter)

            logger.addHandler(console_handler)
            logger.info("Logging is set up.")

    return logger


if __name__ == "__main__":
    '''
    测试 logging.getLogger 返回 Logger 对象是一个全局的 print输出是true
    '''
    # 第一次调用
    log1 = get_logger()
    # 第二次调用
    log2 = get_logger()

    print("Same object?", log1 is log2)