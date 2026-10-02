/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

/*!
    \qmltype GradientColor
    \inqmlmodule org.ukui.quick.platform
    \ingroup ukui-quick-platform
    \brief 渐变色处理，支持纯色和渐变模式。

    GradientColor是一个功渐变色处理工具，支持纯色模式和渐变模式。
    在渐变模式下，可以设置起始色、结束色、背景色和渐变角度。

    \section1 主要属性

    \list
    \li \b{pureColor} - 纯色模式下的颜色
    \li \b{gradientStart} - 渐变起始色
    \li \b{gradientEnd} - 渐变结束色
    \li \b{gradientBackground} - 渐变背景色
    \li \b{gradientDeg} - 渐变角度（度）
    \li \b{colorType} - 颜色类型（纯色或渐变）
    \endlist

    \section1 静态方法

    \list
    \li \b{setAlphaF(color, alpha)} - 设置颜色透明度
    \li \b{mixBackGroundColor(color1, color2, alpha)} - 混合背景色
    \li \b{getGradientStartPoint(deg, width, height)} - 获取渐变起始点
    \endlist

    \section1 使用示例

    在QML中创建渐变色：
    \qml
    import org.ukui.quick.platform 1.0

    DtThemeBackground {
        width: 200
        height: 60
        backgroundColor: GradientColor {
            colorType: DtColorType.Gradient
            gradientDeg: 45
            gradientStart: "#FF6B6B"
            gradientEnd: "#4ECDC4"
            gradientBackground: "#FFFFFF"
        }
    }
    \endqml

    使用静态方法：
    \qml
    Text {
        function onInit() {
            // 设置透明度
            var color = GradientColor.setAlphaF(Qt.rgba(1,0,0,1), 0.5)

            // 混合背景色
            var mixed = GradientColor.mixBackGroundColor(
                Qt.rgba(1,0,0,1),
                Qt.rgba(0,1,0,1),
                0.3
            )
        }
    }
    \endqml

    \sa DtTheme, ShadowData, Theme
*/

