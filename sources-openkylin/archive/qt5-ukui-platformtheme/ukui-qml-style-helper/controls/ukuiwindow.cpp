#include "ukuiwindow.h"
#include <QDebug>
#include <QApplication>
#include <QGSettings>
#include <QFontDatabase>
#include <QIcon>
#include <QDBusConnection>
#include <QIcon>
#include <QPainter>
#include <QPair>
#include <QQuickWindow>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>

using namespace UKUISDKStyle;
UKUIWindow::UKUIWindow(QQuickItem *parent) : QQuickItem(parent)
{  
    m_wlcomDBus = new QDBusInterface("com.kylin.Wlcom",
                                                 "/com/kylin/Wlcom/Renderer",
                                                 "com.kylin.Wlcom.Renderer",
                                                 QDBusConnection::sessionBus());
    if(m_wlcomDBus->isValid()){
        QDBusReply<bool> isRenderingReply = m_wlcomDBus->call("IsHardwareRendering");
        if (isRenderingReply.isValid()) {
            bool isRendering = isRenderingReply.value();
            m_isRendering = isRendering;
//            setBlurEnabled(isRendering);
        }
    }

    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
            if (m_quickWindow) {
                disconnect(m_quickWindow, &QWindow::activeChanged, this, nullptr);
            }
            m_quickWindow = window;
            if(m_quickWindow){
                qDebug() << "windowwwwwwwwww" << m_quickWindow << (m_shadowEnabled && m_borderEnabled && m_roundCornerEnabled);
                if(m_windowHelper == nullptr)
                    m_windowHelper = new UkuiWindowHelper(m_quickWindow);
                if(m_removeTitleBar)
                m_windowHelper->removeTitleBar();
//                m_windowHelper->setSkipSwitcher(true);
//                m_windowHelper->setSkipTaskBar(true);

                qDebug() << "m_blurEnabled...." << m_blurEnabled << "m_isRendering" << m_isRendering << m_quickWindow->isActive();
                applyStandaloneWindow();
                updateBlurEffect();

                // 监听窗口激活/失焦（焦点变化）
                connect(m_quickWindow, &QWindow::activeChanged, this, [this]() {
                    qDebug() << "m_quickWindow activeChanged:" << (m_quickWindow && m_quickWindow->isActive());
                    updateBlurEffect();
                    emit windowActiveChanged();
                });
                emit windowActiveChanged();
            }
    });
}

UKUIWindow::~UKUIWindow()
{

}

UKUIWindow* UKUIWindow::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIWindow(p);
}

bool UKUIWindow::useStyleWindowManager() const
{
    return m_useStyleWindowManager;
}

void UKUIWindow::setUseStyleWindowManager(bool newUseStyleWindowManager)
{
    if (m_useStyleWindowManager == newUseStyleWindowManager)
        return;
    m_useStyleWindowManager = newUseStyleWindowManager;
    if(m_useStyleWindowManager && m_quickWindow){
        UKUIWindowManager::instance()->registerWindow(m_quickWindow);
    }
    emit useStyleWindowManagerChanged();
}

bool UKUIWindow::blurEnabled() const
{
    return m_blurEnabled;
}

void UKUIWindow::setBlurEnabled(bool newBlur)
{
    if(m_blurEnabled != newBlur){
        m_blurEnabled = newBlur;
        qDebug() << "setBlurEnabled....." << m_blurEnabled << m_isRendering
                 << (m_quickWindow ? m_quickWindow->isActive() : false);
        updateBlurEffect();
        emit blurEnabledChanged();
    }
}

void UKUIWindow::updateBlurEffect()
{
    if (!m_quickWindow || !m_windowHelper)
        return;
    const bool enable = m_blurEnabled && m_isRendering && m_quickWindow->isActive();
    m_windowHelper->setBlurEffect(QRegion(), 0, enable);
}

bool UKUIWindow::windowActive() const
{
    return m_quickWindow && m_quickWindow->isActive();
}

bool UKUIWindow::standaloneWindow() const
{
    return m_standaloneWindow;
}

void UKUIWindow::setStandaloneWindow(bool standalone)
{
    if (m_standaloneWindow == standalone)
        return;
    m_standaloneWindow = standalone;
    applyStandaloneWindow();
    emit standaloneWindowChanged();
}

void UKUIWindow::clearTransientParent()
{
    if (!m_quickWindow)
        return;
    m_quickWindow->setTransientParent(nullptr);
}

void UKUIWindow::applyStandaloneWindow()
{
    if (m_standaloneWindow)
        clearTransientParent();
}

bool UKUIWindow::shadowEnabled() const
{
    return m_shadowEnabled;
}

void UKUIWindow::setShadowEnabled(bool newShadow)
{
    if(m_shadowEnabled != newShadow) {
        m_shadowEnabled = newShadow;
        emit shadowEnabledChanged();
    }
}

bool UKUIWindow::borderEnabled() const
{
    return m_borderEnabled;
}

void UKUIWindow::setBorderEnabled(bool newBorder)
{
    if(m_borderEnabled != newBorder) {
        m_borderEnabled = newBorder;
        emit borderEnabledChanged();
    }
}

bool UKUIWindow::roundCornerEnabled() const
{
    return m_roundCornerEnabled;
}

void UKUIWindow::setRoundCornerEnabled(bool newRoundCorner)
{
    if(m_roundCornerEnabled != newRoundCorner) {
        m_roundCornerEnabled = newRoundCorner;
        emit roundCornerEnabledChanged();
    }
}

bool UKUIWindow::removeTitleBar() const
{
    return m_removeTitleBar;
}

void UKUIWindow::setRemoveTitleBar(bool newRemoveTitleBar)
{
    if (m_removeTitleBar == newRemoveTitleBar)
        return;
    m_removeTitleBar = newRemoveTitleBar;
    emit removeTitleBarChanged();
}
