/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "kglobaldbustypes.h"
#include <QDBusMetaType>

#define IM_CONFIGTOOL_KGLOBALACCEL_DEFINE_DBUS_TYPE(TYPE)                      \
    qRegisterMetaType<TYPE>(#TYPE);                                            \
    qDBusRegisterMetaType<TYPE>();                                             \
    qRegisterMetaType<TYPE##List>(#TYPE "List");                               \
    qDBusRegisterMetaType<TYPE##List>();

void registerShortcutDBusTypes() {
    IM_CONFIGTOOL_KGLOBALACCEL_DEFINE_DBUS_TYPE(KGlobalAccelShortcutInfo);
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const KGlobalAccelShortcutInfo &info) {
    argument.beginStructure();
    argument << info.uniqueName();
    argument << info.friendlyName();
    argument << info.componentUniqueName();
    argument << info.componentFriendlyName();
    argument << info.description();
    argument << info.shortcutKey();
    argument << info.defaultKeys();
    argument << info.currentKeys();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                KGlobalAccelShortcutInfo &info) {
    QString uniqueName, friendlyName, componentUniqueName,
        componentFriendlyName, shortcutKey, description;
    QList<int> defaultKeys, currentKeys;

    argument.beginStructure();
    argument >> uniqueName >> friendlyName >> componentUniqueName >>
        componentFriendlyName >> description >> shortcutKey >> defaultKeys >>
        currentKeys;
    argument.endStructure();

    info.setUniqueName(uniqueName);
    info.setFriendlyName(friendlyName);
    info.setComponentUniqueName(componentUniqueName);
    info.setComponentFriendlyName(componentFriendlyName);
    info.setShortcutKey(shortcutKey);
    info.setDescription(description);
    info.setDefaultKeys(defaultKeys);
    info.setCurrentKeys(currentKeys);

    return argument;
}
