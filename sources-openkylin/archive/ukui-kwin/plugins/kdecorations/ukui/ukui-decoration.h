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
#include <KConfigGroup>
#include <KSharedConfig>

namespace KDecoration2 {
class DecorationButtonGroup;
}

namespace UKUI {

struct ShadowParams {
    ShadowParams()
        : offset(QPoint(0, 0))
        , radius(0)
        , opacity(0) {}

    ShadowParams(const QPoint &offset, int radius, qreal opacity)
        : offset(offset)
        , radius(radius)
        , opacity(opacity) {}

    QPoint offset;
    int radius;
    qreal opacity;
};

struct CompositeShadowParams {
    CompositeShadowParams() = default;

    CompositeShadowParams(
            const QPoint &offset,
            const ShadowParams &shadow1,
            const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2) {}

    bool isNone() const {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

//主题
enum KylinThemeMode { Light, Dark };
enum KylinTheme {
    ThemeDefault = 0,
    ThemeClassical,
    ThemeFashion,

    // 定制版本
    ThemeLaika = 100,
    ThemeMavis,

    ThemeMax
};

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    void init();
    void paint(QPainter *painter, const QRect &repaintRegion) override;

    QColor fontColor() const;
    QColor borderColor()const;
    KylinTheme themeId() {return m_theme;}
    KylinThemeMode themeMode() {return m_themeMode;}
    bool getTabletMode() {return m_bTabletMode;}

private:
    void readconfig();
    void saveConfig();
private:
    int m_borderLeft;
    int m_borderTop;
    int m_borderRight;
    int m_borderBottom;

    int m_buttonWidth;          //按钮宽度
    int m_buttonHeight;         //按钮高度

    int m_leftButtonWidth;      //左侧按钮高度
    int m_leftButtonHeight;     //左侧按钮宽度

    int m_ButtonMarginTop;      //按钮上空白

    int m_buttonSpacing;        //按钮空隙

    QColor m_fontActiveColor;   //活动字体颜色
    QColor m_fontInactiveColor; //非活动字体颜色

    QColor m_frameActiveColor;
    QColor m_frameInactiveColor;

    int m_shadowRadius;
    int m_shadowBorder;    
    float m_shadowAlphaF;
    float m_activeDarkness;
    QColor m_borderActiveColor;
    QColor m_borderInactiveColor;



    bool m_bTabletMode;
    int m_nleftButtonCout;
    int m_nrightButtonCout;

    KylinThemeMode m_themeMode;
    KylinTheme m_theme;

    KDecoration2::DecorationButtonGroup* m_leftButtons;
    KDecoration2::DecorationButtonGroup* m_rightButtons;

    QFont m_Font;

    int m_scaler;

public slots:
    void updateButtonsGeometry();
    void calculateBorders();
    void calculateRightButtonCout();
    void updateTitleBar();
    void updateShadow(int);
    void themeModeUpdate(int);
    void updatefont(QFont font);
    void slotSwitchTablet(bool bTabletMode);

};

}

#endif // UKUIDECORATION_H
