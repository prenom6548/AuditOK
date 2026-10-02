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

#ifndef SRC_MODEL_INFO_H
#define SRC_MODEL_INFO_H

#include <jsoncpp/json/json.h>

#include <map>
#include <memory>
#include <string>

#include "logger.h"
#include "modelinfo.h"

struct _TextModelInfo {
    std::string modelName;
    std::string displayName;
    std::string brand;
    ModelDeployType deployType;  // 模型部署方式
    bool supportsWebSearch{false};
    bool supportsThinking{false};
    bool supportsThinkingToggle{false};
    bool supportsToolChoice{false};
};

class _TextModelList {
public:
    _TextModelList() = default;

    explicit _TextModelList(const Json::Value& value);

    int size() const;

    TextModelInfo* getModelInfo(int index) const;

private:
    std::map<int, std::shared_ptr<_TextModelInfo> > modelInfos_;
};

#endif  // SRC_MODEL_INFO_H
