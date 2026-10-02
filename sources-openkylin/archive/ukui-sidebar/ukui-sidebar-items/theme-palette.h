/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-11-6.
//

#ifndef UKUI_SIDEBAR_THEME_PALETTE_H
#define UKUI_SIDEBAR_THEME_PALETTE_H

#include <QObject>
#include <QPalette>

class PaletteRole
{
    Q_GADGET
public:
    // Warning 警告: 谨防 qt版本更新后，删除，增加或者调整调色板的枚举值
    enum ColorGroup { Active, Disabled, Inactive };
    Q_ENUM(ColorGroup)

    enum ColorRole { Window, WindowText, Base, Text, AlternateBase,
        Button, ButtonText, Light, MidLight, Dark, Mid, Shadow,
        Highlight, HighlightedText
    };
    Q_ENUM(ColorRole)
};

class ThemePalette : public QObject
{
    Q_OBJECT
public:
    static ThemePalette *getInstance();

    /**
     * 根据调色板的枚举值，获取主题调色板的颜色
     * @param colorRole
     * @return
     */
    Q_INVOKABLE QColor paletteColor(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    /**
     * 获取自定义透明度的颜色
     * @param colorRole
     * @return
     */
    Q_INVOKABLE QColor paletteColorWithCustomTransparency(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup, qreal alphaF) const;

    /**
     * 获取带有主题透明度的颜色
     * @param colorRole
     * @return
     */
    Q_INVOKABLE QColor paletteColorWithTransparency(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor window(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor windowText(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor base(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor text(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor alternateBase(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor button(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor buttonText(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor light(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor midLight(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor dark(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor mid(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor shadow(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor highlight(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;
    Q_INVOKABLE QColor highlightedText(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE QColor separator(PaletteRole::ColorGroup colorGroup = PaletteRole::Active) const;

    Q_INVOKABLE qreal fontSize() const;

    Q_INVOKABLE QString themeFont() const;

Q_SIGNALS:
    void styleColorChanged();
    void fontSizeChanged();
    void fontChanged();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
#endif

private:
    explicit ThemePalette(QObject *parent = nullptr);
    static QPalette::ColorGroup switchColorGroup(PaletteRole::ColorGroup colorGroup);
    void initTransparency();
    void initStyleSetting();

private:
    qreal m_transparency = 1.0;
    qreal m_fontSize = 12;
    QString m_font = "Noto Sans CJK SC";
};


#endif //UKUI_SIDEBAR_THEME_PALETTE_H
