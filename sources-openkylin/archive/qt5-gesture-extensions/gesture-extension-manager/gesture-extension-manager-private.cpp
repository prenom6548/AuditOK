/*
Copyright (c) KylinSoft  Co., Ltd. 2021.All rights reserved.
License: GPL-2+
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 2 can be found in "/usr/share/common-licenses/GPL-2".
*/

#include "gesture-extension-manager-private.h"
#include "../gesture-extensions/slide-gesture.h"

#include <QDebug>

namespace UkuiGestureExtension {

GestureExtensionManagerPrivate::GestureExtensionManagerPrivate(QObject* parent) : QObject(parent)
{
    registerGestureExtension();
}

GestureExtensionManagerPrivate::~GestureExtensionManagerPrivate()
{
//    qDebug() << "~GestureExtensionManagerPrivate";
    if (m_extensions.isEmpty())
        return;
    for (auto begin = m_extensions.begin(); begin != m_extensions.end(); ++begin) {
        delete *begin;
    }
}

void GestureExtensionManagerPrivate::registerWidget(QWidget* widget)
{
    for (auto begin = m_extensions.begin(); begin != m_extensions.end(); ++begin) {
        if (widget && (*begin)->isWidgetUseExtension(widget))
            (*begin)->registerWidget(widget);
    }
}

void GestureExtensionManagerPrivate::unregisterWidget(QWidget* widget)
{
    for (auto begin = m_extensions.begin(); begin != m_extensions.end(); ++begin) {
        (*begin)->unregisterWidget(widget);
    }
}

void GestureExtensionManagerPrivate::registerGestureExtension()
{
    SlideGesture* slideListView = new SlideGesture(this);
    m_extensions.push_back(slideListView);

    //! \todo load extension from extension dir
}

} // namespace
