/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef GEN_AI_NLP_SERVER_H
#define GEN_AI_NLP_SERVER_H

#include <mutex>
#include <string>

#include "gennlpserviceglue.h"

namespace kyai::genai::nlp {

class GenAiNlpServer {
public:
    static GenAiNlpServer &getInstance();

    ~GenAiNlpServer();

    void init();

    [[nodiscard]] inline bool available() const { return connection_ != nullptr; }

    [[nodiscard]] inline GDBusConnection *getConnection() const { return connection_; }

private:
    GenAiNlpServer();
    void init(const std::string &unixPath);
    void destroy() const;

private:
    GDBusConnection *connection_{nullptr};
    mutable std::mutex mutex_;
};

}  // namespace kyai::genai::nlp

#endif  // GEN_AI_NLP_SERVER_H
