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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

#include <QDebug>
#include <QVariant>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glx.h>

#include <wayland-client.h>
#include <QtDBus/QDBusConnectionInterface>
#include "glinfo-query.h"

/***************************
*       EGLInfoQuery
**************************/
namespace UkuiQuick
{

class EGLInfoQuery
{
public:
    using EglPlatformDisplayFunc = PFNEGLGETPLATFORMDISPLAYEXTPROC;
    enum Platform
    {
        X11,
        Wayland
    };

    EGLInfoQuery(const QString &env, Platform platform = Platform::Wayland);
    ~EGLInfoQuery();
    QString glRenderDevice();
    bool isValid();
private:
    bool m_isValid = false;
    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    EGLContext m_eglContext = EGL_NO_CONTEXT;
    EglPlatformDisplayFunc m_eglPlatformDisplayFunc = nullptr;
};

EGLInfoQuery::EGLInfoQuery(const QString &env, Platform platform)
{
    void *nativeDisplay = nullptr;
    EGLenum eglPlatform = EGL_PLATFORM_WAYLAND_EXT;
    if (platform == Platform::Wayland) {
        nativeDisplay = reinterpret_cast<void *>(wl_display_connect(nullptr));
        eglPlatform = EGL_PLATFORM_WAYLAND_EXT;
    } else if (platform == Platform::X11) {
        nativeDisplay = reinterpret_cast<void *>(XOpenDisplay(nullptr));
        eglPlatform = EGL_PLATFORM_X11_EXT;
    }
    //获取egl display
    m_eglPlatformDisplayFunc = reinterpret_cast<EglPlatformDisplayFunc>(
        eglGetProcAddress(reinterpret_cast<const char *>("eglGetPlatformDisplayEXT")));
    if (nativeDisplay != EGL_DEFAULT_DISPLAY && m_eglPlatformDisplayFunc) {
        m_eglDisplay = m_eglPlatformDisplayFunc(eglPlatform, nativeDisplay, nullptr);
    } else {
        m_eglDisplay = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(nativeDisplay));
    }

    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qWarning() << "[" << env << "] : Get egl display failed .";
        return;
    }
    if (!eglInitialize(m_eglDisplay, nullptr, nullptr)) {
        qWarning() << "[" << env << "] : Init egl failed .";
        eglTerminate(m_eglDisplay);
        return;
    }

    // Bind the GL API (try desktop GL first)
    if (!eglBindAPI(EGL_OPENGL_API)) {
        qWarning() << "[" << env << "] : eglBindAPI(EGL_OPENGL_API) failed, trying EGL_OPENGL_ES_API .";
        if (!eglBindAPI(EGL_OPENGL_ES_API)) {
            qWarning() << "[" << env << "] : eglBindAPI(EGL_OPENGL_ES_API) also failed .";
            eglTerminate(m_eglDisplay);
            return;
        }
    }

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(m_eglDisplay, config_attribs, &config, 1, &num_configs) || num_configs < 1) {
        qWarning() << "[" << env << "] ： egl choose config failed .";
        eglTerminate(m_eglDisplay);
        return;
    }

    // Create context: try desktop GL 3.0+, then fallback to 2.0 / ES
    EGLint ctx_attribs_gl3[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE };
    EGLint ctx_attribs_gl2[] = { EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE };
    EGLint ctx_attribs_es2[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, ctx_attribs_gl3);
    if (m_eglContext == EGL_NO_CONTEXT) {
        m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, ctx_attribs_gl2);
    }
    if (m_eglContext == EGL_NO_CONTEXT) {
        m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, ctx_attribs_es2);
    }
    if (m_eglContext == EGL_NO_CONTEXT) {
        qWarning() << "[" << env << "] ： create egl conttext failed .";
        eglTerminate(m_eglDisplay);
        return;
    }

    if (!eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, m_eglContext)) {
        qWarning() << "[" << env << "] ： Active egl conttext failed .";
        eglDestroyContext(m_eglDisplay, m_eglContext);
        eglTerminate(m_eglDisplay);
        return ;
    }
    m_isValid = true;
}

EGLInfoQuery::~EGLInfoQuery()
{
    if (m_isValid) {
        eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_eglDisplay, m_eglContext);
        eglTerminate(m_eglDisplay);
    }
}

/*!
    获取当前的渲染设备
 */
QString EGLInfoQuery::glRenderDevice()
{
    return QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}
/*!
    表示 EGLInfoQuery 是否可以有效查询 renderer 信息。

    为 true 表示提供的是有效值。
 */
bool EGLInfoQuery::isValid()
{
    return m_isValid;
}

/***************************
*       GLXInfoQuery
**************************/

class GLXInfoQuery
{
public:
    // 函数指针， 用于查询对应glx 扩展对应属性的值。在准备好glx 上下文后调用有效
    using QueryInteger = PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC;

    GLXInfoQuery();
    ~GLXInfoQuery();

    QString glRenderDevice();
    QVariant isHardwareRendering();
    bool isValid();
private:
    void cleanup();

    bool m_isValid = false;
    QueryInteger m_queryInteger{nullptr};
    Display* m_display{nullptr};
    GLXContext m_glxContext{nullptr};
    GLXFBConfig* m_glxFbConfigs{nullptr};
    Window m_window{0};
};

