// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define protected public
#include "ukuipopupwindowhandle.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QDebug>
#include <QCursor>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPair>
#include <QQmlContext>
#include <QQmlEngine>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>

#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>
#include <private/qhighdpiscaling_p.h>
#include "ukuiwindowhelper.h"
#include "../../libqt5-ukui-style/settings/ukui-style-settings.h"

#define INNERRECT_WIDTH 1
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

//// className prepend string of QT_NAMESPACE if existed.
//bool inheritsTheClassType(QObject *object, QString className) {
//#if defined(QT_NAMESPACE)
//#define D_GET_NAMESPACE_STR_IMPL(M) #M "::"
//#define D_GET_NAMESPACE_STR(M) D_GET_NAMESPACE_STR_IMPL(M)
//    className.prepend(D_GET_NAMESPACE_STR(QT_NAMESPACE));
//#undef D_GET_NAMESPACE_STR
//#undef D_GET_NAMESPACE_STR_IMPL
//#endif
//    return object && object->inherits(qPrintable(className));
//}

using namespace UKUIQQC2Style;
UKUIPopupWindowHandle::UKUIPopupWindowHandle(QQuickItem *parent)
    : QQuickItem (parent)
{
    setFlag(ItemHasContents, false); // 不显示任何内容
    setAcceptHoverEvents(false); // 不接受鼠标悬停事件
    setAcceptTouchEvents(false); // 不接受触摸事件
    setAcceptedMouseButtons(Qt::NoButton); // 不接受鼠标点击事件

    QByteArray envValue = qgetenv("QT_QPA_PLATFORM");

    if (!envValue.isEmpty()) {
        QString value = QString::fromUtf8(envValue);
        if(value == "xcb")
        {
            m_isXCB = true;

            // qDebug() << "xcbxbcxcbxcb....." << m_isXCB;

        }
    }
    m_isHardwareRendering = false;
    m_wlcomDBus = new QDBusInterface("com.kylin.Wlcom",
                                                 "/com/kylin/Wlcom/Renderer",
                                                 "com.kylin.Wlcom.Renderer",
                                                 QDBusConnection::sessionBus());
    if(m_wlcomDBus->isValid()){
        QDBusReply<bool> isRenderingReply = m_wlcomDBus->call("IsHardwareRendering");
        if (isRenderingReply.isValid()) {
            bool isRendering = isRenderingReply.value();
            m_isHardwareRendering = isRendering;
        }
    }
    setBlurEnabled(m_isHardwareRendering);

    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        if (window) {
            if(m_windowStaysOnTopHint)
                window->setFlags(Qt::FramelessWindowHint | Qt::Window | Qt::ToolTip | Qt::WindowStaysOnTopHint);
            else{
//task 576023 menu.qml设置popup属性后，虚拟键盘菜单异常
                if(QQmlEngine::contextForObject(this) && QQmlEngine::contextForObject(this)->engine()){
                    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
                    if(engine->property("flags").isValid() && engine->property("flags").canConvert<QString>() &&
                            engine->property("flags").value<QString>() == "WindowDoesNotAcceptFocus"){
                        window->setFlags(Qt::ToolTip);
                    }
                    else
                        window->setFlags(Qt::Popup);
                }
                else
                    window->setFlags(Qt::Popup);
            }

                if(!m_isXCB){
                m_windowHelper = new UkuiWindowHelper(window);
                if (!m_titlebarVisible)
                    m_windowHelper->removeTitleBar();
                m_windowHelper->setSkipSwitcher(true);
                m_windowHelper->setSkipTaskBar(true);
//                QPainterPath path;
//                path.addRoundedRect(x(), y(), width(), height(), m_radius, m_radius);
//                QRegion effectRegion(path.toFillPolygon().toPolygon());
                m_windowHelper->setDecorationCompoents(UkuiWindowHelper::DecorationComponents(UkuiWindowHelper::DecorationComponent::Shadow) |
                                                       UkuiWindowHelper::DecorationComponents(UkuiWindowHelper::DecorationComponent::Border) |
                                                       UkuiWindowHelper::DecorationComponents(UkuiWindowHelper::DecorationComponent::RoundCorner));

                if(m_blurEnabled)
                m_windowHelper->setBlurEffect(QRegion());
            }
        }
    }, Qt::UniqueConnection);
}

