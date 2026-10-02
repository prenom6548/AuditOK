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

from concurrent import futures
import grpc
import document_pb2_grpc
import document_pb2
from async_cache import AsyncCache
import json
import os
import multiprocessing
from log_config import get_logger
logger = get_logger()

def get_real_file_type(file_path) -> str:
    """
    通过MIME类型，获取文件的扩展名并得到真实类型
    可以识别出加密mimetype时返回encrypted，不支持的文件格式返回None
    """
    import magic
    import os

    common_mime_map = {
        'application/pdf': 'pdf',
        'application/vnd.openxmlformats-officedocument.wordprocessingml.document': 'docx',
        'application/vnd.openxmlformats-officedocument.presentationml.presentation': 'pptx',
        'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet': 'xlsx',
        'application/json': 'json',
        'text/html': 'html',
        'text/xml': 'xml',
        'application/zip': 'ofd',
        'application/ofd': 'ofd',
        'application/vnd.ms-powerpoint': 'ppt'
    }

    multi_type_mime_map = {
        'text/plain': ['md', 'txt'],
        'application/msword': ['doc', 'wps']
    }

    file_mime = magic.from_file(file_path, mime=True)
    logger.info(f"get_real_file_type: {file_mime}")

    if file_mime == 'inode/x-empty':  # 空文本文件统一返回txt格式的parser
        return 'txt'
    elif file_mime == 'application/encrypted':
        return "encrypted"

    elif file_mime in common_mime_map.keys():  # MIME类型与常见的扩展名能直接对应，返回常见扩展名
        return common_mime_map[file_mime]
    # MIME类型有多个对应，用扩展名确定，如果能够正确对应，返回扩展名类型，否则返回更通用的parser类型
    elif file_mime in multi_type_mime_map.keys():
        file_type_list = multi_type_mime_map[file_mime]
        extension = os.path.splitext(file_path)[1][1:]
        return extension if extension in file_type_list else file_type_list[0]
    else:  # 如果文件的MIME类型不存在映射，认为是不支持的文件格式
        return None


