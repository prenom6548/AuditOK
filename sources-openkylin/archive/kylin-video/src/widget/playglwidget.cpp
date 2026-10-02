/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "playglwidget.h"
#include <QOpenGLContext>
#include <QApplication>
#include <QMouseEvent>
#include <QGSettings>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <ukui-log4qt.h>

#include "global/global.h"

using namespace Global;

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

PlayGLWidget::PlayGLWidget(QWidget *parent, double s)
    : QOpenGLWidget(parent),
      m_screenScale(s),
      m_render_context(nullptr)
{
    setlocale(LC_NUMERIC, "C");
    setMouseTracking(true);

    m_dbClicked = false;
    m_mouseUsed = true;

    initGlobalSig();

    m_checkMouseTimer = new QTimer;
    m_checkMouseTimer->setObjectName("kylin-video_QTimer_m_checkMouseTimer");
    m_checkMouseTimer->setInterval(2000);
    connect(m_checkMouseTimer, &QTimer::timeout, [&](){
        if(!m_hasVideo)
            return;
        if(m_isMouseEnter) {
            if (m_mouseUsed)
                setCursor(QCursor(Qt::BlankCursor));
        }
        g_user_signal->hideBar(true);

        m_checkMouseTimer->stop();
    });

    m_lMouseClickTimer = new QTimer;
    m_lMouseClickTimer->setObjectName("kylin-video_QTimer_m_lMouseClickTimer");
    m_lMouseClickTimer->setInterval(300);
    connect(m_lMouseClickTimer, &QTimer::timeout, [&](){
        if (m_dbClicked) {
            m_lMouseClickTimer->stop();
            return;
        }
        emit mousePressed();
        m_lMouseClickTimer->stop();
    });
}

PlayGLWidget::~PlayGLWidget()
{
    // 退出时释放新的 render_context
    if (m_render_context) {
        MpvLibLoad::getInstance()->mpv_render_context_free(m_render_context);
        m_render_context = nullptr;
    }
}

void PlayGLWidget::swapped()
{
    if (m_render_context) {
        MpvLibLoad::getInstance()->mpv_render_context_report_swap(m_render_context);
    }
}

void PlayGLWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((PlayGLWidget*)ctx, "update");
}

void PlayGLWidget::initMpvGL()
{
    if(mpv_h)
    {
        // 绑定 Qt 的帧交换信号。核心的 OpenGL 初始化后移到 initializeGL() 保证 OpenGL context 已就绪
        connect(this, &PlayGLWidget::frameSwapped, this, &PlayGLWidget::swapped, Qt::UniqueConnection);
    }
}

void PlayGLWidget::initGlobalSig()
{
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, [&](){
        if (g_playstate == Mpv::Playing) {
            m_screenScale = qApp->devicePixelRatio();
        }
        else if (g_playstate == Mpv::Stopped) {
            update();
        }
    });

    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, this, &PlayGLWidget::playStateChange);
    connect(g_core_signal, &GlobalCoreSignal::sigVideoIdChange, this, &PlayGLWidget::videIdChange);
}

void PlayGLWidget::initializeGL()
{
    // 如果 mpv_h 存在，且还未创建过渲染上下文
    if(mpv_h && !m_render_context)
    {
        mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_API_TYPE, (void *)MPV_RENDER_API_TYPE_OPENGL},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID, nullptr} // 必须以 INVALID 结尾
        };

        // 创建新的 render_context
        int r = MpvLibLoad::getInstance()->mpv_render_context_create(&m_render_context, mpv_h, params);
        if (r < 0)
        {
            KyInfo("could not initialize mpv OpenGL render context");
            return;
        }
        // 设置画面更新回调
        MpvLibLoad::getInstance()->mpv_render_context_set_update_callback(m_render_context, PlayGLWidget::on_update, (void *)this);
    }
}

void PlayGLWidget::paintGL()
{
    if(m_render_context)
    {
        int w = this->width() * devicePixelRatio();
        int h = this->height() * devicePixelRatio();
        int fbo_id = this->defaultFramebufferObject();
        int flip_y = 1; // 1 表示翻转，代替之前传负数 height 的骚操作

        // 构建新的 fbo 结构体
        mpv_opengl_fbo fbo{fbo_id, w, h, 0};

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        // 执行画面渲染
        MpvLibLoad::getInstance()->mpv_render_context_render(m_render_context, params);
    }
}

