/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "knowledgebaseproxy.h"
#include "kylin-ai/business/knowledge-base/knowledgebase.h"

namespace knowledgebase {

struct KnowledgebaseSession::KnowledgebaseImlp {
    KnowledgebaseProxy* proxy;

    KnowledgebaseImlp() : proxy(new KnowledgebaseProxy()) {}
    ~KnowledgebaseImlp() { delete proxy; }
};

KnowledgebaseSession::KnowledgebaseSession() : impl_(std::make_unique<KnowledgebaseImlp>()) {}
KnowledgebaseSession::~KnowledgebaseSession() = default;
Status KnowledgebaseSession::isSupported(bool& success) const { return impl_->proxy->isSupported(success); }

Status KnowledgebaseSession::createKnowledgebase(std::string& knowledgebaseName) {
    return impl_->proxy->createKnowledgebase(knowledgebaseName);
}
Status KnowledgebaseSession::deleteKnowledgebase(const std::string& knowledgebaseName) {
    return impl_->proxy->deleteKnowledgebase(knowledgebaseName);
}

Status KnowledgebaseSession::similaritySearch(const std::string& searchCondition, std::string& result) {
    return impl_->proxy->similaritySearch(searchCondition, result);
}

Status KnowledgebaseSession::addTextFiles(const std::string& input) { return impl_->proxy->addTextFiles(input); }

Status KnowledgebaseSession::getTextFileContents(const std::string& input, std::string& result) {
    return impl_->proxy->getTextFileContents(input, result);
}
}  // namespace knowledgebase