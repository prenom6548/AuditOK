#include <unistd.h>

#include <QDebug>
#include <QGuiApplication>
#include <QWindow>
#include <QWindowList>

// kysdk
#include <kysdk/applications/ukuistylehelper/ukui-decoration-manager.h>
#include <ukuistylehelper/xatom-helper.h>

#include "windowsettings.h"

WindowSettings::WindowSettings(QObject *parent)
    : QObject{parent}
    , m_isWayland(QGuiApplication::platformName().startsWith(QStringLiteral("wayland")))
{
    setSkipTaskbarAndSwitcher();
    setKeepAbove();
    removeTitleBar();
}

void WindowSettings::setKeepAbove()
{
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& windowId){
        QString title = kdk::WindowManager::getWindowTitle(windowId);
        quint32 pid = kdk::WindowManager::getPid(windowId);
        if((quint32)getpid() == pid) {
            foreach (QWindow *window, qGuiApp->allWindows()) {
                if (window->title() == title) {
                    m_windowMap[window] = windowId;
                    kdk::WindowManager::self()->keepWindowAbove(windowId);
                    if (m_isWayland) {
                        UKUIDecorationManager::getInstance()->removeHeaderBar(window);
                    }
                }
            }
        }
    });
}

void WindowSettings::setSkipTaskbarAndSwitcher()
{
    // 设置不显示在任务栏和多任务视图会概率导致qml窗口设置x,y坐标不生效
    QWindowList m_windowList = qGuiApp->allWindows();
    foreach (QWindow *window, m_windowList) {
        kdk::WindowManager::self()->setSkipSwitcher(window, true);
        kdk::WindowManager::self()->setSkipTaskBar(window, true);
    }
}

void WindowSettings::removeTitleBar()
{
    QWindowList m_windowList = qGuiApp->allWindows();
    foreach (QWindow *window, m_windowList) {
        // 无边框窗口不需要去除标题栏
        if (window->flags().testFlag(Qt::FramelessWindowHint)) {
            return;
        }

        if (!m_isWayland) {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(static_cast<int>(window->winId()), hints);
        }
    }
}