void PlayGLWidget::setMouseUsed(bool used)
{
    m_mouseUsed = used;
    if (!m_mouseUsed) {
        setCursor(QCursor(Qt::ArrowCursor));
        m_checkMouseTimer->stop();
    }
}

void PlayGLWidget::playStateChange()
{
    if(g_playstate < 0) {
        m_checkMouseTimer->stop();
    }
    else {
        m_checkMouseTimer->start();
    }
}

void PlayGLWidget::videIdChange(int vid)
{
    if(vid < 0)
        m_hasVideo = false;
    else
        m_hasVideo = true;
}

#ifndef WHITE_TEST
void PlayGLWidget::mousePressEvent(QMouseEvent *e)
{
    m_isMousePressed = true;
    m_mousePosPressed = e->pos();
    m_posStart = e->pos();
    m_posLast = e->pos();

    // 右键点击触发显示菜单
    if (e->button() == Qt::LeftButton) {
        m_timePressStart = QDateTime::currentMSecsSinceEpoch();
    }
    return QOpenGLWidget::mousePressEvent(e);
}

void PlayGLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_isMousePressed = false;
    // 左键离开触发暂停，如果左键按得时间很长就不触发了，不然会和触摸屏长按冲突
    if (e->button() == Qt::LeftButton && (QDateTime::currentMSecsSinceEpoch() - m_timePressStart) < 500) {
        if (!m_mouseUsed)
            return QOpenGLWidget::mouseReleaseEvent(e);
            // 如果是双击的话，无需触发单击暂停逻辑
        if (e->pos() == m_mousePosPressed && !m_dbClicked) {
            m_lMouseClickTimer->start();
        }
    }
    return QOpenGLWidget::mouseReleaseEvent(e);
}

void PlayGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_isMousePressed) {
        m_posCurrent = e->pos();

        int xc = abs(m_posCurrent.x() - m_posLast.x());
        int yc = abs(m_posCurrent.y() - m_posLast.y());
        if (xc < yc) {
            m_volumeChange = yc * 200 / height();
            //符合音量调节
            if (m_volumeChange > 0) {
                if (m_posCurrent.y() - m_posLast.y() > 2) {
                    g_user_signal->setVolumeDown(m_volumeChange);
                    m_posLast = m_posCurrent;
                }
                else {
                    g_user_signal->setVolumeUp(m_volumeChange);
                    m_posLast = m_posCurrent;
                }
            }
        }
    }

    m_isMouseEnter = true;
    m_checkMouseTimer->stop();
    setCursor(QCursor(Qt::ArrowCursor));
    g_user_signal->hideBar(false);
    if(g_playstate > 0)
        m_checkMouseTimer->start();
    e->accept();
}

void PlayGLWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!m_mouseUsed)
        return QOpenGLWidget::mouseDoubleClickEvent(e);
    if(e->button() == Qt::LeftButton) {
        g_user_signal->fullScreen();
        m_dbClicked = true;
        QTimer::singleShot(500, [this]{m_dbClicked = false;});
        m_lMouseClickTimer->stop();
    }
    return QOpenGLWidget::mouseDoubleClickEvent(e);
}

void PlayGLWidget::wheelEvent(QWheelEvent *e)
{
    // 鼠标滚轮和触控板双指滑动调节音量
    int wheel_distance = e->delta();
    if (abs(wheel_distance) / 120 == 1) {
        if (wheel_distance > 0)
            g_user_signal->setVolumeUp(5);
        else
            g_user_signal->setVolumeDown(5);
    }
    return QOpenGLWidget::wheelEvent(e);
}

void PlayGLWidget::enterEvent(QEvent *e)
{
    m_isMouseEnter = true;
    e->accept();
}

void PlayGLWidget::leaveEvent(QEvent *e)
{
    m_isMousePressed = false;
    m_isMouseEnter = false;
    if(g_playstate > 0)
        m_checkMouseTimer->start();

    return QOpenGLWidget::leaveEvent(e);
}

#endif
