// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef DBUSINTERFACEDATE_H
#define DBUSINTERFACEDATE_H

#include <QtDBus>
namespace WlcomData
{

struct DBusEffectData {
    QString name;
    uint priority;
    bool enable;

    DBusEffectData() : name(""), priority(0), enable(false) {}
};

typedef QList<DBusEffectData> DBusEffectDataList;
} // namespace WlcomData

Q_DECLARE_METATYPE(WlcomData::DBusEffectData)
Q_DECLARE_METATYPE(WlcomData::DBusEffectDataList)

inline QDBusArgument &operator<<(QDBusArgument &arg, const WlcomData::DBusEffectData &data)
{
    arg.beginStructure();
    arg << data.name << data.priority << data.enable;
    arg.endStructure();
    return arg;
}

inline const QDBusArgument &operator>>(const QDBusArgument &arg, WlcomData::DBusEffectData &data)
{
    arg.beginStructure();
    arg >> data.name >> data.priority >> data.enable;
    arg.endStructure();
    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const WlcomData::DBusEffectDataList &list)
{
    arg.beginArray(qMetaTypeId<WlcomData::DBusEffectData>());
    for (const auto &item : list) {
        arg << item;
    }
    arg.endArray();
    return arg;
}

inline const QDBusArgument &operator>>(const QDBusArgument &arg,
                                       WlcomData::DBusEffectDataList &list)
{
    arg.beginArray();
    list.clear();
    while (!arg.atEnd()) {
        WlcomData::DBusEffectData item;
        arg >> item;
        list.append(item);
    }
    arg.endArray();
    return arg;
}

#endif // DBUSINTERFACEDATE_H
