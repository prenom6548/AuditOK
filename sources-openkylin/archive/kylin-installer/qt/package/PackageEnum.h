/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PACKAGEENUMS_H
#define PACKAGEENUMS_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QMetaEnum>
#include <QString>
#include <string>

class PackageEnum : public QObject
{
    Q_OBJECT

public:

    enum class Type {
        DEB,
        SNAP,
        APK,
        KARE,
        KARE_ENV,
        OTHERS
    };
    Q_ENUM(Type)

    enum class Status {
        INSTALLED,
        UNINSTALLED,
        UPGRADABLE,
        OTHERS,
        DAMAGED

    };
    Q_ENUM(Status)

    template<typename T>
    static QString enumToString(T value) {
        return QString(QMetaEnum::fromType<T>().valueToKey(static_cast<int>(value)));
    }

    template<typename T>
    static std::string enumToStdString(T value) {
        return QString(QMetaEnum::fromType<T>().valueToKey(static_cast<int>(value))).toStdString();
    }

};

#endif // PACKAGEENUMS_H
