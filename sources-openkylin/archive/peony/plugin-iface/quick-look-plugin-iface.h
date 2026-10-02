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

#ifndef QUICKLOOKPLUGINIFACE_H
#define QUICKLOOKPLUGINIFACE_H

#include <QtPlugin>
#include <QString>
#include "plugin-iface.h"

#define QuickLookPluginIface_iid "org.ukui.peony-qt.plugin-iface.QuickLookPluginInterface"

class QWidget;
class QObject;

namespace Peony {

/*!
 * \brief QuickLookWindowIface
 * \details
 * Represents a popup quick-look window for a single URI.
 * Implementations live in peony-extensions; Peony core only dispatches show/close.
 */
class QuickLookWindowIface
{
public:
    virtual ~QuickLookWindowIface() {}

    virtual void prepare(const QString &uri) = 0;
    virtual void show(QWidget *parent = nullptr) = 0;
    virtual void closeQuickLook() = 0;
    virtual bool isVisible() const = 0;
    virtual QObject *asObject() = 0;
};

/*!
 * \brief QuickLookPluginIface
 * \details
 * Factory plugin for space-key quick look popup windows.
 */
class QuickLookPluginIface : public PluginInterface
{
public:
    virtual ~QuickLookPluginIface() {}

    virtual QuickLookWindowIface *createQuickLookWindow() = 0;
};

}

Q_DECLARE_INTERFACE(Peony::QuickLookPluginIface, QuickLookPluginIface_iid)

#endif // QUICKLOOKPLUGINIFACE_H
