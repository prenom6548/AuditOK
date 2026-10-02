/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef MEDIAKEYABSTRACTSETTINGS_H
#define MEDIAKEYABSTRACTSETTINGS_H

#include <QDebug>
#include <QObject>
#include <QCoreApplication>
#include <QTranslator>
#include "media-type.h"
#include "clib-syslog.h"
#include "QGSettings/qgsettings.h"

#define UKUI_DAEMON_NAME "ukui-settings-daemon"

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <glib/gi18n.h>

#ifdef __cplusplus
}
#endif //__cplusplus


class MediaKeyAbstractSettings : public QObject
{
    Q_OBJECT
    typedef QSharedPointer<QGSettings> GsettingsPtr;
    typedef QList<GsettingsPtr> GsettingsPtrList;
    typedef QMap<QString , QVariant> GsettingsMap;
public:
    explicit MediaKeyAbstractSettings(QString schema, QObject *parent = nullptr);
    ~MediaKeyAbstractSettings();
    virtual void initSettings() = 0;

    //恢复默认
    void resetSettings();
    void setGsettings(QString schema);
    void clearMapData();
    bool resultInitSettings();
    const QStringList getGsettingsKeys();

    QVariant getGsettingsValue(const QString& key);
    QString getSummary(const QString &key) const;

    GsettingsPtr getGsettings();
    GsettingsMap getKeysData();
    void insertKeysData(QString key, QVariant qvar);

private:
    MediaKeyAbstractSettings(const MediaKeyAbstractSettings&) = delete;
    MediaKeyAbstractSettings& operator =(const MediaKeyAbstractSettings&)=delete;

private:
    void initMouseGsettings();

private:
    GsettingsPtr m_settings;
    GsettingsMap m_keysData;


public Q_SLOTS:
    void onKeyChanged(const QString&);
Q_SIGNALS:
    void keyChanged(const QString&, QVariant);
};

#endif // MEDIAKEYABSTRACTSETTINGS_H
