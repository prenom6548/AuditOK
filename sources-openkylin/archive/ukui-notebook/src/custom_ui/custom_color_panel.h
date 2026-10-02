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

#ifndef CUSTOM_COLOR_PANEL_H
#define CUSTOM_COLOR_PANEL_H

#include "ktranslucentfloor.h"
#include "kcolorbutton.h"
#include "ukui_settings_monitor.h"

#include <QVBoxLayout>
#include <QButtonGroup>



class CustomColorPanel : public kdk::KTranslucentFloor
{
    Q_OBJECT

public:
    CustomColorPanel(QWidget *parent = nullptr);
    void init();
    void addColor(const QColor& color = QColor(Qt::black), bool isClicked = false);
    void setSelectColor(const QColor& color);
    static QString BLACK;
    static QString WHITE;

private:
    QVBoxLayout *m_layout{nullptr};
    QButtonGroup *m_group{nullptr};
    QList<kdk::KColorButton *> m_buttons;
    kdk::KColorButton *m_blackOrWhiteButton{nullptr};
    kdk::KColorButton *m_selectButton{nullptr};

    void handleStyleStatus(UkuiSettingsMonitor::StyleStatus status);
    void handleTabletMode(UkuiSettingsMonitor::TabletStatus status);
    bool isBlackOrWhite(const QColor& color);
    void setSelectColorButton(kdk::KColorButton *button);

protected:
    virtual void handleDarkMode();
    virtual void handleLightMode();
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

signals:
    void currentColorChanged(const QColor& color);
};

#endif // CUSTOM_COLOR_PANEL_H
