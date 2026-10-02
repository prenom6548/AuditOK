/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef KYSINGLETON_H
#define KYSINGLETON_H

template <typename T>
class KySingleTon
{
public:
    // 创建单例实例
    template <typename... Args>
    static T *instance(Args &&... args)
    {
        if (m_pSingleInstance == nullptr) {
            m_pSingleInstance = new T(std::forward<Args>(args)...);
        }

        return m_pSingleInstance;
    }

    // 获取单例
    static T *getInstance()
    {
        return m_pSingleInstance;
    }

    // 删除单例
    static void destroyInstance()
    {
        delete m_pSingleInstance;
        m_pSingleInstance = nullptr;
    }

private:
    KySingleTon();
    virtual ~KySingleTon();

private:
    static T *m_pSingleInstance;
};

template <class T>
T *KySingleTon<T>::m_pSingleInstance = nullptr;

#endif
