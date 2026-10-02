/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IDOCUMENT_SERVICE_STUB_H
#define IDOCUMENT_SERVICE_STUB_H

#include <grpcpp/grpcpp.h>
#include "document.pb.h"  // 生成的消息类
#include "document.grpc.pb.h"  // 生成的服务存根

namespace document {

// 文档服务接口
class IDocumentService {
public:
    virtual ~IDocumentService() = default;

    // 同步分块请求
    virtual grpc::Status SyncChunkDocument(grpc::ClientContext* context,
                                           const ::document::ChunkRequest& request,
                                           ::document::ChunkedResponse* response) const = 0;

    // 异步分块请求
    virtual std::unique_ptr<grpc::ClientAsyncReader<::document::ChunkedResponse>> PrepareAsyncChunkDocument(
        grpc::ClientContext* context,
        const ::document::ChunkRequest& request,
        grpc::CompletionQueue* cq) const = 0;
};

// 存根类，继承自 gRPC 自动生成的存根类
class DocumentServiceStub : public IDocumentService {
public:
    explicit DocumentServiceStub(std::shared_ptr<grpc::Channel> channel)
        : stub_(::document::DocumentService::NewStub(channel)), channel_(channel) {}

    grpc::Status SyncChunkDocument(grpc::ClientContext* context,
                                   const ::document::ChunkRequest& request,
                                   ::document::ChunkedResponse* response) const override {
        return stub_->SyncChunkDocument(context, request, response);
    }


    // 获取通道
    std::shared_ptr<grpc::Channel> GetChannel() const {
        return channel_;
    }

private:
    std::unique_ptr<::document::DocumentService::Stub> stub_;  // gRPC 存根
    std::shared_ptr<grpc::Channel> channel_;  // 通道
};

}  // namespace document

#endif  // IDOCUMENT_SERVICE_STUB_H