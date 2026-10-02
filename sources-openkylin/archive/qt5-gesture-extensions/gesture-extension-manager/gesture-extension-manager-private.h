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

#ifndef GESTUREEXTENSIONMANAGERPRIVATE_H
#define GESTUREEXTENSIONMANAGERPRIVATE_H

#include <QObject>
#include <QVector>

namespace UkuiGestureExtension {

class GestureExtension;
class GestureExtensionManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit GestureExtensionManagerPrivate(QObject *parent = nullptr);
    ~GestureExtensionManagerPrivate();

    /*!
     * \brief registerWidget
     * \details register the widget to all extension
    */
    void registerWidget(QWidget* widget);
    /*!
     * \brief unregisterWidget
     * \details unregister the widget to all extension
    */
    void unregisterWidget(QWidget* widget);

    void registerGestureExtension();

private:

    QVector<GestureExtension*> m_extensions;

};

} // namespace

#endif // GESTUREEXTENSIONMANAGERPRIVATE_H
