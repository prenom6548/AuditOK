/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

/*!
    \qmltype BorderGroup
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 矩形边框属性组。

    BorderGroup是一个分组属性类，用于管理ShadowedRectangle的边框属性。
    包括边框宽度和颜色。

    \section1 属性

    \qmlproperty real BorderGroup::width
    \brief 边框宽度。

    边框的宽度，单位为像素。
    默认值为 0。

    \qmlproperty color BorderGroup::color
    \brief 边框颜色。

    边框的颜色，支持完整的 RGBA 颜色格式。
    默认值为黑色。

    \sa ShadowedRectangle
*/

/*!
    \qmltype ShadowGroup
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 矩形阴影属性组。

    ShadowGroup是一个分组属性类，用于管理ShadowedRectangle的阴影属性。
    包括阴影大小、X/Y偏移和颜色。

    \section1 属性

    \qmlproperty real ShadowGroup::size
    \brief 阴影大小。

    阴影的大小，单位为像素。
    默认值为 0。

    \qmlproperty real ShadowGroup::xOffset
    \brief 阴影X轴偏移。

    阴影在X轴方向的偏移量，单位为像素。
    默认值为 0。

    \qmlproperty real ShadowGroup::yOffset
    \brief 阴影Y轴偏移。

    阴影在Y轴方向的偏移量，单位为像素。
    默认值为 0。

    \qmlproperty color ShadowGroup::color
    \brief 阴影颜色。

    阴影的颜色，支持完整的 RGBA 颜色格式。
    默认值为黑色。

    \sa ShadowedRectangle
*/

/*!
    \qmltype CornersGroup
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 矩形圆角属性组。

    CornersGroup是一个分组属性类，用于管理ShadowedRectangle的圆角属性。
    可以为四个角分别设置不同的圆角半径。

    \section1 属性

    \qmlproperty real CornersGroup::topLeftRadius
    \brief 左上角圆角半径。

    左上角的圆角半径，单位为像素。
    设置为 -1 表示使用全局 radius 属性。
    默认值为 -1。

    \qmlproperty real CornersGroup::topRightRadius
    \brief 右上角圆角半径。

    右上角的圆角半径，单位为像素。
    设置为 -1 表示使用全局 radius 属性。
    默认值为 -1。

    \qmlproperty real CornersGroup::bottomLeftRadius
    \brief 左下角圆角半径。

    左下角的圆角半径，单位为像素。
    设置为 -1 表示使用全局 radius 属性。
    默认值为 -1。

    \qmlproperty real CornersGroup::bottomRightRadius
    \brief 右下角圆角半径。

    右下角的圆角半径，单位为像素。
    设置为 -1 表示使用全局 radius 属性。
    默认值为 -1。

    \sa ShadowedRectangle
*/

/*!
    \qmltype ShadowedRectangle
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供带阴影的矩形组件。

    ShadowedRectangle是一个高性能的矩形组件，支持阴影、边框和圆角。
    使用距离场（Distance Field）技术进行渲染，提供了优秀的性能表现。
    阴影在Item边界外渲染。

    \section1 主要特性
    \list
        \li 支持阴影效果（大小、偏移、颜色可配置）
        \li 支持边框（宽度、颜色可配置）
        \li 支持圆角（可为四个角分别设置）
        \li 支持纯色和渐变色背景
        \li 高性能渲染（使用距离场技术）
        \li 支持软件渲染回退
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    import org.ukui.quick.items 1.0

    ShadowedRectangle {
        width: 200
        height: 100
        radius: 8
        color: "white"

        shadow {
            size: 20
            color: Qt.rgba(0, 0, 0, 0.5)
            yOffset: 2
        }

        border {
            width: 1
            color: "lightgray"
        }
    }
    \endqml

    渐变色用法：
    \qml
    ShadowedRectangle {
        width: 200
        height: 100
        pureColor: false
        startColor: "blue"
        endColor: "red"
        angle: 45
    }
    \endqml

    \sa BorderGroup, ShadowGroup, CornersGroup
*/

/*!
    \qmlproperty enumeration ShadowedRectangle::RenderType
    \brief 渲染类型枚举。

    \list
        \li ShadowedRectangle.Auto - 自动确定最佳渲染类型
        \li ShadowedRectangle.HighQuality - 使用最高质量渲染
        \li ShadowedRectangle.LowQuality - 使用最低质量渲染
        \li ShadowedRectangle.Software - 强制使用软件渲染
    \endlist
*/

/*!
    \qmlproperty real ShadowedRectangle::radius
    \brief 矩形的圆角半径。

    设置矩形四个角的圆角半径，单位为像素。
    可以使用 corners 属性为每个角分别设置不同的半径。
    默认值为 0（直角矩形）。

    \sa CornersGroup
*/

