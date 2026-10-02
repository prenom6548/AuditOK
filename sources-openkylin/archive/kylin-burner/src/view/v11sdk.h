#ifndef _V11_SDK_H_
#define _V11_SDK_H_

#include <QWidget>

#if UKUI_SESSION_TYPE == 1
    #include <ukuistylehelper/ukuistylehelper.h>
    using namespace kdk;
#elif UKUI_SESSION_TYPE == 2
    #include <ukuiwindowhelper/ukuiwindowhelper.h>
#endif


static void ukuiWaylandWindowStyleChange(QWidget* window){
#if UKUI_SESSION_TYPE == 2
    UkuiWindowHelper* m_windowHelper = new UkuiWindowHelper(window);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setSkipTaskBar(true);
#elif UKUI_SESSION_TYPE == 1
    kdk::UkuiStyleHelper::self()->removeHeader(window);
    /* 设置Widget的显示属性: 任务栏不显示该窗口、无标题栏、窗口选择器不显示该窗口 */
    window->setProperty("ukui_surface_skip_taskbar", true);
    window->setProperty("ukui_surface_no_titlebar", true);
    window->setProperty("ukui_surface_skip_switcher", true);
#endif
}


static void ukuiWaylandRemoveTitleBar(QWidget* window){
#if UKUI_SESSION_TYPE==2
     UkuiWindowHelper* m_windowHelper = new UkuiWindowHelper(window);
     m_windowHelper->removeTitleBar();
#elif UKUI_SESSION_TYPE==1
    kdk::UkuiStyleHelper::self()->removeHeader(window);
#endif
}


#endif