#include "gradient-color.h"
#include <QtMath>
#include <QPoint>
namespace UkuiQuick {

GradientColor::GradientColor()
{
    m_gradientDeg = 0;
    m_gradientStart = QColor(0,0,0,0);
    m_gradientEnd = QColor(0,0,0,0);
    m_gradientBacrground = QColor(0,0,0,0);
    m_pureColor = QColor(0,0,0,0);
    m_colorType = DtColorType::Type::Pure;
}

GradientColor::GradientColor(QColor pure)
{
    m_gradientDeg = 0;
    m_gradientStart = QColor(0,0,0,0);
    m_gradientEnd = QColor(0,0,0,0);
    m_gradientBacrground = QColor(0,0,0,0);
    m_pureColor = pure;
    m_colorType = DtColorType::Type::Pure;
}

GradientColor::GradientColor(int deg, QColor start, QColor end, QColor background)
{
    m_gradientDeg = deg;
    m_gradientStart = start;
    m_gradientEnd = end;
    m_gradientBacrground = background;
    m_colorType = DtColorType::Type::Gradient;
}

GradientColor::GradientColor(const GradientColor &color)
{
    m_gradientDeg = color.gradientDeg();
    m_gradientStart = color.gradientStart();
    m_gradientEnd = color.gradientEnd();
    m_gradientBacrground = color.gradientBackground();
    m_pureColor = color.pureColor();
    m_colorType = color.colorType();
}

GradientColor &GradientColor::operator= (const GradientColor &color)
{
    m_gradientDeg = color.gradientDeg();
    m_gradientStart = color.gradientStart();
    m_gradientEnd = color.gradientEnd();
    m_gradientBacrground = color.gradientBackground();
    m_pureColor = color.pureColor();
    m_colorType = color.colorType();
    return *this;
}

bool GradientColor::operator!= (GradientColor *color)
{
    if(color == nullptr) {
        return true;
    }
    if(m_colorType != color->colorType()) {
        return true;
    }
    if(m_colorType == DtColorType::Pure && m_pureColor == color->pureColor()) {
        return false;
    }
    if(m_colorType == DtColorType::Gradient && m_gradientDeg == color->gradientDeg() && m_gradientStart == color->gradientStart() &&
       m_gradientEnd == color->gradientEnd() && m_gradientBacrground == color->gradientBackground()) {
        return false;
    }
    return true;
}

bool GradientColor::operator!= (QColor color)
{
    if(m_colorType == DtColorType::Gradient) {
        return true;
    }
    if(m_pureColor == color) {
        return false;
    }
    return true;
}

QColor GradientColor::gradientStart() const
{
    return m_gradientStart;
};

QColor GradientColor::gradientEnd() const
{
    return m_gradientEnd;
};

QColor GradientColor::gradientBackground() const
{
    return m_gradientBacrground;
};

int GradientColor::gradientDeg() const
{
    return m_gradientDeg;
};

QColor GradientColor::pureColor() const
{
    return m_pureColor;
}

DtColorType::Type GradientColor::colorType() const
{
    return m_colorType;
}

void GradientColor::setGradientStart(QColor start)
{
    if(m_gradientStart != start) {
        m_gradientStart = start;
        Q_EMIT gradientStartChanged();
    }
}

void GradientColor::setGradientEnd(QColor end)
{
    if(m_gradientEnd != end) {
        m_gradientEnd = end;
        Q_EMIT gradientEndChanged();
    }
}

void GradientColor::setGradientBackground(QColor background)
{
    if(m_gradientBacrground != background) {
        m_gradientBacrground = background;
        Q_EMIT gradientBackgroundChanged();
    }
}

void GradientColor::setGradientDeg(int deg)
{
    if(m_gradientDeg != deg) {
        m_gradientDeg = deg;
        Q_EMIT gradientDegChanged();
    }
}

void GradientColor::setPureColor(QColor color)
{
    if(m_pureColor != color) {
        m_pureColor = color;
        Q_EMIT pureColorChanged();
    }
}

void GradientColor::setColorType(UkuiQuick::DtColorType::Type type)
{
    if(m_colorType != type) {
        m_colorType = type;
        Q_EMIT colorTypeChanged();
    }
}

QColor GradientColor::setAlphaF(QColor color, qreal alpha)
{
    color.setAlphaF(color.alphaF() * alpha > 1? 1 : color.alphaF() * alpha);
    return color;
}

QColor GradientColor::mixBackGroundColor(const QColor &color, const QColor &back, const qreal &alpha)
{
    if(color.alpha() == 0) {
        return back;
    }
    if(back.alpha() == 0) {
        return color;
    }

    qreal a = back.alphaF() + (1 - back.alphaF()) * color.alphaF();
    if(a == 0) {
        return QColor(0, 0, 0, 0);
    }
    qreal r = (back.red() * back.alphaF() * (1 - color.alphaF()) + color.red() * color.alphaF()) / a;
    qreal g = (back.green() * back.alphaF() * (1 - color.alphaF()) + color.green() * color.alphaF()) / a;
    qreal b = (back.blue() * back.alphaF() * (1 - color.alphaF()) + color.blue() * color.alphaF()) / a;
    a = a * alpha;
    return QColor(qRound(r), qRound(g), qRound(b), qRound(a > 1 ? 255 : a * 255));
}

QPoint GradientColor::getGradientStartPoint(int deg, int width, int height)
{
    int x = qRound((0.5 - 0.5 * qCos(deg * M_PI / 180)) * width);
    int y = qRound((0.5 - 0.5 * qSin(deg * M_PI / 180)) * height);
    return QPoint(x, y);
}

QPoint GradientColor::getGradientEndPoint(int deg, int width, int height)
{
    int x = qRound((0.5 + 0.5 * qCos(deg * M_PI / 180)) * width);
    int y = qRound((0.5 + 0.5 * qSin(deg * M_PI / 180)) * height);
    return QPoint(x, y);
}
}
