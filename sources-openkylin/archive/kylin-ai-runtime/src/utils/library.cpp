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

#include "library.h"

#include <dlfcn.h>

#include "utils/logger.h"

Library::Library(const std::string &path) : path_(path) {}

bool Library::loaded() const { return handle_; }

bool Library::load() {
    if (path_.empty()) {
        return false;
    }

    handle_ = dlopen(path_.c_str(), RTLD_LAZY);

    if (!handle_) {
        error_ = dlerror();
        LOGE("Load library failed: {} {}", error_, path_);
        return false;
    }

    return true;
}

bool Library::unload() {
    if (!handle_) {
        return false;
    }
    if (dlclose(handle_)) {
        error_ = dlerror();
        LOGE("Unload library failed: {}.", error_);
        return false;
    }

    handle_ = nullptr;
    return true;
}

void *Library::resolve(const char *name) {
    auto *result = dlsym(handle_, name);
    if (!result) {
        LOGE("Resolve library failed: {}.", dlerror());
        error_ = dlerror();
    }

    return result;
}

std::string Library::error() const { return error_; }
