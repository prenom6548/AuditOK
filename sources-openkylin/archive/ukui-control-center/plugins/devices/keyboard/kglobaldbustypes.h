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

#ifndef KGLOBALDBUSTYPES_H
#define KGLOBALDBUSTYPES_H

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QString>

void registerShortcutDBusTypes();

#define IM_CONFIGTOOL_KGLOBALACCEL_BEGIN_DECLARE_DBUS_TYPE(TYPE)               \
    class TYPE {                                                               \
        Q_GADGET

#define IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(TYPE, GETTER, SETTER)      \
    Q_PROPERTY(TYPE GETTER READ GETTER WRITE SETTER)

#define IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(TYPE, GETTER, SETTER)         \
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

#define IM_CONFIGTOOL_KGLOBALACCEL_END_DECLARE_DBUS_TYPE(TYPE)                 \
    }                                                                          \
    ;                                                                          \
    typedef QList<TYPE> TYPE##List;                                            \
    QDBusArgument &operator<<(QDBusArgument &argument, const TYPE &value);     \
    const QDBusArgument &operator>>(const QDBusArgument &argument, TYPE &value);

IM_CONFIGTOOL_KGLOBALACCEL_BEGIN_DECLARE_DBUS_TYPE(KGlobalAccelShortcutInfo);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, uniqueName, setUniqueName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, friendlyName,
                                            setFriendlyName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, componentUniqueName,
                                            setComponentUniqueName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, componentFriendlyName,
                                            setComponentFriendlyName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, description,
                                            setDescription);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QString, shortcutKey,
                                            setShortcutKey);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QList<int>, defaultKeys,
                                            setDefaultKeys);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_PROPERTY(QList<int>, currentKeys,
                                            setCurrentKeys);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, uniqueName, setUniqueName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, friendlyName,
                                         setFriendlyName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, componentUniqueName,
                                         setComponentUniqueName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, componentFriendlyName,
                                         setComponentFriendlyName);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, description, setDescription);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QString, shortcutKey, setShortcutKey);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QList<int>, defaultKeys,
                                         setDefaultKeys);
IM_CONFIGTOOL_KGLOBALACCEL_DECLARE_FIELD(QList<int>, currentKeys,
                                         setCurrentKeys);
IM_CONFIGTOOL_KGLOBALACCEL_END_DECLARE_DBUS_TYPE(KGlobalAccelShortcutInfo);

Q_DECLARE_METATYPE(KGlobalAccelShortcutInfo)
Q_DECLARE_METATYPE(KGlobalAccelShortcutInfoList)

#endif // KGLOBALDBUSTYPES_H