/*!
    \qmlproperty color ShadowedRectangle::color
    \brief 矩形的背景颜色。

    设置矩形的背景颜色，支持完整的 RGBA 颜色格式。
    当 pureColor 为 false 时，此属性被 startColor 和 endColor 替代。
    默认值为白色。
*/

/*!
    \qmlproperty bool ShadowedRectangle::pureColor
    \brief 是否使用纯色背景。

    \list
        \li true - 使用纯色背景（默认）
        \li false - 使用渐变色背景
    \endlist

    默认值为 true。
*/

/*!
    \qmlproperty color ShadowedRectangle::startColor
    \brief 渐变起始颜色。

    设置渐变色的起始颜色，仅在 pureColor 为 false 时生效。
    默认值为白色。

    \sa endColor, angle
*/

/*!
    \qmlproperty color ShadowedRectangle::endColor
    \brief 渐变结束颜色。

    设置渐变色的结束颜色，仅在 pureColor 为 false 时生效。
    默认值为白色。

    \sa startColor, angle
*/

/*!
    \qmlproperty real ShadowedRectangle::angle
    \brief 渐变角度。

    设置渐变色的角度，单位为度。
    0 表示从左到右，90 表示从上到下。
    仅在 pureColor 为 false 时生效。
    默认值为 0。

    \sa startColor, endColor
*/

/*!
    \qmlproperty BorderGroup* ShadowedRectangle::border
    \brief 边框属性组。

    \readonly

    返回边框属性组对象，用于设置边框宽度和颜色。

    \qml
    ShadowedRectangle {
        border.width: 2
        border.color: "blue"
    }
    \endqml

    \sa BorderGroup
*/

/*!
    \qmlproperty ShadowGroup* ShadowedRectangle::shadow
    \brief 阴影属性组。

    \readonly

    返回阴影属性组对象，用于设置阴影大小、偏移和颜色。

    \qml
    ShadowedRectangle {
        shadow.size: 20
        shadow.xOffset: 5
        shadow.yOffset: 5
        shadow.color: Qt.rgba(0, 0, 0, 0.3)
    }
    \endqml

    \sa ShadowGroup
*/

/*!
    \qmlproperty CornersGroup* ShadowedRectangle::corners
    \brief 圆角属性组。

    \readonly

    返回圆角属性组对象，用于为四个角分别设置不同的圆角半径。
    此属性的值会覆盖 radius 属性的对应角。

    \qml
    ShadowedRectangle {
        corners.topLeftRadius: 4
        corners.topRightRadius: 5
        corners.bottomLeftRadius: 2
        corners.bottomRightRadius: 10
    }
    \endqml

    \sa CornersGroup
*/

/*!
    \qmlproperty enumeration ShadowedRectangle::renderType
    \brief 渲染模式。

    设置矩形的渲染模式。
    可选值请参阅 ShadowedRectangle::RenderType。
    默认值为 ShadowedRectangle.Auto。

    \sa RenderType
*/

/*!
    \qmlproperty bool ShadowedRectangle::softwareRendering
    \brief 是否正在使用软件渲染。

    \readonly

    返回当前是否正在使用软件渲染。
*/

#include "shadowed-rectangle.h"

#include <cmath>
#include <QQuickWindow>
#include <QSGRectangleNode>
#include <QSGRendererInterface>

#include "painted-rectangle-item.h"
#if QT_CONFIG(opengl) || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "shadowed-rectangle-node.h"
#endif


BorderGroup::BorderGroup(QObject *parent)
    : QObject(parent)
{
}

qreal BorderGroup::width() const
{
    return m_width;
}

void BorderGroup::setWidth(qreal newWidth)
{
    if (newWidth == m_width) {
        return;
    }

    m_width = newWidth;
    Q_EMIT changed();
}

QColor BorderGroup::color() const
{
    return m_color;
}

void BorderGroup::setColor(const QColor &newColor)
{
    if (newColor == m_color) {
        return;
    }

    m_color = newColor;
    Q_EMIT changed();
}

ShadowGroup::ShadowGroup(QObject *parent)
    : QObject(parent)
{
}

qreal ShadowGroup::size() const
{
    return m_size;
}

void ShadowGroup::setSize(qreal newSize)
{
    if (newSize == m_size) {
        return;
    }

    m_size = newSize;
    Q_EMIT changed();
}

qreal ShadowGroup::xOffset() const
{
    return m_xOffset;
}

