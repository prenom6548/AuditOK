/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 */

#ifndef QUICKLOOKPLUGINMANAGER_H
#define QUICKLOOKPLUGINMANAGER_H

#include "peony-core_global.h"

#include <QObject>
#include <QHash>
#include <QStringList>

class QWidget;

namespace Peony {

class QuickLookPluginIface;
class QuickLookWindowIface;

class PEONYCORESHARED_EXPORT QuickLookPluginManager : public QObject
{
    Q_OBJECT
public:
    static QuickLookPluginManager *getInstance();

    bool registerPlugin(QuickLookPluginIface *plugin);
    bool unregisterPlugin(QuickLookPluginIface *plugin);

    const QStringList getPluginIds() const;
    QuickLookPluginIface *getPlugin(const QString &pluginId) const;

    void showQuickLook(const QString &uri, QWidget *parent = nullptr);
    void closeQuickLook();

private:
    explicit QuickLookPluginManager(QObject *parent = nullptr);
    ~QuickLookPluginManager();

    QuickLookPluginIface *availablePlugin() const;
    void clearCurrentWindow();

    QHash<QString, QuickLookPluginIface *> m_hash;
    QuickLookPluginIface *m_currentPlugin = nullptr;
    QuickLookWindowIface *m_currentWindow = nullptr;
};

}

#endif // QUICKLOOKPLUGINMANAGER_H
