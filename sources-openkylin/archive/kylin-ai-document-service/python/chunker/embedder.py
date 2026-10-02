# /*
#  * Copyright 2024 KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify it under
#  * the terms of the GNU General Public License as published by the Free Software
#  * Foundation, either version 3 of the License, or (at your option) any later
#  * version.
#  *
#  * This program is distributed in the hope that it will be useful, but WITHOUT
#  * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
#  * details.
#  *
#  * You should have received a copy of the GNU General Public License along with
#  * this program. If not, see <https://www.gnu.org/licenses/>.
#  */

import ctypes
import platform
import threading
from log_config import get_logger
logger = get_logger()

class EmbeddingResult(ctypes.Structure):
    pass

EmbeddingResultPtr = ctypes.POINTER(EmbeddingResult)
EmbeddingResultPtrPtr = ctypes.POINTER(EmbeddingResultPtr)

class TextEmbeddingSession(ctypes.Structure):
    pass

TextEmbeddingSessionPtr = ctypes.POINTER(TextEmbeddingSession)
TextEmbeddingSessionPtrPtr = ctypes.POINTER(TextEmbeddingSessionPtr)

class EmbeddingModelInfo(ctypes.Structure):
    pass

EmbeddingModelInfoPtr = ctypes.POINTER(EmbeddingModelInfo)

class EmbeddingModelList(ctypes.Structure):
    pass

EmbeddingModelListPtr = ctypes.POINTER(EmbeddingModelList)

# 加载动态库
arch = platform.machine()
try:
    lib = ctypes.CDLL(f"/usr/lib/{arch}-linux-gnu/libkysdk-coreai-embedding.so.1")
except OSError as e:
    logger.error(f"加载向量化SDK动态库失败: {e}")
    raise

# 设置函数原型
# float* embedding_result_get_vector_data(EmbeddingResult* result);
lib.embedding_result_get_vector_data.argtypes = [EmbeddingResultPtr]
lib.embedding_result_get_vector_data.restype = ctypes.POINTER(ctypes.c_float)

# int embedding_result_get_vector_length(EmbeddingResult* result);
lib.embedding_result_get_vector_length.argtypes = [EmbeddingResultPtr]
lib.embedding_result_get_vector_length.restype = ctypes.c_int

# int embedding_result_get_error_code(EmbeddingResult* result);
lib.embedding_result_get_error_code.argtypes = [EmbeddingResultPtr]
lib.embedding_result_get_error_code.restype = ctypes.c_int

# const char* embedding_result_get_error_message(EmbeddingResult* result);
lib.embedding_result_get_error_message.argtypes = [EmbeddingResultPtr]
lib.embedding_result_get_error_message.restype = ctypes.c_char_p

# void embedding_result_destroy(EmbeddingResult** result);
lib.embedding_result_destroy.argtypes = [EmbeddingResultPtrPtr]
lib.embedding_result_destroy.restype = None

# TextEmbeddingSession* text_embedding_create_session();
lib.text_embedding_create_session.argtypes = []
lib.text_embedding_create_session.restype = TextEmbeddingSessionPtr

# void text_embedding_destroy_session(TextEmbeddingSession** session);
lib.text_embedding_destroy_session.argtypes = [TextEmbeddingSessionPtrPtr]
lib.text_embedding_destroy_session.restype = None

# int text_embedding_init_session(TextEmbeddingSession* session);
lib.text_embedding_init_session.argtypes = [TextEmbeddingSessionPtr]
lib.text_embedding_init_session.restype = ctypes.c_int

# bool text_embedding(TextEmbeddingSession* session, const char* text, EmbeddingResult** result);
lib.text_embedding.argtypes = [TextEmbeddingSessionPtr, ctypes.c_char_p, EmbeddingResultPtrPtr]
lib.text_embedding.restype = ctypes.c_bool

# int text_embedding_init_model(TextEmbeddingSession* session, const char* model_name);
lib.text_embedding_init_model.argtypes = [TextEmbeddingSessionPtr, ctypes.c_char_p]
lib.text_embedding_init_model.restype = ctypes.c_int

# EmbeddingModelList* text_embedding_get_model_list(TextEmbeddingSession* session, int* error_code);
lib.text_embedding_get_model_list.argtypes = [TextEmbeddingSessionPtr, ctypes.POINTER(ctypes.c_int)]
lib.text_embedding_get_model_list.restype = EmbeddingModelListPtr

# int embedding_model_list_get_count(const EmbeddingModelList* model_list, int* error_code);
lib.embedding_model_list_get_count.argtypes = [EmbeddingModelListPtr, ctypes.POINTER(ctypes.c_int)]
lib.embedding_model_list_get_count.restype = ctypes.c_int

# const EmbeddingModelInfo* embedding_model_list_get_model(const EmbeddingModelList* model_list, int index, int* error_code);
lib.embedding_model_list_get_model.argtypes = [EmbeddingModelListPtr, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]
lib.embedding_model_list_get_model.restype = EmbeddingModelInfoPtr

# const char* embedding_model_info_get_model_name(const EmbeddingModelInfo* model_info, int* error_code);
lib.embedding_model_info_get_model_name.argtypes = [EmbeddingModelInfoPtr, ctypes.POINTER(ctypes.c_int)]
lib.embedding_model_info_get_model_name.restype = ctypes.c_char_p


