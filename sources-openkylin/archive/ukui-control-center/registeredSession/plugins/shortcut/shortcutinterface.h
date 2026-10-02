/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SHORTCUTINTERFACE_H
#define SHORTCUTINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>
#include <QMap>
#include "getshortcutworker.h"
#include "type.h"
#include "../../universalinterface.h"

class ShortcutInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Shortcut")

public:
    ShortcutInterface();
    ~ShortcutInterface();
    void getThreadStart();

Q_SIGNALS:
    void changed(QString key);

public Q_SLOTS:
    QList<QStringPair> getSystemShortcut();
    QList<QStringPair> getWindowShortcut();
    QList<QStringPair> getCustomShortcut();
    QList<KeyEntry> getSystemShortcutEntry();
    QList<KeyEntry> getWindowShortcutEntry();
    QList<KeyEntry> getCustomShortcutEntry();
    QVariantMap getShortKeyValue();
    bool updateShortcut();
    void setSystemShortcut(const QString &key, const QString &value);
    void setCustomShortcut(const QString &key, const QString &value);
    void setWindowShortcut(const QString &key, const QString &oldValue, const QString &value);
    void defaultShortcut();

private:
    void getShortcutThread();
    void defaultWindowShortcut();
    void defaultGeneralShortcut();
    QMap<QString, QString> MergerOfTheSamekind(QMap<QString, QString> desktopMap);
    void buildGeneralWidget(QString schema, QMap<QString, QString> subShortcutsMap);
    QString getShowShortcutString(QString src);
    void workerCompleteSlot();
    void generalShortcutSlot(QString schema, QString key, QString value);
    void windowShortcutSlot(QString name, QString key, QString value);
    void customShortcutSlot(QString path,
                            QString name,
                            QString binding,
                            QString action);
    void setDefaultShortcut(QString key, QString value);

private:
    QList<KeyEntry> m_systemEntries;
    QList<KeyEntry> m_windowEntries;
    QList<KeyEntry> m_customEntries;
    QList<KeyEntry> m_windowSortEntries;
    QVariantMap m_shortKeyValue;
    QList<QStringPair> m_systemShortcutList;
    QList<QStringPair> m_windowShortcutList;
    QList<QStringPair> m_customShortcutList;
    GetShortcutWorker* m_toGetShortcutThread = nullptr;
};

#endif // SHORTCUTINTERFACE_H
