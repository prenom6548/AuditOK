/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef GESTUREGUIDANCE_H
#define GESTUREGUIDANCE_H

#include <QWidget>

namespace Ui {
class GestureGuidance;
}

namespace TouchScreenSetting {
class FlowLayout;
}

class ItemWidget;
class QSettings;
class GestureGuidance : public QWidget
{
public:
    GestureGuidance(QWidget* parent = nullptr);

private:
    Ui::GestureGuidance* ui;
    TouchScreenSetting::FlowLayout* gifFlowLayout;
    QSettings* gesturesetting;
    ItemWidget* m_currentItem = nullptr;

    void addItem();
};

#endif // GESTUREGUIDANCE_H