class DocumentService(document_pb2_grpc.DocumentServiceServicer):
    def __init__(self, max_workers=4):
        self.max_workers = max_workers
        self.async_cache = AsyncCache()

    @staticmethod
    def parse_and_chunk_file(file_path, chunker_type, chunk_size,max_doc_chars_count,callbake_data):
        from error_code import ErrorCode
        # 检查文件是否存在
        if not os.path.exists(file_path):
            logger.error(f"File not found: {file_path}")
            return (file_path, [],callbake_data,ErrorCode.FILE_NOT_FOUND,f"File not found: {file_path}")

        # 检查文件是否可读
        if not os.access(file_path, os.R_OK):
            logger.error(f"File not readable: {file_path}")
            return (file_path, [],callbake_data,ErrorCode.FILE_PARSER_ERROR,f"File not readable: {file_path}")

        from parser_factory import ParserFactory
        from chunker_factory import ChunkerFactory
        """文件解析和分块逻辑，供进程池执行。"""
        # 获取文件的真实类型 运行到这里说明文件存在且可读，可以获取文件类型
        file_type = get_real_file_type(file_path)
        if file_type == "encrypted":
            logger.error(f"File:{file_path} is encrypted")
            return (file_path, [],callbake_data,ErrorCode.FILE_PARSER_ERROR,f"File is encrypted,file_path:{file_path}")
        elif file_type is None:
            logger.error(f"File:{file_path} is not supported")
            return (file_path, [],callbake_data,ErrorCode.FILE_FORMAT_UNSUPPORTED,f"File format is not supported,file_path:{file_path}")

        parser = ParserFactory.create_parser(file_type)
        try:
            # 各个解析库并不可以区分是什么原因导致抛出的异常，这里统一认为是解析失败
            parsed_content = parser.parser(file_path)
        except Exception as e:
            logger.error(f"File:{file_path} parser failed,exception is:{str(e)}")
            return (file_path, [],callbake_data,ErrorCode.FILE_PARSER_ERROR,str(e))
        if parsed_content is None:
            logger.error(f"File:{file_path} parser content is None")
            return (file_path, [],callbake_data,ErrorCode.FILE_PARSER_ERROR,f"File parser failed,file_path:{file_path}")

        if max_doc_chars_count != -1  and len(parsed_content) > max_doc_chars_count:
            logger.error(f"File:{file_path} char count is too long,chars_count:{len(parsed_content)},max_doc_chars_count:{max_doc_chars_count}")
            return (file_path, [],callbake_data,ErrorCode.FILE_CHAR_COUNT_TOO_LONG,f"File:{file_path} chars count is too long,chunking is skipped")

        if chunker_type == "SemanticChunker":
            # 认为支持段落识别的格式，才可以进行语义切分
            if not parser.is_paragraph_recognition_supported(file_type):
                logger.warning(f"File:{file_path} is not supported semantic chunking,auto checkout to text chunker")
                chunker_type = "TextChunker"

        chunker = ChunkerFactory.create_chunker(chunker_type)
        chunks = chunker.segment_tokenize(parsed_content, chunk_size,file_path)
        return (file_path, chunks,callbake_data,ErrorCode.SUCCESS,"Success")


    def SyncChunkDocument(self, request, context):
        """同步文档分块请求，使用进程池并行处理多个文件。"""
        logger.info(f"Received sync request for files: {request.file_paths}")
        tasks = [(file_path, request.chunker_types[i], request.chunk_sizes[i],request.max_doc_chars_count[i],None) for i, file_path in enumerate(request.file_paths)]
        try:
            results = self.run_in_process_pool(self.parse_and_chunk_file, tasks, self.max_workers)
        except grpc.RpcError as rpc_error:
            context.set_code(grpc.StatusCode.NOT_FOUND)
            context.set_details(f"{str(rpc_error)}")
            return document_pb2.ChunkedResponse()

        response = document_pb2.ChunkedResponse(
            response_data=[]
        )
        for file_path, chunk, _,code,message in results:
            response.response_data.append(document_pb2.ResponseData(chunks = chunk, file_path = file_path,status=document_pb2.Status(error_code=code, error_message=message)))
        logger.debug(f"Returning sync response: {response}")
        logger.info(f"chunk_over: {request.file_paths}")
        return response

    def AsyncChunkDocument(self, request, context):
        """异步文档分块请求，使用进程池并行处理多个文件。"""
        logger.info(f"Received async request for files: {request.file_paths}")
        logger.info(f"Request file paths: {request.file_paths}")
        logger.info(f"Request chunker types: {list(request.chunker_types)}")
        logger.info(f"Request chunk sizes: {list(request.chunk_sizes)}")
        logger.info(f"Request callback data: {request.callback_data}")
        tasks = [
            (file_path, request.chunker_types[i], request.chunk_sizes[i],request.callback_data[0])
            for i, file_path in enumerate(request.file_paths)
        ]
        logger.info(f"Prepared tasks: {tasks}")

        def background_process(tasks, context):
            all_chunks = []
            try:
                with multiprocessing.Pool(processes=self.max_workers) as pool:
                    async_results = [pool.apply_async(self.parse_and_chunk_file, task) for task in tasks]
                    for async_result in async_results:
                        result = async_result.get(timeout=60)
                        (file_path, chunks, callback_data) = result
                        all_chunks.extend(chunks)
            except grpc.RpcError as rpc_error:
                logger.error(f"Error processing task: {rpc_error}")
                all_chunks.append(f"Error: {str(rpc_error)}")
                context.set_code(grpc.StatusCode.NOT_FOUND)
                context.set_details(f"{str(rpc_error)}")
                return document_pb2.ChunkedResponse()

            response = document_pb2.ChunkedResponse(
                    file_paths=request.file_paths,
                    chunks=[str(chunk) for chunk in all_chunks],
                    callback_data=str(request.callback_data[0])
                )
            return response

        def run_background_process():
            response = background_process(tasks, context)
            context.set_code(grpc.StatusCode.OK)
            context.set_details(json.dumps({"status": "completed"}))
            return response
        response_future = self.async_cache.cache_result(run_background_process, key=request.file_paths)
        return response_future.result()

    def run_in_process_pool(self, target, tasks, max_workers):
        with futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
            return list(executor.map(lambda args: target(*args), tasks))

def serve():
    uid = os.getuid()
    home_directory = os.path.expanduser("~")
    logger.info(f"Home directory: {home_directory}")
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10),
                options=[('grpc.max_send_message_length', 1024 * 1024 * 1024),
                        ('grpc.max_receive_message_length', 1024 * 1024 * 1024)])
    document_pb2_grpc.add_DocumentServiceServicer_to_server(DocumentService(), server)
    unix_socket_path_sync = '/tmp/document_service_sync_' + str(uid) +'.sock'
    unix_socket_path_async = '/tmp/document_service_async_' + str(uid) +'.sock'
    try:
        os.unlink(unix_socket_path_sync)
    except OSError:
        if os.path.exists(unix_socket_path_sync):
            raise

    try:
        os.unlink(unix_socket_path_async)
    except OSError:
        if os.path.exists(unix_socket_path_async):
            raise
    server.add_insecure_port(f'unix://{unix_socket_path_sync}')
    server.add_insecure_port(f'unix://{unix_socket_path_async}')
    server.start()
    logger.info("Server started and listening on Unix domain sockets.")
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
