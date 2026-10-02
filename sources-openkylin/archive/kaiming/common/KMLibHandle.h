/*
 * @Author: caoxiansheng caoxiansheng@kylinos.cn
 * @Date: 2025-04-17 17:58:20
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-04-18 09:13:51
 * @FilePath: /kaiming/common/KMLibHandle.h
 * @Description: 
 * 
 * Copyright (c) by KylinSoft  Co., Ltd. 2025. All Rights Reserved.
 */

#ifndef _KM_LIBHANDLE_H
#define _KM_LIBHANDLE_H

#include <dlfcn.h>
#include "common/KMError.h"
#include "common/KMLogger.h"

class KMLibHandle {
    void* handle;
public:
    explicit KMLibHandle(const char* libPath) : handle(dlopen(libPath, RTLD_LAZY)) {
         if (!handle) {
            const char* err = dlerror();
            kmlogger.error("Failed to load library %s: %s", libPath, err ? err : "Unknown error");
            throw std::runtime_error(std::string("Failed to load library '") + libPath + "': " + (err ? err : "Unknown error"));
        }
    }
    ~KMLibHandle() { 
        if (handle) dlclose(handle); 
    }
    // 删除拷贝构造和赋值
    KMLibHandle(const KMLibHandle&) = delete;
    KMLibHandle& operator=(const KMLibHandle&) = delete;
 
    KMLibHandle(KMLibHandle&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    KMLibHandle& operator=(KMLibHandle&& other) noexcept {
        if (this != &other) {
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
    
    template<typename T>
    T loadSymbol(const char* name) {
        dlerror();
        auto sym = reinterpret_cast<T>(dlsym(handle, name));
        if (const char* err = dlerror()) {
            kmlogger.error("Cannot load symbol %s: %s", name, err);
            throw std::runtime_error(std::string("Cannot load symbol '") + name + "': " + err);
        }
        return sym;
    }

    explicit operator bool() const { return handle != nullptr; }
};

#endif // _KM_LIBHANDLE_H