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
#include <QtTest>

#include "shadowed-texture.h"
#include "scenegraph/shadowed-border-texture-material.h"
#include "scenegraph/shadowed-rectangle-node.h"
#include "scenegraph/shadowed-texture-material.h"
#include "scenegraph/shadowed-texture-node.h"

namespace {

class FakeTexture : public QSGTexture
{
public:
    int textureId() const override
    {
        return 1;
    }

    QSize textureSize() const override
    {
        return QSize(32, 32);
    }

    bool hasAlphaChannel() const override
    {
        return true;
    }

    bool hasMipmaps() const override
    {
        return false;
    }

    bool isAtlasTexture() const override
    {
        return atlasTexture;
    }

    QSGTexture *removedFromAtlas() const override
    {
        ++removedFromAtlasCalls;
        return atlasReplacement ? atlasReplacement : const_cast<FakeTexture *>(this);
    }

    void bind() override
    {
        ++bindCalls;
    }

    bool atlasTexture = false;
    mutable int removedFromAtlasCalls = 0;
    int bindCalls = 0;
    QSGTexture *atlasReplacement = nullptr;
};

class FakeDynamicTexture : public QSGDynamicTexture
{
public:
    int textureId() const override
    {
        return 2;
    }

    QSize textureSize() const override
    {
        return QSize(64, 64);
    }

    bool hasAlphaChannel() const override
    {
        return true;
    }

    bool hasMipmaps() const override
    {
        return false;
    }

    void bind() override
    {
        ++bindCalls;
    }

    bool updateTexture() override
    {
        ++updateCalls;
        return true;
    }

    int updateCalls = 0;
    int bindCalls = 0;
};

class FakeTextureProvider : public QSGTextureProvider
{
public:
    QSGTexture *texture() const override
    {
        return currentTexture;
    }

    void setTexture(QSGTexture *texture)
    {
        currentTexture = texture;
    }

    void emitTextureChanged()
    {
        Q_EMIT textureChanged();
    }

    QSGTexture *currentTexture = nullptr;
};

class FakeTextureItem : public QQuickItem
{
public:
    bool isTextureProvider() const override
    {
        return true;
    }

    QSGTextureProvider *textureProvider() const override
    {
        return const_cast<FakeTextureProvider *>(&provider);
    }

