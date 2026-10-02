/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "color-mixer.h"

namespace UkuiQuick {

/*!
    \qmltype ColorMix
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供颜色混合功能的组件。

    ColorMix类通过设置前景色和背景色，根据特定的混合模式生成新的颜色。
    颜色与主题色进行绑定，自动跟随主题切换。

    \section1 主要特性
    \list
        \li 支持多种混合模式（普通、变亮、变暗、加深、减淡、叠加、排除）
        \li 支持前景色和背景色的主题颜色角色绑定
        \li 支持自定义透明度
        \li 自动跟随主题切换
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    import org.ukui.quick.items 1.0
    import org.ukui.quick.platform 1.0

    Rectangle {
        ColorMix {
            id: mixer
            mixMode: ColorMix.Normal
            backColorRole: Theme.Base
            foreColorRole: Theme.Highlight
            foreColorAlpha: 0.5
        }
        color: mixer.color
    }
    \endqml
*/

/*!
    \qmlproperty enumeration ColorMix::MixMode
    \brief 颜色混合模式枚举。

    可选值：
    \list
        \li ColorMix.NoMixer - 禁用混合，直接使用背景色
        \li ColorMix.Normal - 普通混合模式
        \li ColorMix.Lighten - 变亮模式
        \li ColorMix.Darken - 变暗模式
        \li ColorMix.Deepen - 加深模式
        \li ColorMix.Dodge - 减淡模式
        \li ColorMix.Overlay - 叠加模式
        \li ColorMix.Exclusion - 排除模式
    \endlist
*/

/*!
    \qmlproperty color ColorMix::color
    \brief 混合后的颜色。

    \readonly

    返回根据混合模式计算后的最终颜色。这是一个只读属性。
*/

/*!
    \qmlproperty enumeration ColorMix::mixMode
    \brief 颜色混合模式。

    设置前景色和背景色的混合方式。
    可选值请参阅 ColorMix::MixMode。
    默认值为 ColorMix.NoMixer。
*/

/*!
    \qmlproperty real ColorMix::backColorAlpha
    \brief 背景色的透明度。

    取值范围为 [0, 1]。如果不在此范围内，则跟随主题透明度。
    默认值为 0。
*/

/*!
    \qmlproperty real ColorMix::foreColorAlpha
    \brief 前景色的透明度。

    取值范围为 [0, 1]。如果不在此范围内，则跟随主题透明度。
    默认值为 0。
*/

/*!
    \qmlproperty enumeration ColorMix::backColorRole
    \brief 背景色的主题颜色角色。

    指定从主题中获取哪种颜色作为背景色。
    可选值请参阅 Theme::ColorRole。
    默认值为 Theme.Base。

    \sa Theme::ColorRole
*/

/*!
    \qmlproperty enumeration ColorMix::foreColorRole
    \brief 前景色的主题颜色角色。

    指定从主题中获取哪种颜色作为前景色。
    可选值请参阅 Theme::ColorRole。
    默认值为 Theme.Base。

    \sa Theme::ColorRole
*/

/*!
    \qmlproperty enumeration ColorMix::backColorGroup
    \brief 背景色的主题颜色组。

    指定背景色使用的颜色组（Active、Inactive等）。
    可选值请参阅 Theme::ColorGroup。
    默认值为 Theme.Active。

    \sa Theme::ColorGroup
*/

/*!
    \qmlproperty enumeration ColorMix::foreColorGroup
    \brief 前景色的主题颜色组。

    指定前景色使用的颜色组（Active、Inactive等）。
    可选值请参阅 Theme::ColorGroup。
    默认值为 Theme.Active。

    \sa Theme::ColorGroup
*/

ColorMix::ColorMix(QObject *parent) : QObject(parent)
{
    connect(Theme::instance(), &Theme::paletteChanged, this, &ColorMix::updateColor);
    connect(Theme::instance(), &Theme::themeTransparencyChanged, this, [this] {
        // 不在[0, 1]范围内，跟随主题透明度
        if (m_backColorAlpha < 0 || m_backColorAlpha > 1 || m_foreColorAlpha < 0 || m_foreColorAlpha > 1) {
            updateColor();
        }
    });
}

QColor ColorMix::color()
{
    return m_color;
}

ColorMix::MixMode ColorMix::mixMode() const
{
    return m_mixMode;
}

void ColorMix::setMixMode(ColorMix::MixMode mixMode)
{
    if (m_mixMode == mixMode) {
        return;
    }

    m_mixMode = mixMode;
    updateColor();

    Q_EMIT mixModeChanged();
}

void ColorMix::updateColor()
{
    if (m_mixMode == NoMixer) {
        m_color = ColorMix::backColor();
    } else {
        QColor fore = ColorMix::foreColor();
        QColor back = ColorMix::backColor();

        switch (m_mixMode) {
            default:
            case Normal: {
              qreal tiny = 1 - fore.alphaF();
              qreal alpha = fore.alphaF() + back.alphaF() * tiny;

              qreal r = (fore.redF() * fore.alphaF() +
                         back.redF() * back.alphaF() * tiny) /
                        alpha;
              qreal g = (fore.greenF() * fore.alphaF() +
                         back.greenF() * back.alphaF() * tiny) /
                        alpha;
              qreal b = (fore.blueF() * fore.alphaF() +
                         back.blueF() * back.alphaF() * tiny) /
                        alpha;

              m_color = QColor::fromRgbF(r, g, b, alpha);
              break;
            }
            case Lighten:
              break;
            case Darken:
              break;
            case Deepen:
              break;
            case Dodge:
              break;
            case Overlay:
              break;
            case Exclusion:
              break;
        }
    }

    Q_EMIT colorChanged();
}

Theme::ColorRole ColorMix::backColorRole() const
{
    return m_backColorRole;
}

void ColorMix::setBackColorRole(Theme::ColorRole role)
{
    if (m_backColorRole == role) {
        return;
    }

    m_backColorRole = role;
    updateColor();
}

Theme::ColorRole ColorMix::foreColorRole() const
{
    return m_foreColorRole;
}

void ColorMix::setForeColorRole(Theme::ColorRole role)
{
    if (m_foreColorRole == role) {
        return;
    }

    m_foreColorRole = role;
    updateColor();
}

Theme::ColorGroup ColorMix::backColorGroup() const
{
    return m_backColorGroup;
}

void ColorMix::setBackColorGroup(Theme::ColorGroup group)
{
    if (m_backColorGroup == group) {
        return;
    }

    m_backColorGroup = group;
    updateColor();
}

Theme::ColorGroup ColorMix::foreColorGroup() const
{
    return m_foreColorGroup;
}

void ColorMix::setForeColorGroup(Theme::ColorGroup group)
{
    if (m_foreColorGroup == group) {
        return;
    }

    m_foreColorGroup = group;
    updateColor();
}

qreal ColorMix::backColorAlpha() const
{
    return m_backColorAlpha;
}

void ColorMix::setBackColorAlpha(qreal alpha)
{
    if (m_backColorAlpha == alpha) {
        return;
    }

    m_backColorAlpha = alpha;
    updateColor();
}

qreal ColorMix::foreColorAlpha() const
{
    return m_foreColorAlpha;
}

void ColorMix::setForeColorAlpha(qreal alpha)
{
    if (m_foreColorAlpha == alpha) {
        return;
    }

    m_foreColorAlpha = alpha;
    updateColor();
}

QColor ColorMix::backColor()
{
    return Theme::instance()->color(m_backColorRole, m_backColorGroup, m_backColorAlpha);
}

QColor ColorMix::foreColor()
{
    return Theme::instance()->color(m_foreColorRole, m_foreColorGroup, m_foreColorAlpha);
}

} // UkuiQuick
