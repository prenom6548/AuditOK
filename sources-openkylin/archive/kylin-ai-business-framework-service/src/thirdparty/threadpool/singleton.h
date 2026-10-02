/*
 * MIT License
 * Copyright (c) 2017-2021 Huu Nguyen
 * Copyright (c) 2022 libcpr and many other contributors

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CPR_SINGLETON_H
#define CPR_SINGLETON_H

#include <mutex>

#ifndef CPR_DISABLE_COPY
#define CPR_DISABLE_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;
#endif

#ifndef CPR_SINGLETON_DECL
#define CPR_SINGLETON_DECL(Class) \
    public: \
        static Class* GetInstance(); \
        static void ExitInstance(); \
    private: \
        CPR_DISABLE_COPY(Class) \
        static Class* s_pInstance; \
        static std::mutex s_mutex;
#endif

#ifndef CPR_SINGLETON_IMPL
#define CPR_SINGLETON_IMPL(Class) \
    Class* Class::s_pInstance = nullptr; \
    std::mutex Class::s_mutex; \
    Class* Class::GetInstance() { \
        if (s_pInstance == nullptr) { \
            s_mutex.lock(); \
            if (s_pInstance == nullptr) { \
                s_pInstance = new Class; \
            } \
            s_mutex.unlock(); \
        } \
        return s_pInstance; \
    } \
    void Class::ExitInstance() { \
        s_mutex.lock(); \
        if (s_pInstance) {  \
            delete s_pInstance; \
            s_pInstance = nullptr; \
        }   \
        s_mutex.unlock(); \
    }
#endif

#endif
