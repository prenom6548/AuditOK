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

#ifndef DOCUMENT_CLIENT_H
#define DOCUMENT_CLIENT_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace document {

struct DocumentChunkResponse {
    std::string data;
    void *callbackData;
    int errorCode;
    std::string errorMsg;
};

class ChunkStubManager;
class DocumentChunkClient {
public:
    DocumentChunkClient();
    ~DocumentChunkClient();

    DocumentChunkResponse SyncChunkDocument(const std::vector<std::string> &file_paths,
                                            const std::vector<std::string> &chunker_types,
                                            const std::vector<int32_t> &chunk_sizes);

    using DocumentChunkCallback = std::function<void(const DocumentChunkResponse &response, void *callback_data)>;

    void AsyncChunkDocument(const std::vector<std::string> &file_paths, const std::vector<std::string> &chunker_types,
                            const std::vector<int32_t> &chunk_sizes, void *callback_data,
                            DocumentChunkCallback callback);

    DocumentChunkResponse SyncChunkDocument(const std::vector<std::string> &file_paths,
                                            const std::vector<std::string> &chunker_types,
                                            const std::vector<int32_t> &chunk_sizes,
                                            const std::vector<int32_t> &max_doc_chars_count);

private:
    std::unique_ptr<ChunkStubManager> stub_manager_;
};

}  // namespace document

#endif  // DOCUMENT_CLIENT_H