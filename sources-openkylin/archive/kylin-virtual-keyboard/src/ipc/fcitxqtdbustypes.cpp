/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
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

#include "fcitxqtdbustypes.h"
#include <QDBusMetaType>

#define KVKBD_FCITX5_QT_DEFINE_DBUS_TYPE(TYPE)                                 \
    qRegisterMetaType<TYPE>(#TYPE);                                            \
    qDBusRegisterMetaType<TYPE>();                                             \
    qRegisterMetaType<TYPE##List>(#TYPE "List");                               \
    qDBusRegisterMetaType<TYPE##List>();

void registerKvkbdFcitxQtDBusTypes() {
    KVKBD_FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtFullInputMethodEntry);
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtFullInputMethodEntry &arg) {
    argument.beginStructure();
    argument << arg.uniqueName();
    argument << arg.name();
    argument << arg.nativeName();
    argument << arg.icon();
    argument << arg.label();
    argument << arg.languageCode();
    argument << arg.addon();
    argument << arg.configurable();
    argument << arg.layout();
    argument << arg.properties();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtFullInputMethodEntry &arg) {
    QString uniqueName, name, nativeName, icon, label, languageCode, addon,
        layout;
    bool configurable;
    QVariantMap properties;
    argument.beginStructure();
    argument >> uniqueName >> name >> nativeName >> icon >> label >>
        languageCode >> addon >> configurable >> layout >> properties;
    argument.endStructure();
    arg.setUniqueName(uniqueName);
    arg.setName(name);
    arg.setNativeName(nativeName);
    arg.setIcon(icon);
    arg.setLabel(label);
    arg.setLanguageCode(languageCode);
    arg.setAddon(addon);
    arg.setConfigurable(configurable);
    arg.setLayout(layout);
    arg.setProperties(properties);
    return argument;
}
