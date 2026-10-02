/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <tuple>
#include <dlfcn.h>
#include <syslog.h>


class LibWrapper
{
public:
    LibWrapper(const char* libPath, int flag = RTLD_LAZY) : mModuleHandle(nullptr) {
        mModuleHandle = dlopen(libPath, flag);
        if (!mModuleHandle) {
            syslog(LOG_ERR, "Load library '%s' failed: %s", libPath, dlerror());
        }
    }

    ~LibWrapper() {
        if (mModuleHandle) {
            dlclose(mModuleHandle);
            mModuleHandle = nullptr;
        }
    }

    std::tuple<bool, void*> getSymbol(const char* symbol) {
        if (mModuleHandle) {
            char* error = nullptr;
            void* sym = dlsym(mModuleHandle, symbol);
            if ((error = dlerror()) == nullptr) {
                return {true, sym};
            }
            syslog(LOG_ERR, "Get symbol '%s' failed from library! error: %s", symbol, error);
        }
        return {false, nullptr};
    }

    LibWrapper(const LibWrapper&) = delete;
    LibWrapper& operator = (const LibWrapper&) = delete;

private:
    void *mModuleHandle;
};