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

import grpc
import json
import document_pb2_grpc
import document_pb2
import time
import os
import logging
import asyncio

# 配置日志文件的位置
log_file_path = "logs/parse_client.log"
# 创建日志目录（如果不存在）
os.makedirs(os.path.dirname(log_file_path), exist_ok=True)
# 配置日志记录器
logging.basicConfig(
    level=logging.INFO,  # 日志级别
    format="%(asctime)s - %(levelname)s - %(message)s",  # 日志格式
    handlers=[
        logging.FileHandler(log_file_path),  # 输出到文件
        logging.StreamHandler()  # 同时输出到控制台
    ]
)
logging.info("Logging is set up. Starting the client...")

def run_sync(client, files):
    # 构造包含多个文件及其解析类型的请求
    request = document_pb2.ParseRequest(files=files)
    # 调用同步解析接口
    response = client.SyncParseDocument(request)
    print(response.results)

async def run_async(client, files):
    request = document_pb2.ParseRequest(files=files)
    logging.info(f"Sending async request for files: {files}")
    # responses = client.AsyncParseDocument(request)
    async for response in client.AsyncParseDocument(request):
        print(f"Async response: {response}")
        logging.info(f"Received async response: {response}")
        for field in response.DESCRIPTOR.fields:
                value = getattr(response, field.name)
                logging.info(f"  {field.name}: {value}")


def main():
    with grpc.insecure_channel('localhost:50051') as channel_sync, grpc.insecure_channel('localhost:50052') as channel_async:
        sync_client = document_pb2_grpc.DocumentServiceStub(channel_sync)
        async_client = document_pb2_grpc.DocumentServiceStub(channel_async)

        # 示例：文件路径与解析类型的映射
        files = {
            '/home/mayuan/codeyard/DocumentService/pdftest.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/embedding.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/data/金融知识库.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/data/麒麟软件工会经费报销单.docx': 'TextChunker'
        }

        # 运行同步请求
        run_sync(async_client, files)
async def main():
    # 初始化 gRPC 异步通道
    channel = grpc.aio.insecure_channel('localhost:50052')
    # 检查通道是否准备好
    try:
        await channel.channel_ready()
        logging.info("Channel is ready")
    except grpc.FutureTimeoutError:
        logging.error("Channel is not ready. Check if the server is running and accessible.")
        return
    stub = document_pb2_grpc.DocumentServiceStub(channel)
    # 文件列表
    files = {
            '/home/mayuan/codeyard/DocumentService/pdftest.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/embedding.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/data/金融知识库.pdf': 'TextChunker',
            '/home/mayuan/codeyard/DocumentService/data/麒麟软件工会经费报销单.docx': 'TextChunker'
        }

    # 调用异步函数
    await run_async(stub, files)

if __name__ == '__main__':
    time_start = time.time()
    # main()
    asyncio.run(main())
    time_end = time.time()
    print(time_end - time_start)
