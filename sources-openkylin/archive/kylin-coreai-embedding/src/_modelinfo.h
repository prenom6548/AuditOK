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

#pragma once
#include <string>
#include <vector>

#include "modelinfo.h"

class _EmbeddingModelInfo {
public:
    _EmbeddingModelInfo() = default;
    _EmbeddingModelInfo(const std::string &name, int dim, int priority)
        : name_(name), dim_(dim), priority_(priority) {}

    const std::string &getName() const { return name_; }
    int getDim() const { return dim_; }
    int getPriority() const { return priority_; }

private:
    std::string name_{};
    int dim_{};
    int priority_{};
};

class _EmbeddingModelList {
public:
    _EmbeddingModelList() = default;
    _EmbeddingModelList(std::vector<_EmbeddingModelInfo> info) : info_(info) {}

    int size() const { return static_cast<int>(info_.size()); }

    const _EmbeddingModelInfo *getModelInfo(int index) const {
        if (index < 0 || index >= static_cast<int>(info_.size())) {
            return nullptr;
        }
        return &info_[index];
    }

private:
    std::vector<_EmbeddingModelInfo> info_;
};