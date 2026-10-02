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

#include <QDBusMetaType>

#include "customdata.h"

namespace KylinRubbishClear
{

/*QDBusArgument &operator<<(QDBusArgument &argument, const CustomData &data) {
    argument.beginStructure();
    argument << data.hash << data.name << data.description << data.index << data.valid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CustomData &data) {
    argument.beginStructure();
    argument >> data.hash >> data.name >> data.description >> data.index >> data.valid;
    argument.endStructure();
    return argument;
}

void CustomData::registerCustomDataMetaType()
{
    qRegisterMetaType<CustomData>("CustomData");
    qDBusRegisterMetaType<CustomData>();
}*/

void registerCustomDataMetaType()
{
    qRegisterMetaType<CustomData>("CustomData");
    qDBusRegisterMetaType<CustomData>();
}
} // namespace KylinRubbishClear
