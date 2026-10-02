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

#ifndef FCITXQTDBUSTYPES_H
#define FCITXQTDBUSTYPES_H

#include <type_traits>
#include <QDBusArgument>
#include <QList>
#include <QMetaType>

void registerKvkbdFcitxQtDBusTypes();

#define KVKBD_FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(TYPE)                          \
    class TYPE {                                                               \
        Q_GADGET

#define KVKBD_FCITX5_QT_DECLARE_PROPERTY(TYPE, GETTER, SETTER)                 \
    Q_PROPERTY(TYPE GETTER READ GETTER WRITE SETTER)

#define KVKBD_FCITX5_QT_DECLARE_FIELD(TYPE, GETTER, SETTER)                    \
public:                                                                        \
    std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE>         \
    GETTER() const {                                                           \
        return GETTER##_;                                                      \
    }                                                                          \
    void SETTER(                                                               \
        std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE>     \
            value) {                                                           \
        GETTER##_ = value;                                                     \
    }                                                                          \
                                                                               \
private:                                                                       \
    TYPE GETTER##_ = TYPE();

#define KVKBD_FCITX5_QT_END_DECLARE_DBUS_TYPE(TYPE)                            \
    }                                                                          \
    ;                                                                          \
    typedef QList<TYPE> TYPE##List;                                            \
    QDBusArgument &operator<<(QDBusArgument &argument, const TYPE &value);     \
    const QDBusArgument &operator>>(const QDBusArgument &argument, TYPE &value);

KVKBD_FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtFullInputMethodEntry);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, uniqueName, setUniqueName);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, nativeName, setNativeName);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, icon, setIcon);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, label, setLabel);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, languageCode, setLanguageCode);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, addon, setAddon);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(bool, configurable, setConfigurable);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QString, layout, setLayout);
KVKBD_FCITX5_QT_DECLARE_PROPERTY(QVariantMap, properties, setProperties);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, name, setName);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, nativeName, setNativeName);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, icon, setIcon);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, label, setLabel);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, languageCode, setLanguageCode);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, addon, setAddon);
KVKBD_FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
KVKBD_FCITX5_QT_DECLARE_FIELD(QString, layout, setLayout);
KVKBD_FCITX5_QT_DECLARE_FIELD(QVariantMap, properties, setProperties);
KVKBD_FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtFullInputMethodEntry);

Q_DECLARE_METATYPE(FcitxQtFullInputMethodEntry)
Q_DECLARE_METATYPE(FcitxQtFullInputMethodEntryList)

#endif // FCITXQTDBUSTYPES_H