GLXInfoQuery::GLXInfoQuery()
{
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        qWarning() << "Open display failed .";
        return;
    }

    // 选择FBConfig
    int fbAttribs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DOUBLEBUFFER, False,
        None
    };

    int numConfigs = 0;
    int screen = DefaultScreen(m_display);
    m_glxFbConfigs = glXChooseFBConfig(m_display, screen, fbAttribs, &numConfigs);
    if (!m_glxFbConfigs || numConfigs == 0) {
        qWarning() << "Get glx fb config failed .";
        cleanup();
        return;
    }

    // 创建上下文
    m_glxContext = glXCreateNewContext(m_display, m_glxFbConfigs[0], GLX_RGBA_TYPE, nullptr, True);
    if (!m_glxContext) {
        qWarning() << "Create glx context failed .";
        cleanup();
        return;
    }

    m_window = XCreateSimpleWindow(m_display, RootWindow(m_display, screen), 0, 0, 1, 1, 0, 0, 0);
    if (!m_window) {
        qWarning() << "Create x11 window failed .";
        cleanup();
        return;
    }

    // 激活上下文
    if (!glXMakeCurrent(m_display, m_window, m_glxContext)) {
        qWarning() << "Active glx context failed .";
        cleanup();
        return;
    }
    m_isValid = true;

#ifdef GLX_MESA_query_renderer
    m_queryInteger = reinterpret_cast<QueryInteger>(glXGetProcAddressARB(
        reinterpret_cast<const GLubyte *>("glXQueryCurrentRendererIntegerMESA")));
#endif

}

GLXInfoQuery::~GLXInfoQuery()
{
    cleanup();
}

void GLXInfoQuery::cleanup()
{
    if (!m_display) {
        return;
    }

    if (m_glxContext) {
        if (m_isValid) {
            glXMakeCurrent(m_display, None, nullptr);
        }
        glXDestroyContext(m_display, m_glxContext);
        m_glxContext = nullptr;
    }

    if (m_window) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
    }

    if (m_glxFbConfigs) {
        XFree(m_glxFbConfigs);
        m_glxFbConfigs = nullptr;
    }

    XSync(m_display, True);
    XCloseDisplay(m_display);
    m_display = nullptr;
    m_isValid = false;
}

/*!
    获取当前的渲染设备
 */
QString GLXInfoQuery::glRenderDevice()
{
    return QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}

/*!
    是否支持硬件渲染, 需要先检查是否有效，再使用返回值。
 */
QVariant GLXInfoQuery::isHardwareRendering()
{
    unsigned int isHardWareAccelerated = 0;
    if (m_queryInteger && m_queryInteger(GLX_RENDERER_ACCELERATED_MESA, &isHardWareAccelerated)) {
        return QVariant(static_cast<bool>(isHardWareAccelerated));
    }
    return QVariant(QVariant::Invalid);
}

/*!
    表示 GLXInfoQuery 是否可以有效查询 renderer 信息。

    为 true 表示提供的是有效值。
 */
bool GLXInfoQuery::isValid()
{
    return m_isValid;
}


/***************************
*       GLInfoQuery
**************************/

GLInfoQuery::GLInfoQuery()
{
    QByteArray env = qgetenv("XDG_SESSION_TYPE");
    if (env == "wayland") {
        m_eglQuery.reset(new EGLInfoQuery(QString::fromUtf8(env), EGLInfoQuery::Platform::Wayland));
    } else {
        env = qgetenv("DISPLAY");
        if (!env.isEmpty()) {
            m_glxQuery.reset(new GLXInfoQuery());
            if (!(m_glxQuery && m_glxQuery->isValid())) {
                m_eglQuery.reset(new EGLInfoQuery(QString::fromUtf8(env), EGLInfoQuery::Platform::X11));
            }
        }
    }
}

GLInfoQuery::~GLInfoQuery()
{

}

QString GLInfoQuery::glRenderDevice()
{
    if (m_eglQuery && m_eglQuery->isValid()) {
        return m_eglQuery->glRenderDevice();
    }

    if (m_glxQuery && m_glxQuery->isValid()) {
        return m_glxQuery->glRenderDevice();
    }
    qWarning() << "GLInfoQuery::glRenderDevice() failed.";
    return QString();
}

bool GLInfoQuery::isHardwareRendering()
{
    //dbus
    QDBusReply<bool> registeredReply = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("com.kylin.Wlcom"));
    if (registeredReply.isValid() && registeredReply.value()) {
        QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("com.kylin.Wlcom"),
                                                          QStringLiteral("/com/kylin/Wlcom/Renderer"),
                                                          QStringLiteral("com.kylin.Wlcom.Renderer"),
                                                          QStringLiteral("IsHardwareRendering"));
        QDBusReply<bool> reply  = QDBusConnection::sessionBus().call(msg);
        if(reply.isValid()) {
            return reply.value();
        }
    }

    //glx
    if (m_glxQuery && m_glxQuery->isValid()) {
        QVariant isHardwareRendering = m_glxQuery->isHardwareRendering();
        if (isHardwareRendering.isValid()) {
            return isHardwareRendering.toBool();
        }
    }

    qWarning() << "GLInfoQuery::isHardwareRendering: failed to retrieve from dbus and glx, return true";
    return true;
}

}
