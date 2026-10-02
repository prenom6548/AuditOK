/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef CUSTOMDATA_H
#define CUSTOMDATA_H

#include <QDBusMetaType>
#include <QDBusArgument>
#include <QString>
#include <QDebug>
namespace KylinRubbishClear
{

class CustomData
{
public:
    QString hash;
    QString name;
    QString description;
    int index;
    bool valid;

    //    friend QDebug operator<<(QDebug argument, const CustomData &data);
    friend QDebug operator<<(QDebug argument, const CustomData &data)
    {
        argument << data.name;
        return argument;
    }

    //    friend QDBusArgument &operator<<(QDBusArgument &argument, const CustomData &data);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CustomData &data)
    {
        argument.beginStructure();
        argument << data.hash << data.name << data.description << data.index << data.valid;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CustomData &data)
    {
        argument.beginStructure();
        argument >> data.hash >> data.name >> data.description >> data.index >> data.valid;
        argument.endStructure();
        return argument;
    }

    bool operator==(const CustomData data) const
    {
        return data.hash == hash;
    }

    bool operator!=(const CustomData data) const
    {
        return data.hash != hash;
    }

    //    static void registerCustomDataMetaType();
};
} // namespace KylinRubbishClear

Q_DECLARE_METATYPE(KylinRubbishClear::CustomData)

void registerCustomDataMetaType();


#endif // CUSTOMDATA_H
