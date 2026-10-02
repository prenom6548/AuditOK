/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
    \class UkuiQuick::ShadowData
    \brief 阴影数据类，配置阴影效果参数。

    ShadowData类提供完整的阴影效果配置，包括偏移、模糊半径、
    扩展、颜色等参数。支持多层阴影效果。

    \section1 主要属性

    \list
    \li \b{xOffset} - X轴偏移
    \li \b{yOffset} - Y轴偏移
    \li \b{blurRadius} - 模糊半径
    \li \b{spread} - 扩展值
    \li \b{shadowColor} - 阴影颜色
    \li \b{isMultiShadow} - 是否为多层阴影
    \li \b{childShadow} - 子阴影对象
    \endlist

    \section1 构造方法

    \list
    \li \b{ShadowData()} - 默认构造函数
    \li \b{ShadowData(int x, int y, int blurRadius, int spread, const QColor &color)} - 完整构造函数
    \li \b{ShadowData(int x, int y, int blurRadius, int spread, const QColor &color, bool isMulti, ShadowData* child)} - 多层阴影构造函数
    \endlist

    \section1 使用示例

    在QML中配置阴影：
    \qml
    import org.ukui.quick.platform 1.0

    Rectangle {
        width: 100
        height: 100
        color: "white"

        shadowData: ShadowData {
            xOffset: 4
            yOffset: 4
            blurRadius: 10
            spread: 0
            shadowColor: Qt.rgba(0,0,0,0.3)
        }
    }
    \endqml

    \sa GradientColor, DtTheme
*/

