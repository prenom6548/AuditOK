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

#include "ubreffect.h"
#include "shaderhelper.h"
#include "ubrtexturehelper.h"
#include "wayland/surface_interface.h"
#include "xatom-helper.h"

#include <kwineffects.h>
#include <kwinglplatform.h>
#include <kwinglutils.h>

#include <QVector4D>

#include <KConfigGroup>

#define UNITY_BORDER_RADIUS 12

#define MIN_BORDER_RADIUS 2 // 圆角小于2时，按0处理

typedef void (*ToplevelSetDepth)(void *, int);

static ToplevelSetDepth setDepthfunc = nullptr;

UBREffect::UBREffect()
{
    // try get Toplevel::setDepth(), for resolving ubr effect has black corners of opaque window.
    if (qApp->applicationDirPath().startsWith("/usr")) {
        setDepthfunc = (ToplevelSetDepth)QLibrary::resolve("kwin.so." + qApp->applicationVersion(), "_ZN4KWin8Toplevel8setDepthEi");
    } else {
        setDepthfunc = (ToplevelSetDepth)QLibrary::resolve(qApp->applicationDirPath() + QLatin1String("kwin.so"), "_ZN4KWin8Toplevel8setDepthEi");
    }

    m_ubrShader = ShaderHelper::getShader();
    if (!m_ubrShader->isValid()) {
        m_ubrShader = KWin::ShaderManager::instance()->generateCustomShader(KWin::ShaderTrait::MapTexture | KWin::ShaderTrait::Modulate | KWin::ShaderTrait::AdjustSaturation);
    }

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        m_settings.reset(new QGSettings("org.ukui.style"));
        if (m_settings) {
            int radius = m_settings->get("window-radius").toInt();
            windowRadiusUpdate(radius);
            connect(m_settings.get(), &QGSettings::changed, this, [this](const QString &key) {
                if (key == "windowRadius") {
                    int radius = m_settings->get("window-radius").toInt();
                    windowRadiusUpdate(radius);
                }
            });
        }
    }

    if (QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.xsettings")) {
        m_scaleSettings.reset(new QGSettings("org.ukui.SettingsDaemon.plugins.xsettings"));
        if (m_scaleSettings) {
            const double scale = m_scaleSettings->get("scaling-factor").toDouble();
            m_scale = scale >= 1.0 ? scale : 1.0;
            connect(m_scaleSettings.get(), &QGSettings::changed, this, [this](const QString &key) {
                if (key == "scalingFactor") {
                    const double scale = m_scaleSettings->get("scaling-factor").toDouble();
                    m_scale = scale >= 1.0 ? scale : 1.0;
                }
            });
        }
    }

    auto effectsManager = KWin::effects;
    for (auto window : effectsManager->stackingOrder()) {
        initEffectWindow(window);
    }

    connect(effectsManager, &KWin::EffectsHandler::windowAdded, this, [=](KWin::EffectWindow *window) {
        initEffectWindow(window);
    });

    connect(effectsManager, &KWin::EffectsHandler::windowDeleted, this, [=](KWin::EffectWindow *window) {
        window->setData(UnityBorderRadius, QVariant());
        m_windowsQuickTileMode.remove(window);
    });
}

UBREffect::~UBREffect()
{
    UBRTextureHelper::getInstance()->release();
}

bool UBREffect::supported()
{
    bool supported = KWin::effects->isOpenGLCompositing() && KWin::GLRenderTarget::supported();
    return supported;
}

bool UBREffect::enabledByDefault()
{
    return true;
}

void UBREffect::prePaintScreen(KWin::ScreenPrePaintData &data, std::chrono::milliseconds time)
{
    KWin::Effect::prePaintScreen(data, time);
}

void UBREffect::prePaintWindow(KWin::EffectWindow *w, KWin::WindowPrePaintData &data, std::chrono::milliseconds time)
{
    bool needRepaint = false;
    bool isContain = m_windowsQuickTileMode.contains(w);
    u_int32_t newMode = w->quickTileMode();
    u_int32_t oldMode = isContain ? m_windowsQuickTileMode[w] : newMode;

    if (isContain) {
        if (oldMode != newMode) {
            m_windowsQuickTileMode[w] = newMode;
            needRepaint = true;
        }
    } else {
        m_windowsQuickTileMode.insert(w, newMode);
    }

    int radius = w->windowRadius() >= 0 ? w->windowRadius() : m_radius;
    KWin::Effect::prePaintWindow(w, data, time);
    QRegion newClip;
    const QRegion oldClip = data.clip;
    for (const QRect &rect : oldClip) {
        newClip |= rect.adjusted(radius, radius, -radius, -radius);
    }
    data.clip = newClip;

    if (needRepaint) {
        KWin::effects->addRepaintFull();
    }
}

bool UBREffect::shouldUBR(KWin::EffectWindow *w, int mask)
{
    if (m_radius < MIN_BORDER_RADIUS || (!KWin::effects->isOpenGLCompositing()) || (!w->isPaintingEnabled() || (mask & PAINT_WINDOW_LANCZOS))
        || w->quickTileMode() || w->noBorder() || !w->isManaged()) {
        return false;
    }
    return true;
}

