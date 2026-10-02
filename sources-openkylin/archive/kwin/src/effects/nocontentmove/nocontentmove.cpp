#include "nocontentmove.h"
// #include "nocontentmoveconfig.h"

#include <kwinglutils.h>
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <kwinxrenderutils.h>
#include <xcb/render.h>
#endif

#include <KConfigGroup>

#include <QPainter>

namespace KWin
{
NoContentMoveEffect::NoContentMoveEffect()
    : AnimationEffect()
    , m_active(false)
    , m_moveWindow(nullptr)
{
    LoadConfig();

    connect(effects, &EffectsHandler::windowStartUserMovedResized, this, &NoContentMoveEffect::slotWindowStartUserMovedResized);
    connect(effects, &EffectsHandler::windowStepUserMovedResized, this, &NoContentMoveEffect::slotWindowStepUserMovedResized);
    connect(effects, &EffectsHandler::windowFinishUserMovedResized, this, &NoContentMoveEffect::slotWindowFinishUserMovedResized);
}

NoContentMoveEffect::~NoContentMoveEffect()
{
}

void NoContentMoveEffect::LoadConfig()
{
    KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "NoContentMove");
    showMode = kwinConfig.readEntry("Mode", showMode);
    border_width = kwinConfig.readEntry("Border", border_width);
    float alpha_t = kwinConfig.readEntry("Alpha", alpha);
    this->alpha = alpha_t;
    const QString s = QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue());
    QString color_str = kwinConfig.readEntry("Color", s);
    if (!color_str.isEmpty()) {
        QStringList strs = color_str.split(',');
        if (strs.length() == 3) {
            color.setRed(strs[0].toInt());
            color.setGreen(strs[0].toInt());
            color.setBlue(strs[0].toInt());
        }
    }
}

bool NoContentMoveEffect::supported()
{
    return effects->isOpenGLCompositing();
}

bool NoContentMoveEffect::enabledByDefault()
{
    return true;
}

void NoContentMoveEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    if (m_active)
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
    AnimationEffect::prePaintScreen(data, presentTime);
}

void NoContentMoveEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime)
{
    if (m_active && w == m_moveWindow)
        data.mask |= PAINT_WINDOW_TRANSFORMED;
    AnimationEffect::prePaintWindow(w, data, presentTime);
}

// each frame painting
void NoContentMoveEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (m_active && w == m_moveWindow) {
        if (showMode & 0x01) {
            data += (m_originalGeometry.topLeft() - m_currentGeometry.topLeft());
            AnimationEffect::paintWindow(w, mask, region, data);
        }
        QRegion paintRegion = QRegion(m_currentGeometry);
        if (effects->isOpenGLCompositing()) {
            paintOpenGL(m_currentGeometry, data);
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
        } else if (effects->compositingType() == XRenderCompositing) {
            paintXrender(m_currentGeometry);
#endif
        }
    } else {
        effects->paintWindow(w, mask, region, data);
    }
}
// following 3 functions are to update the memory data of moved window
void NoContentMoveEffect::slotWindowStartUserMovedResized(EffectWindow *w)
{
    if (w->isUserMove() && !w->isUserResize()) {
        m_active = true;
        m_moveWindow = w;
        m_originalGeometry = w->frameGeometry();
        m_currentGeometry = w->frameGeometry();
        w->addRepaintFull();
    }
}

void NoContentMoveEffect::slotWindowFinishUserMovedResized(EffectWindow *w)
{
    if (m_active && w == m_moveWindow) {
        m_active = false;
        m_moveWindow = nullptr;
        effects->addRepaintFull();
    }
}

void NoContentMoveEffect::slotWindowStepUserMovedResized(EffectWindow *w, const QRect &geometry)
{
    if (m_active && w == m_moveWindow) {
        m_currentGeometry = geometry;
        effects->addRepaintFull();
    }
}

void NoContentMoveEffect::setColor(int red, int green, int blue)
{
    color.setRed(red);
    color.setGreen(green);
    color.setBlue(blue);
    const QString s = QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue());
    KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "NoContentMove");
    kwinConfig.writeEntry("Color", s);
    kwinConfig.sync();
}

void NoContentMoveEffect::setAlpha(float alpha)
{
    this->alpha = alpha;
    KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "NoContentMove");
    kwinConfig.writeEntry("Alpha", alpha);
    kwinConfig.sync();
}

void NoContentMoveEffect::setMode(int mode)
{
    showMode = mode;
    KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "NoContentMove");
    kwinConfig.writeEntry("Mode", showMode);
    kwinConfig.sync();
}

void NoContentMoveEffect::setBorderWith(int border)
{
    border_width = border;
    KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "NoContentMove");
    kwinConfig.writeEntry("Border", border_width);
    kwinConfig.sync();
}

void NoContentMoveEffect::paintOpenGL(QRect r, WindowPaintData &data)
{
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    ShaderBinder binder(ShaderTrait::UniformColor);
    binder.shader()->setUniform(GLShader::ModelViewProjectionMatrix, data.screenProjectionMatrix());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    QColor col = color;
    col.setAlphaF(alpha);
    vbo->setColor(col);

    if ((showMode & 0x02) == 0) {
        QVector<float> verts;
        verts.reserve(48); // OpenGL绘制的节点数，重复点重复计算
        // Left
        verts << r.x() + border_width << r.y();
        verts << r.x() << r.y();
        verts << r.x() + border_width << r.y() + r.height();
        verts << r.x() << r.y() + r.height();
        verts << r.x() << r.y();
        verts << r.x() + border_width << r.y() + r.height();
        // Top
        verts << r.x() + border_width << r.y();
        verts << r.x() + r.width() - border_width << r.y();
        verts << r.x() + r.width() - border_width << r.y() + border_width;
        verts << r.x() + border_width << r.y();
        verts << r.x() + r.width() - border_width << r.y() + border_width;
        verts << r.x() + border_width << r.y() + border_width;
        // Right
        verts << r.x() + r.width() - border_width << r.y();
        verts << r.x() + r.width() << r.y();
        verts << r.x() + r.width() << r.y() + r.height();
        verts << r.x() + r.width() - border_width << r.y();
        verts << r.x() + r.width() - border_width << r.y() + r.height();
        verts << r.x() + r.width() << r.y() + r.height();
        // Bottom
        verts << r.x() + border_width << r.y() + r.height() - border_width;
        verts << r.x() + r.width() - border_width << r.y() + r.height() - border_width;
        verts << r.x() + r.width() - border_width << r.y() + r.height();
        verts << r.x() + border_width << r.y() + r.height() - border_width;
        verts << r.x() + r.width() - border_width << r.y() + r.height();
        verts << r.x() + border_width << r.y() + r.height();
        vbo->setData(verts.count() / 2, 2, verts.data(), nullptr);
    } else {
        QVector<float> verts;
        verts.reserve(12); // OpenGL绘制的节点数，重复点重复计算

        verts << r.x() + r.width() << r.y();
        verts << r.x() << r.y();
        verts << r.x() + r.width() << r.y() + r.height();
        verts << r.x() << r.y() + r.height();
        verts << r.x() << r.y();
        verts << r.x() + r.width() << r.y() + r.height();
        vbo->setData(verts.count() / 2, 2, verts.data(), nullptr);
    }

    vbo->render(GL_TRIANGLES);
    glDisable(GL_BLEND);
}

}
