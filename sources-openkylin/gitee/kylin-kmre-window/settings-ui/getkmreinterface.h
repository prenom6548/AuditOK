/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
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

#include <QString>
#include <QFile>
#include <QDebug>

#include <dlfcn.h>


class GetKmreInterface
{
public:
    GetKmreInterface(QString name) {
        mName = name;
    }

    ~GetKmreInterface() {
        if (mModuleHandle) {
            dlclose(mModuleHandle);
            mModuleHandle = nullptr;
        }
    }

    void* operator() () {
        do {
            if (!QFile::exists(mLibPath)) {
                break;
            }

            mModuleHandle = dlopen(mLibPath.toStdString().c_str(), RTLD_LAZY);
            if (!mModuleHandle) {
                break;
            }

            void* func = dlsym(mModuleHandle, mName.toStdString().c_str());
            if (dlerror() != nullptr) {
                break;
            }

            return func;
        }while(0);
        qDebug() << "Get function '" << mName <<"' failed from library!";
        return nullptr;
    }

private:
    QString mName;
    void *mModuleHandle = nullptr;
    const QString mLibPath = "/usr/lib/libkmre.so";
};