void ShadowGroup::setXOffset(qreal newXOffset)
{
    if (newXOffset == m_xOffset) {
        return;
    }

    m_xOffset = newXOffset;
    Q_EMIT changed();
}

qreal ShadowGroup::yOffset() const
{
    return m_yOffset;
}

void ShadowGroup::setYOffset(qreal newYOffset)
{
    if (newYOffset == m_yOffset) {
        return;
    }

    m_yOffset = newYOffset;
    Q_EMIT changed();
}

QColor ShadowGroup::color() const
{
    return m_color;
}

void ShadowGroup::setColor(const QColor &newColor)
{
    if (newColor == m_color) {
        return;
    }

    m_color = newColor;
    Q_EMIT changed();
}

CornersGroup::CornersGroup(QObject *parent)
    : QObject(parent)
{
}

qreal CornersGroup::topLeft() const
{
    return m_topLeft;
}

void CornersGroup::setTopLeft(qreal newTopLeft)
{
    if (newTopLeft == m_topLeft) {
        return;
    }

    m_topLeft = newTopLeft;
    Q_EMIT changed();
}

qreal CornersGroup::topRight() const
{
    return m_topRight;
}

void CornersGroup::setTopRight(qreal newTopRight)
{
    if (newTopRight == m_topRight) {
        return;
    }

    m_topRight = newTopRight;
    Q_EMIT changed();
}

qreal CornersGroup::bottomLeft() const
{
    return m_bottomLeft;
}

void CornersGroup::setBottomLeft(qreal newBottomLeft)
{
    if (newBottomLeft == m_bottomLeft) {
        return;
    }

    m_bottomLeft = newBottomLeft;
    Q_EMIT changed();
}

qreal CornersGroup::bottomRight() const
{
    return m_bottomRight;
}

void CornersGroup::setBottomRight(qreal newBottomRight)
{
    if (newBottomRight == m_bottomRight) {
        return;
    }

    m_bottomRight = newBottomRight;
    Q_EMIT changed();
}

QVector4D CornersGroup::toVector4D(float all) const
{
    return QVector4D{m_bottomRight < 0.0 ? all : m_bottomRight,
                     m_topRight < 0.0 ? all : m_topRight,
                     m_bottomLeft < 0.0 ? all : m_bottomLeft,
                     m_topLeft < 0.0 ? all : m_topLeft};
}

ShadowedRectangle::ShadowedRectangle(QQuickItem *parentItem)
    : QQuickItem(parentItem)
    , m_border(std::make_unique<BorderGroup>())
    , m_shadow(std::make_unique<ShadowGroup>())
    , m_corners(std::make_unique<CornersGroup>())
{
    QQmlEngine::setObjectOwnership(m_border.get(), QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(m_shadow.get(), QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(m_corners.get(), QQmlEngine::CppOwnership);

    setFlag(QQuickItem::ItemHasContents, true);

    connect(m_border.get(), &BorderGroup::changed, this, &ShadowedRectangle::update);
    connect(m_shadow.get(), &ShadowGroup::changed, this, &ShadowedRectangle::update);
    connect(m_corners.get(), &CornersGroup::changed, this, &ShadowedRectangle::update);
    //workaround
    connect(this, &QQuickItem::activeFocusOnTabChanged, this, &ShadowedRectangle::activeFocusOnTabChanged1);
}

ShadowedRectangle::~ShadowedRectangle()
{
}

BorderGroup *ShadowedRectangle::border() const
{
    return m_border.get();
}

ShadowGroup *ShadowedRectangle::shadow() const
{
    return m_shadow.get();
}

CornersGroup *ShadowedRectangle::corners() const
{
    return m_corners.get();
}

qreal ShadowedRectangle::radius() const
{
    return m_radius;
}

void ShadowedRectangle::setRadius(qreal newRadius)
{
    if (newRadius == m_radius) {
        return;
    }

    m_radius = newRadius;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT radiusChanged();
}

QColor ShadowedRectangle::color() const
{
    return m_color;
}

void ShadowedRectangle::setColor(const QColor &newColor)
{
    if (newColor == m_color) {
        return;
    }

    m_color = newColor;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT colorChanged();
}

bool ShadowedRectangle::pureColor() const
{
    return m_pureColor;
}

void ShadowedRectangle::setPureColor(bool pureColor)
{
    if (pureColor == m_pureColor) {
        return;
    }

    m_pureColor = pureColor;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT pureColorChanged();
}

QColor ShadowedRectangle::startColor() const
{
    return m_startColor;
}

void ShadowedRectangle::setStartColor(const QColor& startColor)
{
    if (startColor == m_startColor) {
        return;
    }

    m_startColor = startColor;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT startColorChanged();
}

QColor ShadowedRectangle::endColor() const
{
    return m_endColor;
}

void ShadowedRectangle::setEndColor(const QColor& endColor)
{
    if (endColor == m_endColor) {
        return;
    }

    m_endColor = endColor;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT endColorChanged();
}

qreal ShadowedRectangle::angle() const
{
    return m_angle;
}

void ShadowedRectangle::setAngle(qreal newAngle)
{
    auto deg = newAngle / 180.0 * M_PI;
    if (m_angle == deg) {
        return;
    }

    m_angle = deg;
    if (!isSoftwareRendering()) {
        update();
    }
    Q_EMIT angleChanged();
}

ShadowedRectangle::RenderType ShadowedRectangle::renderType() const
{
    return m_renderType;
}

void ShadowedRectangle::setRenderType(RenderType renderType)
{
    if (renderType == m_renderType) {
        return;
    }
    m_renderType = renderType;
    update();
    Q_EMIT renderTypeChanged();
}

void ShadowedRectangle::componentComplete()
{
    QQuickItem::componentComplete();

    checkSoftwareItem();
}

bool ShadowedRectangle::isSoftwareRendering() const
{
    return (window() && window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) || m_renderType == RenderType::Software;
}

PaintedRectangleItem *ShadowedRectangle::softwareItem() const
{
    return m_softwareItem;
}

void ShadowedRectangle::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange && value.window) {
        checkSoftwareItem();
        // TODO: only conditionally emit?
        Q_EMIT softwareRenderingChanged();
    }

    QQuickItem::itemChange(change, value);
}