#include "shadow-data.h"
namespace UkuiQuick {

ShadowData::ShadowData(QObject* parent): QObject(parent)
{
}

ShadowData::~ShadowData()
{
    deleteChildShadow();
}

ShadowData::ShadowData(int x, int y, int blurRadius, int spread, const QColor&color, bool isMulti, ShadowData* child,
                       QObject* parent)
    : QObject(parent)
      , m_xOffset(x)
      , m_yOffset(y)
      , m_blurRadius(blurRadius)
      , m_spread(spread)
      , m_color(color)
      , m_isMultiShadow(isMulti)
{
    setChildShadow(child);
}

ShadowData::ShadowData(const ShadowData &shadow)
{
    m_xOffset = shadow.xOffset();
    m_yOffset = shadow.yOffset();
    m_blurRadius = shadow.blurRadius();
    m_spread = shadow.spread();
    m_color = shadow.shadowColor();
    m_isMultiShadow = shadow.isMultiShadow();
    setChildShadow(shadow.childShadow());
}

ShadowData &ShadowData::operator=(const ShadowData &shadow)
{
    m_xOffset = shadow.xOffset();
    m_yOffset = shadow.yOffset();
    m_blurRadius = shadow.blurRadius();
    m_spread = shadow.spread();
    m_color = shadow.shadowColor();
    m_isMultiShadow = shadow.isMultiShadow();
    setChildShadow(shadow.childShadow());
    return *this;
}

bool ShadowData::operator!=(const ShadowData &shadow)
{
    if(m_xOffset == shadow.xOffset() &&
        m_yOffset == shadow.yOffset()&&
        m_blurRadius == shadow.blurRadius() &&
        m_spread == shadow.spread() &&
        m_color == shadow.shadowColor() &&
        m_isMultiShadow == shadow.isMultiShadow()) {
        if (m_isMultiShadow && m_childShadow && shadow.childShadow()){
            return *m_childShadow != *shadow.childShadow();
        }
        return false;
    }
    return true;
}

void ShadowData::setXOffset(int x)
{
    if(m_xOffset != x) {
        m_xOffset = x;
        Q_EMIT xOffsetChanged();
    }
}

void ShadowData::setYOffset(int y)
{
    if(m_yOffset != y) {
        m_yOffset = y;
        Q_EMIT yOffsetChanged();
    }
}

void ShadowData::setBlurRadius(int radius)
{
    if(m_blurRadius != radius) {
        m_blurRadius = radius;
        Q_EMIT blurRadiusChanged();
    }
}

void ShadowData::setSpread(int spread)
{
    if(m_spread != spread) {
        m_spread = spread;
        Q_EMIT spreadChanged();
    }
}

void ShadowData::setShadowColor(const QColor &color)
{
    if(m_color != color) {
        m_color = color;
        Q_EMIT shadowColorChanged();
    }
}

void ShadowData::setIsMultiShadow(bool isMultiShadow)
{
    if(m_isMultiShadow != isMultiShadow) {
        m_isMultiShadow = isMultiShadow;
        Q_EMIT isMultiShadowChanged();
    }
}

void ShadowData::setChildShadow(ShadowData *child)
{
    if (child && childShadowWouldCreateCycle(child)) {
        return;
    }

    QSet<const ShadowData *> visited;
    setChildShadow(child, visited);
}

void ShadowData::setChildShadow(ShadowData *child, QSet<const ShadowData *> &visited)
{
    if (child == m_childShadow.data()) {
        return;
    }

    if (child && visited.contains(child)) {
        return;
    }

    if (!child) {
        if (!m_childShadow && !m_isMultiShadow) {
            return;
        }

        deleteChildShadow();
        setIsMultiShadow(false);
        Q_EMIT childShadowChanged();
        return;
    }

    const bool hadChildShadow = m_childShadow;
    const bool sourceChanged = m_childShadowSource != child;
    if (!hadChildShadow) {
        clearChildShadowConnections();
        m_childShadow.reset(new ShadowData(this));
    } else if (sourceChanged) {
        clearChildShadowConnections();
    }

    visited.insert(child);
    copyChildShadowValues(child, visited);
    if (!hadChildShadow || sourceChanged) {
        m_childShadowSource = child;
        connectChildShadowSource(child);
    }

    setIsMultiShadow(true);
    if (!hadChildShadow || sourceChanged) {
        Q_EMIT childShadowChanged();
    }
}

int ShadowData::xOffset() const
{
    return m_xOffset;
}

int ShadowData::yOffset() const
{
    return m_yOffset;
}

int ShadowData::blurRadius() const
{
    return m_blurRadius;
}

int ShadowData::spread() const
{
    return m_spread;
}

QColor ShadowData::shadowColor() const
{
    return m_color;
}

bool ShadowData::isMultiShadow() const
{
    return m_isMultiShadow;
}

ShadowData *ShadowData::childShadow() const
{
    return m_childShadow.data();
}

bool ShadowData::childShadowWouldCreateCycle(const ShadowData *child) const
{
    QSet<const ShadowData *> visited;
    return referencesShadowData(child, this, visited);
}

bool ShadowData::referencesShadowData(const ShadowData *root, const ShadowData *target, QSet<const ShadowData *> &visited)
{
    if (!root) {
        return false;
    }

    if (root == target) {
        return true;
    }

    if (visited.contains(root)) {
        return false;
    }
    visited.insert(root);

    if (referencesShadowData(root->childShadow(), target, visited)) {
        return true;
    }

    return referencesShadowData(root->m_childShadowSource.data(), target, visited);
}

void ShadowData::clearChildShadowConnections()
{
    for (const auto &connection : m_childShadowConnections) {
        QObject::disconnect(connection);
    }
    m_childShadowConnections.clear();
    m_childShadowSource = nullptr;
}

void ShadowData::copyChildShadowValues(const ShadowData *child, QSet<const ShadowData *> &visited)
{
    if (!child || !m_childShadow) {
        return;
    }

    m_childShadow->setXOffset(child->xOffset());
    m_childShadow->setYOffset(child->yOffset());
    m_childShadow->setBlurRadius(child->blurRadius());
    m_childShadow->setSpread(child->spread());
    m_childShadow->setShadowColor(child->shadowColor());
    m_childShadow->setIsMultiShadow(child->isMultiShadow());
    m_childShadow->setChildShadow(child->childShadow(), visited);
}

void ShadowData::connectChildShadowSource(ShadowData *child)
{
    if (!child || !m_childShadow || child == m_childShadow.data()) {
        return;
    }

    m_childShadowConnections.append(connect(child, &ShadowData::xOffsetChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setXOffset(child->xOffset());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::yOffsetChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setYOffset(child->yOffset());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::blurRadiusChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setBlurRadius(child->blurRadius());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::spreadChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setSpread(child->spread());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::shadowColorChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setShadowColor(child->shadowColor());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::isMultiShadowChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setIsMultiShadow(child->isMultiShadow());
        }
    }));
    m_childShadowConnections.append(connect(child, &ShadowData::childShadowChanged, this, [this, child]() {
        if (m_childShadow) {
            m_childShadow->setChildShadow(child->childShadow());
        }
    }));
}

void ShadowData::deleteChildShadow()
{
    clearChildShadowConnections();
    m_childShadow.reset();
}

}
