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

#ifndef GRPC_DOCUMENT_SERVICE_STUB_H
#define GRPC_DOCUMENT_SERVICE_STUB_H

#include "idocument_service_stub.h"
#include <memory>
#include <grpcpp/grpcpp.h>
#include "document.grpc.pb.h"

namespace document {

class GrpcDocumentServiceStub : public IDocumentServiceStub {
public:
    explicit GrpcDocumentServiceStub(std::shared_ptr<grpc::Channel> channel)
        : stub_(::document::DocumentService::NewStub(channel)) {}

    ::grpc::Status ChunkDocument(
        ::grpc::ClientContext* context,
        const ::document::ChunkRequest& request,
        ::document::ChunkedResponse* response) override {
        return stub_->SyncChunkDocument(context, request, response);
    }

    std::unique_ptr<grpc::ClientReader<::document::ChunkedResponse>> AsyncChunkDocument(
        ::grpc::ClientContext* context,
        const ::document::ChunkRequest& request) override {
        return stub_->AsyncChunkDocument(context, request);
    }

private:
    std::unique_ptr<::document::DocumentService::Stub> stub_;
};

}  // namespace document

#endif  // GRPC_DOCUMENT_SERVICE_STUB_H