class EmbeddingProxy:
    """单例类"""

    def __init__(self):
        """初始化"""
        self.session = None
        self.lock = threading.Lock()
        self.isInit = False

        # 获取连接 并加载模型
        self.isInit = self.__init_resources()

    def __del__(self):
        self.__destroy_session()
        self.isInit = False

    def __init_session(self):
        """创建嵌入会话"""
        self.session = lib.text_embedding_create_session()
        if not self.session:
            logger.error("Create embedding session failed")
            return False

        result = lib.text_embedding_init_session(self.session)
        if result != 0:
            logger.error(f"Embedding init failed with error code: {result}")
            # 初始化失败需清理内存
            lib.text_embedding_destroy_session(ctypes.byref(self.session))
            self.session = None
            return False

        logger.info("Embedding session init success")
        return True

    def __load_model(self):
        """加载向量化模型 使用默认模型即可"""
        if not self.session:
            logger.error("Session not initialized, cannot load model")
            return False

        # 获取模型列表
        error_code = ctypes.c_int()
        model_list = lib.text_embedding_get_model_list(self.session, ctypes.byref(error_code))
        if error_code.value != 0 or not model_list:
            logger.error(f"Get model list failed with error code: {error_code.value}")
            return False

        # 获取模型数量
        count_error_code = ctypes.c_int()
        model_count = lib.embedding_model_list_get_count(model_list, ctypes.byref(count_error_code))
        if count_error_code.value != 0 or model_count <= 0:
            logger.error(f"Get model count failed or no models available, error code: {count_error_code.value}, count: {model_count}")
            return False

        # 获取索引0的模型信息
        model_info_error_code = ctypes.c_int()
        model_info = lib.embedding_model_list_get_model(model_list, 0, ctypes.byref(model_info_error_code))
        if model_info_error_code.value != 0 or not model_info:
            logger.error(f"Get model info failed with error code: {model_info_error_code.value}")
            return False

        # 获取模型名称
        name_error_code = ctypes.c_int()
        model_name_ptr = lib.embedding_model_info_get_model_name(model_info, ctypes.byref(name_error_code))
        if name_error_code.value != 0 or not model_name_ptr:
            logger.error(f"Get model name failed with error code: {name_error_code.value}")
            return False

        model_name = model_name_ptr.decode('utf-8')
        logger.info(f"Selected embedding model: {model_name}")

        # 加载模型
        init_model_result = lib.text_embedding_init_model(self.session, model_name.encode('utf-8'))
        if init_model_result != 0:
            logger.error(f"Init embedding model failed with error code: {init_model_result}")
            return False

        logger.info("Embedding model loaded successfully")
        return True

    def __init_resources(self):
        """初始化所有资源（session和模型）"""
        # 初始化session
        if not self.__init_session():
            return False

        # 加载模型
        if not self.__load_model():
            # 模型信息的相关资源由session管理，加载模型失败，清理session
            lib.text_embedding_destroy_session(ctypes.byref(self.session))
            self.session = None
            return False

        logger.info("Embedding proxy init success")
        return True

    def embed_text(self, text):
        """执行文本嵌入"""
        with self.lock:
            if not self.isInit:
                logger.error("Embedding proxy not initialized")
                self.isInit = self.__init_resources()
                if not self.isInit:
                    return None

            result_ptr = EmbeddingResultPtr()
            success = lib.text_embedding(
                self.session,
                text.encode('utf-8'),
                ctypes.byref(result_ptr)
            )

            if not result_ptr:
                logger.error("Embedding call failed, result_ptr is None")
                return None

            err_code, err_msg = self.__get_error_info(result_ptr)
            if err_code != 0:
                logger.error(f"Embedding call failed, error:{err_code}, msg:{err_msg}")
                lib.embedding_result_destroy(ctypes.byref(result_ptr))
                return None

            res = self.__get_vector_data(result_ptr)
            lib.embedding_result_destroy(ctypes.byref(result_ptr))
            return res


    def __get_vector_data(self, result_ptr):
        """从结果中获取向量数据"""
        data_ptr = lib.embedding_result_get_vector_data(result_ptr)
        length = lib.embedding_result_get_vector_length(result_ptr)

        if not data_ptr or length <= 0:
            return None

        # 将C数组转换为Python列表
        # 这里会发生内存拷贝，生成纯 Python list，所以返回后是安全的
        return [data_ptr[i] for i in range(length)]

    def __get_error_info(self, result_ptr):
        """获取错误信息"""
        code = lib.embedding_result_get_error_code(result_ptr)
        msg_ptr = lib.embedding_result_get_error_message(result_ptr)
        msg = msg_ptr.decode('utf-8') if msg_ptr else ""
        return code, msg

    def __destroy_session(self):
        """销毁会话"""
        if self.session:
            lib.text_embedding_destroy_session(ctypes.byref(self.session))
            self.session = None

# python天然单例
embedding_instance = EmbeddingProxy()

if __name__ == "__main__":
    import time

    def worker(thread_id, text):
        print(f"Thread-{thread_id} start requesting...")
        vec = embedding_instance.embed_text(text)
        if vec:
            print(f"Thread-{thread_id} finished. Vector dim: {len(vec)}")
        else:
            print(f"Thread-{thread_id} failed.")

    # 模拟并发
    threads = []
    start_t = time.time()
    for i in range(5):
        t = threading.Thread(target=worker, args=(i, f"Hello thread {i}"))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    print(f"Total cost: {time.time() - start_t:.4f}s")