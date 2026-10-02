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

#ifndef QT5GESTUREEXTENSIONS_GLOBAL_H
#define QT5GESTUREEXTENSIONS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QT5GESTUREEXTENSIONS_LIBRARY)
#  define QT5GESTUREEXTENSIONS_EXPORT Q_DECL_EXPORT
#else
#  define QT5GESTUREEXTENSIONS_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \brief QLibrary only can use C-style interface, so provide
 * GestureExtensionManager class at the same time provide C interface
*/
class QWidget;
class QObject;
extern "C" void registerWidget(QWidget* widget, QObject* parent);
extern "C" void unregisterWidget(QWidget* widget, QObject* parent);

#endif // QT5GESTUREEXTENSIONS_GLOBAL_H