void UBREffect::drawWindow(KWin::EffectWindow *w, int mask, const QRegion &region, KWin::WindowPaintData &data)
{
    if (!shouldUBR(w, mask))
        return KWin::Effect::drawWindow(w, mask, region, data);

    if (!w->hasAlpha()) {
        if (setDepthfunc) {
            setDepthfunc(w->parent(), 32);
        }
    }

    auto ubr = qvariant_cast<QVector4D>(w->data(UnityBorderRadius));

    KWin::WindowPaintData tmpData = data;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

    bool showTopUBR = !w->hasDecoration();

    auto textureTopLeft = UBRTextureHelper::getInstance()->getTexture(showTopUBR ? ubr.x() : 0);
    glActiveTexture(GL_TEXTURE10);
    textureTopLeft->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureTopRight = UBRTextureHelper::getInstance()->getTexture(showTopUBR ? ubr.y() : 0);
    glActiveTexture(GL_TEXTURE11);
    textureTopRight->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureBottomLeft = UBRTextureHelper::getInstance()->getTexture(ubr.z());
    glActiveTexture(GL_TEXTURE12);
    textureBottomLeft->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    auto textureBottomRight = UBRTextureHelper::getInstance()->getTexture(ubr.w());
    glActiveTexture(GL_TEXTURE13);
    textureBottomRight->bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glActiveTexture(GL_TEXTURE0);

    tmpData.shader = m_ubrShader;
    KWin::ShaderManager::instance()->pushShader(m_ubrShader);

    m_ubrShader->setUniform("topleft", 10);
    m_ubrShader->setUniform("scale", QVector2D(w->width() * 1.0 / textureTopLeft->width(), w->height() * 1.0 / textureTopLeft->height()));

    m_ubrShader->setUniform("topright", 11);
    m_ubrShader->setUniform("scale1", QVector2D(w->width() * 1.0 / textureTopRight->width(), w->height() * 1.0 / textureTopRight->height()));

    m_ubrShader->setUniform("bottomleft", 12);
    m_ubrShader->setUniform("scale2", QVector2D(w->width() * 1.0 / textureBottomLeft->width(), w->height() * 1.0 / textureBottomLeft->height()));

    m_ubrShader->setUniform("bottomright", 13);
    m_ubrShader->setUniform("scale3", QVector2D(w->width() * 1.0 / textureBottomRight->width(), w->height() * 1.0 / textureBottomRight->height()));

    KWin::Effect::drawWindow(w, mask, region, tmpData);

    KWin::ShaderManager::instance()->popShader();

    glActiveTexture(GL_TEXTURE10);
    textureTopLeft->unbind();

    glActiveTexture(GL_TEXTURE11);
    textureTopRight->unbind();
    glActiveTexture(GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE12);
    textureBottomLeft->unbind();
    glActiveTexture(GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE13);
    textureBottomRight->unbind();
    glActiveTexture(GL_TEXTURE0);

    glDisable(GL_BLEND);
}

void UBREffect::windowRadiusUpdate(int radius)
{
    if (m_radius == radius) {
        return;
    }

    m_radius = radius;
    auto effectsManager = KWin::effects;

    for (auto window : effectsManager->stackingOrder()) {
        initEffectWindow(window);
    }
}

void UBREffect::initEffectWindow(KWin::EffectWindow *window)
{
    if (!window->surface()) {
        auto ubr = XAtomHelper::getInstance()->getWindowBorderRadius(window);
        if (ubr.topLeft <= 0) {
            ubr.topLeft = (m_radius < MIN_BORDER_RADIUS) ? 0 : m_radius * m_scale;
        }
        if (ubr.topRight <= 0) {
            ubr.topRight = (m_radius < MIN_BORDER_RADIUS) ? 0 : m_radius * m_scale;
        }
        if (ubr.bottomLeft <= 0) {
            ubr.bottomLeft = (m_radius < MIN_BORDER_RADIUS) ? 0 : m_radius * m_scale;
        }
        if (ubr.bottomRight <= 0) {
            ubr.bottomRight = (m_radius < MIN_BORDER_RADIUS) ? 0 : m_radius * m_scale;
        }
        window->setData(UnityBorderRadius, QVector4D(ubr.topLeft, ubr.topRight, ubr.bottomLeft, ubr.bottomRight));
    } else {
        // init on wayland
        int radius = window->windowRadius() >= 0 ? window->windowRadius() : m_radius;
        QVector4D ubr = qvariant_cast<QVector4D>(window->surface()->property("ubr"));
        if (ubr.x() <= 0) {
            ubr.setX((radius < MIN_BORDER_RADIUS) ? 0 : radius);
        }
        if (ubr.y() <= 0) {
            ubr.setY((radius < MIN_BORDER_RADIUS) ? 0 : radius);
        }
        if (ubr.z() <= 0) {
            ubr.setZ((radius < MIN_BORDER_RADIUS) ? 0 : radius);
        }
        if (ubr.w() <= 0) {
            ubr.setW((radius < MIN_BORDER_RADIUS) ? 0 : radius);
        }
        window->setData(UnityBorderRadius, ubr);
    }
}
