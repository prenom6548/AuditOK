/*
 * KWin Style UKUI
 *
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef UKUIDECORATION_H
#define UKUIDECORATION_H

#include <KDecoration2/Decoration>
#include <KPluginFactory>
#include <QFont>
#include <QGSettings>

namespace KDecoration2
{
class DecorationButtonGroup;
}

namespace UKUI
{

struct ShadowParams
{
    ShadowParams()
        : offset(QPoint(0, 0))
        , radius(0)
        , opacity(0)
    {
    }

    ShadowParams(const QPoint &offset, int radius, qreal opacity)
        : offset(offset)
        , radius(radius)
        , opacity(opacity)
    {
    }

    QPoint offset;
    int radius;
    qreal opacity;
};

struct CompositeShadowParams
{
    CompositeShadowParams() = default;

    CompositeShadowParams(
        const QPoint &offset,
        const ShadowParams &shadow1,
        const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2)
    {
    }

    bool isNone() const
    {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

enum ThemeProject {
    THEME_PROJECT_UKUI_BASE = 0,
    THEME_PROJECT_LAIKA,
    THEME_PROJECT_MAVIS,
    THEME_PROJECT_XC_TABLET,

    THEME_PROJECT_MAX
};

enum KylinIconTheme {
    IconThemeDefault = 0,
    IconThemeFashion,
    IconThemeClassical
};

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    void init() override;
    void paint(QPainter *painter, const QRect &repaintRegion) override;

    QColor fontColor() const;
    int themeId()
    {
        return m_themeId;
    }
    KylinIconTheme iconThemeId()
    {
        return m_iconTheme;
    }
    ThemeProject getThemeProject()
    {
        return m_themeProject;
    }
    bool getTabletMode()
    {
        return m_bTabletMode;
    }

    int borderTop() const;
    int buttonWidth() const;
    int buttonHeight() const;
    int leftButtonWidth() const;
    int leftButtonHeight() const;
    int innerButtonBorder() const;
    int buttonMarginTop() const;
    int leftButtonMarginTop() const;
    int buttonSpacing() const;
    int leftButtonSpacing() const;

private:
    // pc members
    int m_borderTop; // decoration 顶部宽度，即标题栏高度
    int m_borderBottom; // decoration 底部宽度
    int m_borderLeft; // decoration 左侧宽度
    int m_borderRight; // decoration 右侧宽度

    int m_innerButtonBorder; // 三联按钮内边距

    int m_buttonWidth; // 按钮宽度
    int m_buttonHeight; // 按钮高度

    int m_leftButtonWidth; // 左侧按钮宽度
    int m_leftButtonHeight; // 左侧按钮高度

    int m_ButtonMarginTop; // 三联按钮上边缘与 decoration 上边缘间距
    int m_leftButtonMarginTop; // 左侧按钮上边缘与 decoration 上边缘间距

    int m_buttonSpacing; // 三联按钮间隔
    int m_leftButtonSpacing; // 左侧按钮间隔

    // tablet members
    int m_tabletBorderTop; // 平板模式下主界面 decoration 顶部宽度，即标题栏高度
    int m_tabletBorderTopForModal; // 平板模式下模态窗口 decoration 顶部宽度，即标题栏高度

    int m_tabletInnerButtonBorder; // 平板三联按钮内边距

    int m_tabletButtonWidth; // 平板三联按钮宽度
    int m_tabletButtonHeight; // 平板三联按钮高度

    int m_tabletLeftButtonWidth; // 平板左侧按钮宽度
    int m_tabletLeftButtonHeight; // 平板左侧按钮高度

    int m_tabletButtonMarginTop; // 三联按钮上边缘与 decoration 上边缘间距
    int m_tabletLeftButtonMarginTop; // 平板左侧按钮上边缘与 decoration 上边缘间距
    int m_tabletButtonMarginTopForModal; // 平板模态窗口三联按钮上边缘与 decoration 上边缘间距
    int m_tabletLeftButtonMarginTopForModal; // 平板模态窗口左侧按钮上边缘与 decoration 上边缘间距

    int m_tabletButtonSpacing; // 平板三联按钮间隔
    int m_tabletLeftButtonSpacing; // 平板左侧按钮间隔

    QColor m_BorderColor; // 边框颜色

    int m_nleftButtonCout;
    int m_nrightButtonCout;

    KDecoration2::DecorationButtonGroup *m_leftButtons;
    KDecoration2::DecorationButtonGroup *m_rightButtons;

    int m_shadowRadius;
    int m_shadowBorder_active;
    int m_shadowBorder_inactive;
    QPoint m_shadowShift_active;
    QPoint m_shadowShift_inactive;
    float m_shadowAlphaF_active;
    float m_shadowAlphaF_inactive;
    float m_borderAlphaF;
    QColor m_borderActiveColor;
    QColor m_borderInactiveColor;
    QColor m_shadowActiveColor;
    QColor m_shadowInactiveColor;
    QColor m_frameActiveColor;
    QColor m_frameInactiveColor;
    QColor m_fontColor; // 活动字体颜色
    int m_themeId;
    ThemeProject m_themeProject;
    KylinIconTheme m_iconTheme;
    bool m_bTabletMode;

    double m_scaler;
    QFont m_Font;
    std::unique_ptr<QGSettings> m_styleSettings;

    bool m_rightToLeft = false;

public Q_SLOTS:
    void updateButtonsGeometry();
    void calculateBorders();
    void calculateLeftButtonCount();
    void calculateRightButtonCout();
    void updateTitleBar();
    void updateShadow();
    void themeUpdate(int themeId);
    void updatefont(QFont font);
    void slotSwitchTablet(bool bTabletMode);
    void iconThemeUpdate(int);
};

}

#endif // UKUIDECORATION_H
