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

#include <client.h>

#include <iostream>

int main() {
    try {
        auto client = std::make_unique<document::DocumentChunkClient>();
        if (!client) {
            return 1;
        }
        std::vector<std::string> file_paths = {
            "/home/kylin/文档/操作系统大会演示素材v0.2/智能数据管理素材/文档素材/公司报销制度及流程.docx"
            // "/home/kylin/文档/操作系统大会演示素材v0.2/智能数据管理素材/文档素材/公司报销制度及流程.docx"
        };
        std::vector<std::string> chunker_types = {"TextChunker"};
        std::vector<int32_t> chunk_sizes = {500};
        void* callback_data = nullptr;

        // 同步请求
        auto sync_response = client->SyncChunkDocument(file_paths, chunker_types, chunk_sizes);
        std::cout << "SyncChunkDocument Response: " << std::endl;
        std::cout << "data: " << sync_response.data << std::endl;
        std::cout << "errorCode: " << sync_response.errorCode << std::endl;
        std::cout << "errorMsg: " << sync_response.errorMsg << std::endl;
    } catch (const std::exception& ex) {
        return 0;
    }
    return 0;
}