UKUIPopupWindowHandle::~UKUIPopupWindowHandle()
{
    if(m_windowHelper){
        m_windowHelper->deleteLater();
        m_windowHelper = nullptr;
    }
    if (m_wlcomDBus) {
        delete m_wlcomDBus;
        m_wlcomDBus = nullptr;
    }
}

void UKUIPopupWindowHandle::setBlurStrength(int strength)
{
    m_blurStrength = strength;
}

void UKUIPopupWindowHandle::setBlurEnabled(bool enabled)
{
    m_blurEnabled = enabled;
}

void UKUIPopupWindowHandle::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    if (m_windowHelper && m_blurEnabled && !m_isXCB) {
//        QPainterPath path;
//        path.addRoundedRect(x(), y(), width(), height(), m_radius, m_radius);
//        QRegion effectRegion(path.toFillPolygon().toPolygon());

        m_windowHelper->setBlurEffect(QRegion());

    }
}

void UKUIPopupWindowHandle::setTitlebarVisible(bool visible)
{
    m_titlebarVisible = visible;
}



QSize UKUIPopupWindowHandle::getScreenSize()
{
    // 获取鼠标所在屏幕，如果没有特别指定，这里通常会是主屏幕
    const QScreen *screen =QGuiApplication::screenAt(QCursor::pos());
    Q_ASSERT(screen);
    if(!screen)
        return QSize();

    // 获取屏幕尺寸
    QRect screenGeometry = screen->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    return QSize(screenWidth, screenHeight);

}

QPoint UKUIPopupWindowHandle::getScreenPoint()
{
    // 获取鼠标所在屏幕，如果没有特别指定，这里通常会是主屏幕
    const QScreen *screen =QGuiApplication::screenAt(QCursor::pos());
    Q_ASSERT(screen);
    if(!screen)
        return QPoint();

    // 获取屏幕尺寸
    QRect screenGeometry = screen->availableGeometry();
    return QPoint(screenGeometry.x(), screenGeometry.y());

}

bool UKUIPopupWindowHandle::blur() const
{
    return m_blurEnabled;
}

QPoint UKUIPopupWindowHandle::posByCursor(int width, int height)
{
    QPoint p = QCursor::pos();
    // qDebug() << "cursor pos:" << p;
    const QScreen *screen =QGuiApplication::screenAt(QCursor::pos());
    if (const QPlatformScreen *platformScreen = screen ? screen->handle() : nullptr) {
        QPlatformCursor *cursor = platformScreen->cursor();
        const QSize nativeSize = cursor ? cursor->size() : QSize(16, 16);
        const QSize cursorSize = QHighDpi::fromNativePixels(nativeSize, platformScreen);
        QPoint offset(2, cursorSize.height());
        if (cursorSize.height() > 2 * height) {
            offset = QPoint(cursorSize.width() / 2, 0);
        }
        p += offset;

        QRect screenRect = screen->availableGeometry();
        if (p.x() + width > screenRect.x() + screenRect.width())
            p.rx() -= 4 + width;
        if (p.y() + height > screenRect.y() + screenRect.height())
            p.ry() -= 24 + height;
        if (p.y() < screenRect.y())
            p.setY(screenRect.y());
        if (p.x() + width > screenRect.x() + screenRect.width())
            p.setX(screenRect.x() + screenRect.width() - width);
        if (p.x() < screenRect.x())
            p.setX(screenRect.x());
        if (p.y() + height > screenRect.y() + screenRect.height())
            p.setY(screenRect.y() + screenRect.height() - height);
    }
    // qDebug() << "cursor pos1:" << p;

    return p;
}

bool UKUIPopupWindowHandle::posFollwMouse() const
{
    return m_posFollwMouse;
}

void UKUIPopupWindowHandle::setPosFollwMouse(bool newPosFollwMouse)
{
    m_posFollwMouse = newPosFollwMouse;
}

void UKUIPopupWindowHandle::setRadius(int newRadius)
{
    m_radius = newRadius;
}

void UKUIPopupWindowHandle::setWindowStaysOnTopHint(bool newWindowStaysOnTopHint)
{
    m_windowStaysOnTopHint = newWindowStaysOnTopHint;
}
