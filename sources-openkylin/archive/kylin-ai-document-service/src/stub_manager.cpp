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

#include "stub_manager.h"

namespace document {

// 构造函数
ChunkStubManager::ChunkStubManager(std::shared_ptr<grpc::Channel> sync_channel,
                                   std::shared_ptr<grpc::Channel> async_channel) {
    // 传入非空的channel 在去创建stub
    if (sync_channel) {
        sync_stub_ = ::document::DocumentService::NewStub(sync_channel);
    }
    if (async_channel) {
        async_stub_ = ::document::DocumentService::NewStub(async_channel);
    }
}

// 析构函数
ChunkStubManager::~ChunkStubManager() {
    // 析构函数中也不需要做额外的事情，因为智能指针会自动管理内存
}

// 获取同步存根
::document::DocumentService::Stub* ChunkStubManager::GetSyncStub() const { return sync_stub_.get(); }

// 获取异步存根
::document::DocumentService::Stub* ChunkStubManager::GetAsyncStub() const { return async_stub_.get(); }

}  // namespace document