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

#include <mutex>

namespace kmre {

template<class T>
class Singleton 
{
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator = (const Singleton&) = delete;

    virtual ~Singleton() {}

protected:
    Singleton(){}

private:
    
};

template<class T>
class SingletonP
{
public:
    static T* getInstance() {
        std::lock_guard<std::mutex> lk(mtx);
        if (pInstance == nullptr) {
            pInstance = new T();
        }
        return pInstance;
    }

    static void destroy() {
        std::lock_guard<std::mutex> lk(mtx);
        if (pInstance) {
            delete pInstance;
            pInstance = nullptr;
        }
    }

    SingletonP(const SingletonP&) = delete;
    SingletonP& operator = (const SingletonP&) = delete;

protected:
    SingletonP(){}
    virtual ~SingletonP() {}

private:
    static T* pInstance;
    static std::mutex mtx;
};

template<class T> 
T* SingletonP<T>::pInstance = nullptr;
template<class T>
std::mutex SingletonP<T>::mtx;

}