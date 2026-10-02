/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __FONT_PROPERTY_WIDGET_H__
#define __FONT_PROPERTY_WIDGET_H__

#include <QWidget>
#include "font_property_push_button.h"
QT_BEGIN_NAMESPACE
namespace Ui { class FontPropertyWidget; }
QT_END_NAMESPACE



class FontPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    FontPropertyWidget(QWidget *parent = nullptr);
    ~FontPropertyWidget();

    FontPropertyPushButton *boldBtn();
    FontPropertyPushButton *italicBtn();
    FontPropertyPushButton *underlineBtn();
    FontPropertyPushButton *strikeOutBtn();
    FontPropertyPushButton *unorderedBtn();
    FontPropertyPushButton *orderedBtn();
    FontPropertyPushButton *insertBtn();
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(UkuiSettingsMonitor::TabletStatus status);
private:
    Ui::FontPropertyWidget *ui;

};
#endif // __FONT_PROPERTY_WIDGET_H__
