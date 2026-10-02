/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSignalSpy>
#include <QtTest>

#include "shadowed-rectangle.h"
#include "scenegraph/painted-rectangle-item.h"
#include "scenegraph/shadowed-border-rectangle-material.h"
#include "scenegraph/shadowed-rectangle-node.h"

class TestableShadowedRectangle : public ShadowedRectangle
{
public:
    using ShadowedRectangle::ShadowedRectangle;
    using ShadowedRectangle::componentComplete;
    using ShadowedRectangle::softwareItem;
};

namespace {

QColor premultipliedColor(const QColor &color)
{
    return QColor::fromRgbF(color.redF() * color.alphaF(),
                            color.greenF() * color.alphaF(),
                            color.blueF() * color.alphaF(),
                            color.alphaF());
}

QRectF geometryBounds(const QSGGeometry *geometry)
{
    const auto *vertices = geometry->vertexDataAsTexturedPoint2D();
    qreal minX = vertices[0].x;
    qreal maxX = vertices[0].x;
    qreal minY = vertices[0].y;
    qreal maxY = vertices[0].y;

    for (int index = 1; index < geometry->vertexCount(); ++index) {
        minX = qMin<qreal>(minX, vertices[index].x);
        maxX = qMax<qreal>(maxX, vertices[index].x);
        minY = qMin<qreal>(minY, vertices[index].y);
        maxY = qMax<qreal>(maxY, vertices[index].y);
    }

    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

class ExposedShadowedRectangleNode : public ShadowedRectangleNode
{
public:
    ShadowedRectangleMaterial *material() const
    {
        return m_material;
    }

    QSGGeometry *geometry() const
    {
        return m_geometry;
    }
};

class ExposedShadowedRectangleShader : public ShadowedRectangleShader
{
public:
    using ShadowedRectangleShader::ShadowedRectangleShader;
    using ShadowedRectangleShader::compile;
    using ShadowedRectangleShader::initialize;
    using ShadowedRectangleShader::updateState;
};

class ExposedShadowedBorderRectangleShader : public ShadowedBorderRectangleShader
{
public:
    using ShadowedBorderRectangleShader::ShadowedBorderRectangleShader;
    using ShadowedBorderRectangleShader::compile;
    using ShadowedBorderRectangleShader::initialize;
    using ShadowedBorderRectangleShader::updateState;
};

bool makeCurrentOffscreenContext(QOffscreenSurface &surface, QOpenGLContext &context)
{
    surface.setFormat(QSurfaceFormat::defaultFormat());
    surface.create();
    if (!surface.isValid()) {
        return false;
    }

    context.setFormat(surface.requestedFormat());
    if (!context.create()) {
        return false;
    }

    return context.makeCurrent(&surface);
}

}

class shadowed_rectangle_test : public QObject
{
    Q_OBJECT

private slots:
    void groupedPropertiesExposeDefaultsAndEmitOnChange();
    void cornersGroupFallsBackToGlobalRadius();
    void rectangleMaterialsCompareOnTrackedState();
    void borderMaterialsCompareOnTrackedState();
    void rectangleNodePropagatesStateIntoMaterials();
    void rectangleNodeHandlesPortraitAspectAndIgnoresBorderSettersWithoutBorderMaterial();
    void rectangleNodeUpdatesGeometryForShaderMode();
    void rectangleShadersUploadStateOffscreen();
    void rectanglePropertiesEmitOnlyOnRealChanges();
    void softwareRenderTypeCreatesFallbackItem();
};

void shadowed_rectangle_test::groupedPropertiesExposeDefaultsAndEmitOnChange()
{
    BorderGroup border;
    QCOMPARE(border.width(), 0.0);
    QCOMPARE(border.color(), QColor(Qt::black));
    QVERIFY(!border.isEnabled());

    QSignalSpy borderSpy(&border, &BorderGroup::changed);
    border.setWidth(0.0);
    border.setColor(QColor(Qt::black));
    QCOMPARE(borderSpy.count(), 0);

    border.setWidth(2.5);
    QCOMPARE(border.width(), 2.5);
    QVERIFY(border.isEnabled());
    QCOMPARE(borderSpy.count(), 1);

    border.setColor(QColor(Qt::red));
    QCOMPARE(border.color(), QColor(Qt::red));
    QCOMPARE(borderSpy.count(), 2);

    ShadowGroup shadow;
    QCOMPARE(shadow.size(), 0.0);
    QCOMPARE(shadow.xOffset(), 0.0);
    QCOMPARE(shadow.yOffset(), 0.0);
    QCOMPARE(shadow.color(), QColor(Qt::black));

    QSignalSpy shadowSpy(&shadow, &ShadowGroup::changed);
    shadow.setSize(12.0);
    shadow.setXOffset(3.0);
    shadow.setYOffset(-4.0);
    shadow.setColor(QColor(Qt::blue));

    QCOMPARE(shadow.size(), 12.0);
    QCOMPARE(shadow.xOffset(), 3.0);
    QCOMPARE(shadow.yOffset(), -4.0);
    QCOMPARE(shadow.color(), QColor(Qt::blue));
    QCOMPARE(shadowSpy.count(), 4);
}

void shadowed_rectangle_test::cornersGroupFallsBackToGlobalRadius()
{
    CornersGroup corners;
    QCOMPARE(corners.topLeft(), -1.0);
    QCOMPARE(corners.topRight(), -1.0);
    QCOMPARE(corners.bottomLeft(), -1.0);
    QCOMPARE(corners.bottomRight(), -1.0);
    QCOMPARE(corners.toVector4D(6.0f), QVector4D(6.0f, 6.0f, 6.0f, 6.0f));

    QSignalSpy cornersSpy(&corners, &CornersGroup::changed);
    corners.setTopLeft(1.0);
    corners.setTopRight(2.0);
    corners.setBottomLeft(3.0);
    corners.setBottomRight(4.0);

    QCOMPARE(cornersSpy.count(), 4);
    QCOMPARE(corners.toVector4D(6.0f), QVector4D(4.0f, 2.0f, 3.0f, 1.0f));
}

void shadowed_rectangle_test::rectangleMaterialsCompareOnTrackedState()
{
    ShadowedRectangleMaterial first;
    ShadowedRectangleMaterial same;
    ShadowedRectangleMaterial differentColor;
    ShadowedRectangleMaterial differentShadow;
    ShadowedRectangleMaterial differentOffset;
    ShadowedRectangleMaterial differentAspect;
    ShadowedRectangleMaterial differentTailState;

    QCOMPARE(first.type(), &ShadowedRectangleMaterial::staticType);
    QCOMPARE(first.compare(&same), 0);

    differentColor.color = QColor(Qt::red);
    QVERIFY(first.compare(&differentColor) != 0);

    differentShadow.shadowColor = QColor(Qt::blue);
    QVERIFY(first.compare(&differentShadow) != 0);

    differentOffset.offset = QVector2D(0.25f, 0.5f);
    QVERIFY(first.compare(&differentOffset) != 0);

    differentAspect.aspect = QVector2D(2.0f, 1.0f);
    QVERIFY(first.compare(&differentAspect) != 0);

    differentTailState.pureColor = false;
    differentTailState.startColor = QColor(Qt::green);
    differentTailState.endColor = QColor(Qt::yellow);
    differentTailState.angle = 90.0f;
    differentTailState.size = 0.5f;
    differentTailState.radius = QVector4D(0.2f, 0.3f, 0.4f, 0.5f);
    QVERIFY(first.compare(&differentTailState) != 0);
}

void shadowed_rectangle_test::borderMaterialsCompareOnTrackedState()
{
    ShadowedBorderRectangleMaterial first;
    ShadowedBorderRectangleMaterial same;
    ShadowedBorderRectangleMaterial differentColor;
    ShadowedBorderRectangleMaterial differentWidth;

    QCOMPARE(first.type(), &ShadowedBorderRectangleMaterial::staticType);
    QCOMPARE(first.compare(&same), 0);

    differentColor.borderColor = QColor(Qt::red);
    QVERIFY(first.compare(&differentColor) != 0);

    differentWidth.borderWidth = 0.25f;
    QVERIFY(first.compare(&differentWidth) != 0);
}

void shadowed_rectangle_test::rectangleNodePropagatesStateIntoMaterials()
{
    ExposedShadowedRectangleNode node;
    node.setBorderEnabled(false);
    QVERIFY(node.material() != nullptr);
    QCOMPARE(node.material()->type(), &ShadowedRectangleMaterial::staticType);

    node.setRect(QRectF(10.0, 20.0, 80.0, 40.0));
    node.setSize(10.0);
    node.setRadius(QVector4D(6.0f, 8.0f, 10.0f, 12.0f));
    node.setColor(QColor(255, 0, 0, 128));
    node.setShadowColor(QColor(0, 0, 255, 64));
    node.setOffset(QVector2D(4.0f, -8.0f));
    node.setPureColor(false);
    node.setStartColor(QColor(Qt::yellow));
    node.setEndColor(QColor(Qt::cyan));
    node.setAngle(90.0);

    QCOMPARE(node.material()->aspect, QVector2D(2.0f, 1.0f));
    QCOMPARE(node.material()->size, 0.5f);
    QCOMPARE(node.material()->radius, QVector4D(0.3f, 0.4f, 0.5f, 0.6f));
    QCOMPARE(node.material()->color, premultipliedColor(QColor(255, 0, 0, 128)));
    QCOMPARE(node.material()->shadowColor, premultipliedColor(QColor(0, 0, 255, 64)));
    QCOMPARE(node.material()->offset, QVector2D(0.1f, -0.2f));
    QVERIFY(!node.material()->pureColor);
    QCOMPARE(node.material()->startColor, premultipliedColor(QColor(Qt::yellow)));
    QCOMPARE(node.material()->endColor, premultipliedColor(QColor(Qt::cyan)));
    QCOMPARE(node.material()->angle, 90.0f);

    node.setShaderType(ShadowedRectangleMaterial::ShaderType::LowPower);
    node.setBorderEnabled(true);

    auto *borderMaterial = static_cast<ShadowedBorderRectangleMaterial *>(node.material());
    QCOMPARE(borderMaterial->type(), &ShadowedBorderRectangleMaterial::staticType);
    QCOMPARE(borderMaterial->shaderType, ShadowedRectangleMaterial::ShaderType::LowPower);

    node.setRect(QRectF(10.0, 20.0, 80.0, 40.0));
    node.setBorderWidth(4.0);
    node.setBorderColor(QColor(20, 40, 60, 200));

    QCOMPARE(borderMaterial->borderWidth, 0.1f);
    QCOMPARE(borderMaterial->borderColor, premultipliedColor(QColor(20, 40, 60, 200)));
}

void shadowed_rectangle_test::rectangleNodeHandlesPortraitAspectAndIgnoresBorderSettersWithoutBorderMaterial()
{
    ExposedShadowedRectangleNode node;
    node.setBorderEnabled(false);
    node.setRect(QRectF(0.0, 0.0, 20.0, 40.0));

    QCOMPARE(node.material()->aspect, QVector2D(1.0f, 2.0f));

    node.setBorderWidth(5.0);
    node.setBorderColor(QColor(Qt::magenta));

    QCOMPARE(node.material()->type(), &ShadowedRectangleMaterial::staticType);
}

void shadowed_rectangle_test::rectangleNodeUpdatesGeometryForShaderMode()
{
    ExposedShadowedRectangleNode lowPowerNode;
    lowPowerNode.setBorderEnabled(false);
    lowPowerNode.setShaderType(ShadowedRectangleMaterial::ShaderType::LowPower);
    lowPowerNode.setRect(QRectF(5.0, 6.0, 40.0, 20.0));
    lowPowerNode.updateGeometry();
    QCOMPARE(geometryBounds(lowPowerNode.geometry()), QRectF(5.0, 6.0, 40.0, 20.0));

    ExposedShadowedRectangleNode standardNode;
    standardNode.setBorderEnabled(false);
    standardNode.setRect(QRectF(0.0, 0.0, 80.0, 40.0));
    standardNode.setSize(10.0);
    standardNode.setOffset(QVector2D(3.0f, 4.0f));
    standardNode.updateGeometry();
    QCOMPARE(geometryBounds(standardNode.geometry()), QRectF(-30.0, -15.0, 140.0, 70.0));
}

void shadowed_rectangle_test::rectangleShadersUploadStateOffscreen()
{
    QOffscreenSurface surface;
    QOpenGLContext context;
    if (!makeCurrentOffscreenContext(surface, context)) {
        QSKIP("离屏 OpenGL 上下文不可用");
    }

    ShadowedRectangleMaterial material;
    material.aspect = QVector2D(2.0f, 1.0f);
    material.size = 0.5f;
    material.radius = QVector4D(0.2f, 0.3f, 0.4f, 0.5f);
    material.color = QColor(Qt::red);
    material.shadowColor = QColor(Qt::blue);
    material.offset = QVector2D(0.1f, -0.2f);
    material.pureColor = false;
    material.startColor = QColor(Qt::yellow);
    material.endColor = QColor(Qt::cyan);
    material.angle = 90.0f;

    auto *createdShader = material.createShader();
    QVERIFY(createdShader != nullptr);
    const auto *attributeNames = createdShader->attributeNames();
    QCOMPARE(QByteArray(attributeNames[0]), QByteArray("in_vertex"));
    QCOMPARE(QByteArray(attributeNames[1]), QByteArray("in_uv"));
    QVERIFY(attributeNames[2] == nullptr);
    delete createdShader;

    ExposedShadowedRectangleShader shader(ShadowedRectangleMaterial::ShaderType::LowPower);
    shader.compile();
    shader.initialize();

    QSGMaterialShader::RenderState state{};
    shader.updateState(state, &material, nullptr);

    ShadowedBorderRectangleMaterial borderMaterial;
    borderMaterial.borderWidth = 0.25f;
    borderMaterial.borderColor = QColor(Qt::green);

    auto *createdBorderShader = borderMaterial.createShader();
    QVERIFY(createdBorderShader != nullptr);
    delete createdBorderShader;

    ExposedShadowedBorderRectangleShader borderShader(ShadowedRectangleMaterial::ShaderType::Standard);
    borderShader.compile();
    borderShader.initialize();
    borderShader.updateState(state, &borderMaterial, nullptr);
}

void shadowed_rectangle_test::rectanglePropertiesEmitOnlyOnRealChanges()
{
    TestableShadowedRectangle rectangle;
    QVERIFY(rectangle.border() != nullptr);
    QVERIFY(rectangle.shadow() != nullptr);
    QVERIFY(rectangle.corners() != nullptr);
    QCOMPARE(rectangle.border(), rectangle.border());
    QCOMPARE(rectangle.shadow(), rectangle.shadow());
    QCOMPARE(rectangle.corners(), rectangle.corners());

    QCOMPARE(rectangle.radius(), 0.0);
    QCOMPARE(rectangle.color(), QColor(Qt::white));
    QVERIFY(rectangle.pureColor());
    QCOMPARE(rectangle.startColor(), QColor(Qt::white));
    QCOMPARE(rectangle.endColor(), QColor(Qt::white));
    QCOMPARE(rectangle.angle(), 0.0);
    QCOMPARE(rectangle.renderType(), ShadowedRectangle::Auto);
    QVERIFY(!rectangle.isSoftwareRendering());

    QSignalSpy radiusSpy(&rectangle, &ShadowedRectangle::radiusChanged);
    QSignalSpy colorSpy(&rectangle, &ShadowedRectangle::colorChanged);
    QSignalSpy pureColorSpy(&rectangle, &ShadowedRectangle::pureColorChanged);
    QSignalSpy startColorSpy(&rectangle, &ShadowedRectangle::startColorChanged);
    QSignalSpy endColorSpy(&rectangle, &ShadowedRectangle::endColorChanged);
    QSignalSpy angleSpy(&rectangle, &ShadowedRectangle::angleChanged);
    QSignalSpy renderTypeSpy(&rectangle, &ShadowedRectangle::renderTypeChanged);

    rectangle.setRadius(0.0);
    rectangle.setColor(QColor(Qt::white));
    rectangle.setPureColor(true);
    rectangle.setStartColor(QColor(Qt::white));
    rectangle.setEndColor(QColor(Qt::white));
    rectangle.setAngle(0.0);
    rectangle.setRenderType(ShadowedRectangle::Auto);

    QCOMPARE(radiusSpy.count(), 0);
    QCOMPARE(colorSpy.count(), 0);
    QCOMPARE(pureColorSpy.count(), 0);
    QCOMPARE(startColorSpy.count(), 0);
    QCOMPARE(endColorSpy.count(), 0);
    QCOMPARE(angleSpy.count(), 0);
    QCOMPARE(renderTypeSpy.count(), 0);

    rectangle.setRadius(8.0);
    rectangle.setColor(QColor(Qt::green));
    rectangle.setPureColor(false);
    rectangle.setStartColor(QColor(Qt::yellow));
    rectangle.setEndColor(QColor(Qt::cyan));
    rectangle.setAngle(90.0);
    rectangle.setRenderType(ShadowedRectangle::Software);

    QCOMPARE(rectangle.radius(), 8.0);
    QCOMPARE(rectangle.color(), QColor(Qt::green));
    QVERIFY(!rectangle.pureColor());
    QCOMPARE(rectangle.startColor(), QColor(Qt::yellow));
    QCOMPARE(rectangle.endColor(), QColor(Qt::cyan));
    QVERIFY(!qFuzzyIsNull(rectangle.angle()));
    QCOMPARE(rectangle.renderType(), ShadowedRectangle::Software);
    QVERIFY(rectangle.isSoftwareRendering());

    QCOMPARE(radiusSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(pureColorSpy.count(), 1);
    QCOMPARE(startColorSpy.count(), 1);
    QCOMPARE(endColorSpy.count(), 1);
    QCOMPARE(angleSpy.count(), 1);
    QCOMPARE(renderTypeSpy.count(), 1);

    rectangle.setRenderType(ShadowedRectangle::Auto);
    QCOMPARE(renderTypeSpy.count(), 2);
    QVERIFY(!rectangle.isSoftwareRendering());
}

void shadowed_rectangle_test::softwareRenderTypeCreatesFallbackItem()
{
    TestableShadowedRectangle rectangle;
    rectangle.setWidth(80.0);
    rectangle.setHeight(30.0);
    rectangle.setRenderType(ShadowedRectangle::Software);

    QVERIFY(rectangle.softwareItem() == nullptr);
    QVERIFY(rectangle.flags().testFlag(QQuickItem::ItemHasContents));

    rectangle.componentComplete();

    PaintedRectangleItem *softwareItem = rectangle.softwareItem();
    QVERIFY(softwareItem != nullptr);
    QCOMPARE(softwareItem->parentItem(), &rectangle);
    QCOMPARE(rectangle.childItems().size(), 1);
    QCOMPARE(rectangle.childItems().constFirst(), static_cast<QQuickItem *>(softwareItem));
    QCOMPARE(softwareItem->z(), -99.0);
    QCOMPARE(softwareItem->width(), 80.0);
    QCOMPARE(softwareItem->height(), 30.0);
    QVERIFY(!rectangle.flags().testFlag(QQuickItem::ItemHasContents));

    rectangle.setWidth(120.0);
    rectangle.setHeight(45.0);
    QCOMPARE(softwareItem->width(), 120.0);
    QCOMPARE(softwareItem->height(), 45.0);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    shadowed_rectangle_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-shadowed-rectangle.moc"