    mutable FakeTextureProvider provider;
};

class ExposedShadowedTextureNode : public ShadowedTextureNode
{
public:
    ShadowedRectangleMaterial *material() const
    {
        return m_material;
    }
};

class ExposedShadowedTextureShader : public ShadowedTextureShader
{
public:
    using ShadowedTextureShader::ShadowedTextureShader;
    using ShadowedTextureShader::compile;
    using ShadowedTextureShader::initialize;
    using ShadowedTextureShader::updateState;
};

class ExposedShadowedBorderTextureShader : public ShadowedBorderTextureShader
{
public:
    using ShadowedBorderTextureShader::ShadowedBorderTextureShader;
    using ShadowedBorderTextureShader::compile;
    using ShadowedBorderTextureShader::initialize;
    using ShadowedBorderTextureShader::updateState;
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

class TestableShadowedTexture : public ShadowedTexture
{
public:
    using ShadowedTexture::ShadowedTexture;
    using ShadowedTexture::updatePaintNode;
};

}

class shadowed_texture_test : public QObject
{
    Q_OBJECT

private slots:
    void sourceSetterParentsAndSignals();
    void updatePaintNodeCreatesExpectedNodeTypes();
    void textureMaterialsCompareOnTextureSource();
    void borderTextureMaterialsCompareOnTextureSource();
    void textureNodePreprocessUsesDirectTextureAndDynamicUpdates();
    void textureNodePreprocessRemovesAtlasTexture();
    void textureNodeReconnectsWhenSourceChanges();
    void textureShadersInitializeOffscreenAndBindTextures();
};

void shadowed_texture_test::sourceSetterParentsAndSignals()
{
    TestableShadowedTexture item;
    FakeTextureItem source;
    QSignalSpy sourceSpy(&item, &ShadowedTexture::sourceChanged);

    QVERIFY(item.source() == nullptr);

    item.setSource(nullptr);
    QCOMPARE(sourceSpy.count(), 0);

    item.setSource(&source);
    QCOMPARE(item.source(), static_cast<QQuickItem *>(&source));
    QCOMPARE(source.parentItem(), static_cast<QQuickItem *>(&item));
    QCOMPARE(sourceSpy.count(), 1);

    item.setSource(&source);
    QCOMPARE(sourceSpy.count(), 1);

    item.setSource(nullptr);
    QVERIFY(item.source() == nullptr);
    QCOMPARE(sourceSpy.count(), 2);
}

void shadowed_texture_test::updatePaintNodeCreatesExpectedNodeTypes()
{
    TestableShadowedTexture item;
    FakeTextureItem source;

    item.setWidth(60);
    item.setHeight(30);
    item.shadow()->setSize(8.0);
    item.shadow()->setYOffset(2.0);
    item.border()->setWidth(1.0);

    QSGNode *node = item.updatePaintNode(nullptr, nullptr);
    QVERIFY(dynamic_cast<ShadowedRectangleNode *>(node) != nullptr);
    QVERIFY(dynamic_cast<ShadowedTextureNode *>(node) == nullptr);

    item.setSource(&source);
    node = item.updatePaintNode(node, nullptr);
    QVERIFY(dynamic_cast<ShadowedTextureNode *>(node) != nullptr);

    QSGNode *sameNode = item.updatePaintNode(node, nullptr);
    QCOMPARE(sameNode, node);

    delete sameNode;
}

void shadowed_texture_test::textureMaterialsCompareOnTextureSource()
{
    ShadowedTextureMaterial first;
    ShadowedTextureMaterial same;
    ShadowedTextureMaterial differentTexture;
    ShadowedTextureMaterial differentBaseState;
    FakeTexture firstTexture;
    FakeTexture secondTexture;

    QCOMPARE(first.type(), &ShadowedTextureMaterial::staticType);

    first.textureSource = &firstTexture;
    same.textureSource = &firstTexture;
    QCOMPARE(first.compare(&same), 0);

    differentTexture.textureSource = &secondTexture;
    QVERIFY(first.compare(&differentTexture) != 0);

    differentBaseState.textureSource = &firstTexture;
    differentBaseState.color = QColor(Qt::red);
    QVERIFY(first.compare(&differentBaseState) != 0);
}

void shadowed_texture_test::borderTextureMaterialsCompareOnTextureSource()
{
    ShadowedBorderTextureMaterial first;
    ShadowedBorderTextureMaterial same;
    ShadowedBorderTextureMaterial differentTexture;
    ShadowedBorderTextureMaterial differentBaseState;
    FakeTexture firstTexture;
    FakeTexture secondTexture;

    QCOMPARE(first.type(), &ShadowedBorderTextureMaterial::staticType);

    first.textureSource = &firstTexture;
    same.textureSource = &firstTexture;
    QCOMPARE(first.compare(&same), 0);

    differentTexture.textureSource = &secondTexture;
    QVERIFY(first.compare(&differentTexture) != 0);

    differentBaseState.textureSource = &firstTexture;
    differentBaseState.borderWidth = 0.25f;
    QVERIFY(first.compare(&differentBaseState) != 0);
}

void shadowed_texture_test::textureNodePreprocessUsesDirectTextureAndDynamicUpdates()
{
    ExposedShadowedTextureNode node;
    FakeTextureProvider provider;
    FakeDynamicTexture texture;

    provider.setTexture(&texture);

    node.setBorderEnabled(false);
    node.setTextureSource(&provider);
    node.preprocess();

    auto *borderlessMaterial = static_cast<ShadowedTextureMaterial *>(node.material());
    QCOMPARE(borderlessMaterial->type(), &ShadowedTextureMaterial::staticType);
    QCOMPARE(borderlessMaterial->textureSource, static_cast<QSGTexture *>(&texture));
    QCOMPARE(texture.updateCalls, 1);

    node.setBorderEnabled(true);
    node.preprocess();

    auto *borderMaterial = static_cast<ShadowedBorderTextureMaterial *>(node.material());
    QCOMPARE(borderMaterial->type(), &ShadowedBorderTextureMaterial::staticType);
    QCOMPARE(borderMaterial->textureSource, static_cast<QSGTexture *>(&texture));
    QCOMPARE(texture.updateCalls, 2);
}

void shadowed_texture_test::textureNodePreprocessRemovesAtlasTexture()
{
    ExposedShadowedTextureNode node;
    FakeTextureProvider provider;
    FakeTexture atlasTexture;
    FakeDynamicTexture atlasReplacement;

    atlasTexture.atlasTexture = true;
    atlasTexture.atlasReplacement = &atlasReplacement;
    provider.setTexture(&atlasTexture);

    node.setBorderEnabled(false);
    node.setTextureSource(&provider);
    node.preprocess();

    auto *material = static_cast<ShadowedTextureMaterial *>(node.material());
    QCOMPARE(material->textureSource, static_cast<QSGTexture *>(&atlasReplacement));
    QCOMPARE(atlasTexture.removedFromAtlasCalls, 1);
    QCOMPARE(atlasReplacement.updateCalls, 1);
}

void shadowed_texture_test::textureNodeReconnectsWhenSourceChanges()
{
    ExposedShadowedTextureNode node;
    FakeTextureProvider firstProvider;
    FakeTextureProvider secondProvider;
    FakeTexture firstTexture;
    FakeTexture secondTexture;

    firstProvider.setTexture(&firstTexture);
    secondProvider.setTexture(&secondTexture);

    node.setBorderEnabled(false);
    node.setTextureSource(&firstProvider);
    firstProvider.emitTextureChanged();

    node.setTextureSource(&secondProvider);
    secondProvider.emitTextureChanged();
    node.preprocess();

    auto *material = static_cast<ShadowedTextureMaterial *>(node.material());
    QCOMPARE(material->textureSource, static_cast<QSGTexture *>(&secondTexture));

    node.setTextureSource(nullptr);
}

void shadowed_texture_test::textureShadersInitializeOffscreenAndBindTextures()
{
    QOffscreenSurface surface;
    QOpenGLContext context;
    if (!makeCurrentOffscreenContext(surface, context)) {
        QSKIP("离屏 OpenGL 上下文不可用");
    }

    FakeDynamicTexture texture;
    ShadowedTextureMaterial material;
    material.textureSource = &texture;

    auto *createdShader = material.createShader();
    QVERIFY(createdShader != nullptr);
    delete createdShader;

    ExposedShadowedTextureShader shader(ShadowedRectangleMaterial::ShaderType::Standard);
    shader.compile();
    shader.initialize();

    QSGMaterialShader::RenderState state{};
    shader.updateState(state, &material, &material);
    QCOMPARE(texture.bindCalls, 1);

    ShadowedBorderTextureMaterial borderMaterial;
    borderMaterial.textureSource = &texture;

    auto *createdBorderShader = borderMaterial.createShader();
    QVERIFY(createdBorderShader != nullptr);
    delete createdBorderShader;

    ExposedShadowedBorderTextureShader borderShader(ShadowedRectangleMaterial::ShaderType::LowPower);
    borderShader.compile();
    borderShader.initialize();
    borderShader.updateState(state, &borderMaterial, &borderMaterial);
    QCOMPARE(texture.bindCalls, 2);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    shadowed_texture_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-shadowed-texture.moc"
