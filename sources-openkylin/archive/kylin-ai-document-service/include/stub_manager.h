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

#ifndef STUB_MANAGER_H
#define STUB_MANAGER_H

#include <grpcpp/grpcpp.h>
#include "document.grpc.pb.h"

namespace document {

class ChunkStubManager {
public:
  // 构造函数
  ChunkStubManager(std::shared_ptr<grpc::Channel> sync_channel,
              std::shared_ptr<grpc::Channel> async_channel);

  // 析构函数
  ~ChunkStubManager();

  // 获取同步存根
  ::document::DocumentService::Stub *GetSyncStub() const;

  // 获取异步存根
  ::document::DocumentService::Stub *GetAsyncStub() const;

private:
  std::unique_ptr<::document::DocumentService::Stub> sync_stub_;
  std::unique_ptr<::document::DocumentService::Stub> async_stub_;
};

} // namespace document

#endif // STUB_MANAGER_H