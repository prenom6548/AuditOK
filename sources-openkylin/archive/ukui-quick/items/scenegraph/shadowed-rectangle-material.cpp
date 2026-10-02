/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "shadowed-rectangle-material.h"

#include <QOpenGLContext>

QSGMaterialType ShadowedRectangleMaterial::staticType;

ShadowedRectangleMaterial::ShadowedRectangleMaterial()
{
    setFlag(QSGMaterial::Blending, true);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QSGMaterialShader *ShadowedRectangleMaterial::createShader() const
#else
QSGMaterialShader *ShadowedRectangleMaterial::createShader(QSGRendererInterface::RenderMode) const
#endif
{
    return new ShadowedRectangleShader{shaderType};
}

QSGMaterialType *ShadowedRectangleMaterial::type() const
{
    return &staticType;
}

int ShadowedRectangleMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const ShadowedRectangleMaterial *>(other);
    /* clang-format off */
    if (material->color == color
        && material->shadowColor == shadowColor
        && material->offset == offset
        && material->aspect == aspect
        && material->pureColor == pureColor
        && material->startColor == startColor
        && material->endColor == endColor
        && material->angle == angle
        && qFuzzyCompare(material->size, size)
        && qFuzzyCompare(material->radius, radius)) { /* clang-format on */
        return 0;
    }

    return QSGMaterial::compare(other);
}

ShadowedRectangleShader::ShadowedRectangleShader(ShadowedRectangleMaterial::ShaderType shaderType)
{
    setShader(shaderType, QStringLiteral("shadowedrectangle"));
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
const char *const *ShadowedRectangleShader::attributeNames() const
{
    static char const *const names[] = {"in_vertex", "in_uv", nullptr};
    return names;
}

void ShadowedRectangleShader::initialize()
{
    QSGMaterialShader::initialize();
    m_matrixLocation = program()->uniformLocation("matrix");
    m_aspectLocation = program()->uniformLocation("aspect");
    m_opacityLocation = program()->uniformLocation("opacity");
    m_sizeLocation = program()->uniformLocation("size");
    m_radiusLocation = program()->uniformLocation("radius");
    m_colorLocation = program()->uniformLocation("color");
    m_shadowColorLocation = program()->uniformLocation("shadowColor");
    m_offsetLocation = program()->uniformLocation("offset");
    m_pureColorLocation = program()->uniformLocation("pureColor");
    m_startColorLocation = program()->uniformLocation("startColor");
    m_endColorLocation = program()->uniformLocation("endColor");
    m_angleLocation = program()->uniformLocation("angle");
}

void ShadowedRectangleShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    auto p = program();

    if (state.isMatrixDirty()) {
        p->setUniformValue(m_matrixLocation, state.combinedMatrix());
    }

    if (state.isOpacityDirty()) {
        p->setUniformValue(m_opacityLocation, state.opacity());
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0 || state.isCachedMaterialDataDirty()) {
        auto material = static_cast<ShadowedRectangleMaterial *>(newMaterial);
        p->setUniformValue(m_aspectLocation, material->aspect);
        p->setUniformValue(m_sizeLocation, material->size);
        p->setUniformValue(m_radiusLocation, material->radius);
        p->setUniformValue(m_colorLocation, material->color);
        p->setUniformValue(m_shadowColorLocation, material->shadowColor);
        p->setUniformValue(m_offsetLocation, material->offset);
        p->setUniformValue(m_pureColorLocation, material->pureColor);
        p->setUniformValue(m_startColorLocation, material->startColor);
        p->setUniformValue(m_endColorLocation, material->endColor);
        p->setUniformValue(m_angleLocation, material->angle);
    }
}
#else
bool ShadowedRectangleShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= 196);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 176, &opacity, 4);
        changed = true;
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0) {
        const auto material = static_cast<ShadowedRectangleMaterial *>(newMaterial);
        memcpy(buf->data() + 160, &material->aspect, 8);
        memcpy(buf->data() + 180, &material->size, 4);
        memcpy(buf->data() + 64, &material->radius, 16);
        float c[4];
        material->color.getRgbF(&c[0], &c[1], &c[2], &c[3]);
        memcpy(buf->data() + 80, c, 16);
        material->shadowColor.getRgbF(&c[0], &c[1], &c[2], &c[3]);
        memcpy(buf->data() + 96, c, 16);
        memcpy(buf->data() + 168, &material->offset, 8);
        const bool pureColor = material->pureColor;
        memcpy(buf->data() + 192, &pureColor, 4);
        material->startColor.getRgbF(&c[0], &c[1], &c[2], &c[   3]);
        memcpy(buf->data() + 112, c, 16);
        material->endColor.getRgbF(&c[0], &c[1], &c[2], &c[3]);
        memcpy(buf->data() + 128, c, 16);
        memcpy(buf->data() + 184, &material->angle, 4);
        changed = true;
    }

    return changed;
}
#endif
void ShadowedRectangleShader::setShader(ShadowedRectangleMaterial::ShaderType shaderType, const QString &shader)
{
    const auto shaderRoot = QStringLiteral(":/org/ukui/quick/shaders/");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto header = QOpenGLContext::currentContext()->isOpenGLES() ? QStringLiteral("header_es.glsl") : QStringLiteral("header_desktop.glsl");

    setShaderSourceFiles(QOpenGLShader::Vertex, {shaderRoot + header, shaderRoot + QStringLiteral("shadowedrectangle.vert")});

    QString shaderFile = shader + QStringLiteral(".frag");
    auto sdfFile = QStringLiteral("sdf.glsl");
    if (shaderType == ShadowedRectangleMaterial::ShaderType::LowPower) {
        shaderFile = shader + QStringLiteral("_lowpower.frag");
        sdfFile = QStringLiteral("sdf_lowpower.glsl");
    }

    setShaderSourceFiles(QOpenGLShader::Fragment, {shaderRoot + header, shaderRoot + sdfFile, shaderRoot + shaderFile});
#else
    setShaderFileName(QSGMaterialShader::VertexStage, shaderRoot + QStringLiteral("shadowedrectangle.vert.qsb"));

    auto shaderFile = shader;
    if (shaderType == ShadowedRectangleMaterial::ShaderType::LowPower) {
        shaderFile += QStringLiteral("_lowpower");
    }
    setShaderFileName(QSGMaterialShader::FragmentStage, shaderRoot + shaderFile + QStringLiteral(".frag.qsb"));
#endif
}