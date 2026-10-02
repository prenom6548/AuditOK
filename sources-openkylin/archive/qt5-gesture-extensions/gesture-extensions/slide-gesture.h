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

#ifndef SLIDEGESTURE_H
#define SLIDEGESTURE_H

#include "gesture-extension.h"

#include <QObject>
#include <QPoint>

class QTimer;
class QAbstractScrollArea;
namespace UkuiGestureExtension {

class SlideGesture : public QObject, public GestureExtension
{
    Q_OBJECT
public:
    explicit SlideGesture(QObject* parent = nullptr);
//    ~SlideListViewGesture() { qDebug() << "~SlideListViewGesture"; }
    void registerWidget(QWidget* widget);
    void unregisterWidget(QWidget* widget);

    const QString extensionId() override { return "SlideListViewGesture"; }

private:
    QPoint m_press_pos;
    QTimer* m_touch_slide_timer;
    QString m_project_codename;

    bool isWidgetUseExtension(QWidget* widget);
};

} // namespace

#endif // SLIDEGESTURE_H
