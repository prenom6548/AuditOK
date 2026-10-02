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

#ifndef FONT_COLOR_PUSH_BUTTON_H
#define FONT_COLOR_PUSH_BUTTON_H

#include "resizable_push_button.h"
#include "ukui_settings_monitor.h"
#include <QColor>

class FontColorPushButton : public ResizablePushButton
{
    Q_OBJECT
private:
    /* data */
public:
    enum BtnType {
        None = 0,
        Circle,
        Rect
    };
    explicit FontColorPushButton(QWidget *parent = 0,
                                 const QColor &color = QColor(),
                                 BtnType type = BtnType::None);
    ~FontColorPushButton();
    void setButtonColor(const QColor &color);
    void setIconType(BtnType type);

    QColor buttonColor() const;
private:
    QColor m_buttonColor{QColor()};
    BtnType m_IconType{Rect};
    void handleStyleStatus(UkuiSettingsMonitor::StyleStatus status);
    QIcon getButtonIcon(const QColor &buttonColor, BtnType iconType);
protected:
    virtual void handleDarkMode();
    virtual void handleLightMode();
    void showEvent(QShowEvent *event) override;

protected:
    void handlePCMode() override;
    void handlePADMode() override;
};

#endif // FONT_COLOR_PUSH_BUTTON_H