QSGNode *ShadowedRectangle::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

#if QT_CONFIG(opengl) || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (boundingRect().isEmpty()) {
        delete node;
        return nullptr;
    }

    auto shadowNode = static_cast<ShadowedRectangleNode *>(node);

    if (!shadowNode) {
        shadowNode = new ShadowedRectangleNode{};

        if (m_renderType == RenderType::LowQuality) {
            shadowNode->setShaderType(ShadowedRectangleMaterial::ShaderType::LowPower);
        }
    }

    shadowNode->setBorderEnabled(m_border->isEnabled());
    shadowNode->setRect(boundingRect());
    shadowNode->setSize(m_shadow->size());
    shadowNode->setRadius(m_corners->toVector4D(m_radius));
    shadowNode->setOffset(QVector2D{static_cast<float>(m_shadow->xOffset()), static_cast<float>(m_shadow->yOffset())});
    shadowNode->setColor(m_color);
    shadowNode->setPureColor(m_pureColor);
    shadowNode->setStartColor(m_startColor);
    shadowNode->setEndColor(m_endColor);
    shadowNode->setAngle(m_angle);
    shadowNode->setShadowColor(m_shadow->color());
    shadowNode->setBorderWidth(m_border->width());
    shadowNode->setBorderColor(m_border->color());
    shadowNode->updateGeometry();
    return shadowNode;
#else
    Q_UNUSED(node)
    return nullptr;
#endif
}

void ShadowedRectangle::checkSoftwareItem()
{
    if (!m_softwareItem && isSoftwareRendering()) {
        m_softwareItem = new PaintedRectangleItem{this};
        // The software item is added as a "normal" child item, this means it
        // will be part of the normal item sort order. Since there is no way to
        // control the ordering of children, just make sure to have a very low Z
        // value for the child, to force it to be the lowest item.
        m_softwareItem->setZ(-99.0);

        auto updateItem = [this]() {
            auto borderWidth = m_border->width();
            auto rect = boundingRect();
            m_softwareItem->setSize(rect.size());
            m_softwareItem->setColor(m_color);
            m_softwareItem->setRadius(m_corners->toVector4D(m_radius));
            m_softwareItem->setPureColor(m_pureColor);
            m_softwareItem->setStartColor(m_startColor);
            m_softwareItem->setEndColor(m_endColor);
            m_softwareItem->setAngle(m_angle);
            m_softwareItem->setBorderWidth(borderWidth);
            m_softwareItem->setBorderColor(m_border->color());
        };

        updateItem();

        connect(this, &ShadowedRectangle::widthChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::heightChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::colorChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::radiusChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::pureColorChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::startColorChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::endColorChanged, m_softwareItem, updateItem);
        connect(this, &ShadowedRectangle::angleChanged, m_softwareItem, updateItem);
        connect(m_border.get(), &BorderGroup::changed, m_softwareItem, updateItem);
        connect(m_corners.get(), &CornersGroup::changed, m_softwareItem, updateItem);

        setFlag(QQuickItem::ItemHasContents, false);
    }
}
