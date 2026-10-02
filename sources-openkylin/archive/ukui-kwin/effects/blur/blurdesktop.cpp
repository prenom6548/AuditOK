#include "blurdesktop.h"

#include "blurshader.h"
// KConfigSkeleton
#include "blurconfig.h"

#include <QtDBus>
#include <QGuiApplication>
#include <QMatrix4x4>
#include <QPainterPath>
#include <QScreen> // for QGuiApplication
#include <QTime>
#include <QWindow>
#include <QFile>
#include <cmath> // for ceil()

#include <KWindowEffects>

namespace KWin {

BlurDesktopEffect::BlurDesktopEffect()
    : m_bTabletMode(false),
      m_blurHelperWindow(nullptr),
      m_activated(false)
{
    connect(effects, &EffectsHandler::windowAdded, this, &BlurDesktopEffect::slotWindowAdded);
    connect(effects, &EffectsHandler::windowClosed, this, &BlurDesktopEffect::slotWindowClosed);
    connect(effects, &EffectsHandler::windowActivated, this, &BlurDesktopEffect::slotWindowActivated);
    connect(effects, &EffectsHandler::windowMinimized, this, &BlurDesktopEffect::slotWindowMinimized);
    connect(effects, &EffectsHandler::windowUnminimized, this, &BlurDesktopEffect::slotWindowUnminimized);
    connect(effects, &EffectsHandler::windowMaximizedStateChanged, this, &BlurDesktopEffect::slotWindowMaximizedStateChanged);

    init();
    initBlurStrengthValues();
    initSlotConnection();
}

BlurDesktopEffect::~BlurDesktopEffect()
{

}

void BlurDesktopEffect::init()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.kylin.statusmanager.interface",
                                                            "/",
                                                            "com.kylin.statusmanager.interface",
                                                            "get_current_tabletmode");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);

    if (response.type() == QDBusMessage::ReplyMessage) {
        m_bTabletMode = response.arguments().takeFirst().toBool();

        //printf("TabletMode = %d\n", m_bTabletMode);
    } else {
        qDebug() << "info: [BlurDesktopEffect][init]: DBus Tablet-Mode Get Failed! Error message = "
                 << response.errorMessage();
    }

    findBlurHelperWindow();
    // 更新虚化窗口的状态
    slotUpdateBlurWindow();

    // 初始化虚化的桌面组价列表
    m_blurAllowList.push_back("ukui-bluetooth");   // 蓝牙很特殊！和sidebar的属性一致，无法区分出来，只能特殊判断
}

void BlurDesktopEffect::initBlurStrengthValues()
{

}

void BlurDesktopEffect::initSlotConnection()
{
    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                          "/",
                                          "com.kylin.statusmanager.interface",
                                          "mode_change_signal",
                                          this,
                                          SLOT(slotTabletModeChanged(bool)));
}

bool BlurDesktopEffect::isActive() const
{
    // 特效处于默认打开状态，只在平板模式下激活
    return m_bTabletMode && m_activated;
}

void BlurDesktopEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    effects->prePaintScreen(data, time);
}

void BlurDesktopEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
}

void BlurDesktopEffect::setTabletMode(bool isTabletMode){
    m_bTabletMode = isTabletMode;
}

bool BlurDesktopEffect::isTabletMode() const {
    return m_bTabletMode;
}

// slots
void BlurDesktopEffect::slotTabletModeChanged(bool isTabletMode)
{
    setTabletMode(isTabletMode);
    // printf("========slotTabletModeChanged\n");
    slotUpdateBlurWindow();
    if (!m_bTabletMode)
        setActive(false);
}

void BlurDesktopEffect::slotUpdateBlurWindow()
{
    EffectWindowList windowList = effects->stackingOrder();
    for(EffectWindow *w : windowList) {
        slotWindowActivated(w);
    }
}

//-----------------------------------------------------------------------------
// User interaction

void BlurDesktopEffect::slotWindowActivated(EffectWindow* w)
{
    if (!w)
        return;

    if (ignoreWindow(w)) {
        return;
    }

    bool active = !skipWindow(w);
    setActive(active);
}

void BlurDesktopEffect::slotWindowAdded(EffectWindow* w)
{
    slotWindowActivated(w);
}

void BlurDesktopEffect::slotWindowClosed(EffectWindow* w)
{
    Q_UNUSED(w);
    EffectWindow* activeWindow = effects->activeWindow();
    if (ignoreWindow(activeWindow) || ignoreWindow(w))
        return;
    slotWindowActivated(activeWindow);
    setActive(false);
}

void BlurDesktopEffect::slotWindowMinimized(EffectWindow* w)
{
    if (w->isDesktop())
        return;

    EffectWindow* activeWindow = effects->activeWindow();
    slotWindowActivated(activeWindow);
}

void BlurDesktopEffect::slotWindowUnminimized(EffectWindow* w)
{
    slotWindowActivated(w);
}

void BlurDesktopEffect::slotWindowMaximizedStateChanged(KWin::EffectWindow *w, bool horizontal, bool vertical)
{
    Q_UNUSED(horizontal);
    Q_UNUSED(vertical);
    slotWindowActivated(w);
}


bool BlurDesktopEffect::isInBlurAllowList(EffectWindow *w)
{
    const QString &wClass = w->windowClass();
    for (const QString &str : m_blurAllowList) {
        if(wClass.indexOf(str) == 0) {
            return true;
        }
    }
    return false;
}

KWin::EffectWindow* BlurDesktopEffect::findBlurHelperWindow()
{
    if (m_blurHelperWindow)
        return m_blurHelperWindow;

    EffectWindowList windowList = effects->stackingOrder();
    for (EffectWindow *w : windowList) {
        if(w && w->keepBelow() && w->caption() == "ukui-kwin_tablet_background") {
            m_blurHelperWindow = w;
            m_blurHelperWindow->minimize();
            break;
        }
    }
    return m_blurHelperWindow;
}

bool BlurDesktopEffect::ignoreWindow(KWin::EffectWindow *w)
{
    if (!w)
        return true;
    //printEffectWindowInfo(w);

    if (!m_bTabletMode)
        return true;
    if (m_blurHelperWindow == w)
        return true;
    if (w->isMinimized())
        return true;

    // 下面三个判断有先后顺序的区别
    if (w->isDesktop())
        return false;
    if (w->isSpecialWindow())
        return true;
    if (w->isNormalWindow())
        return false;

    return false;
}

bool BlurDesktopEffect::skipWindow(KWin::EffectWindow *w)
{
    if (ignoreWindow(w))
        return true;

    // 针对白名单应用，直接对窗口进行虚化
    if (isInBlurAllowList(w))
        return false;

    if (w->isDesktop())
        return true;

    // 跳过 Utility(小的持久实用程序窗口)类型的窗口，如调色板或工具箱
    // if (w->isUtility()) {
    //     return true;
    // }

    // 跳过 Tooltip(提示窗口，鼠标在窗口上停留一段时间后显示) 类型的窗口，
    if (w->isTooltip()) {
        return true;
    }
    // 跳过 Menu(可插入的菜单窗口) 类型的窗口
    if (w->isMenu()) {
        return true;
    }

    return false;
}

void BlurDesktopEffect::setActive(bool active)
{
    if (!m_blurHelperWindow)
        return;
    if (m_activated == active)
        return; // Already in that state
    m_activated = active;

    bool needUnminized = m_bTabletMode && active;
    bool isUnminized = !m_blurHelperWindow->isMinimized();
    if (needUnminized ^ isUnminized){
        m_blurHelperWindow->setMinimized(!needUnminized);
        effects->addRepaintFull();
    }
}

void BlurDesktopEffect::printEffectWindowInfo(EffectWindow *w)
{
    printf("\n+++++++++++++++++++++++++++++++++\n");
    printf("caption: %s\n"
           "windowClass: %s\n"
           "windowRole: %s\n"
           "windowType: %d\n"
           "isModal: %d\n"
           "isManaged: %d\n"
           "isPopupWindow: %d\n"
           "isToolbar: %d\n"
           "isMenu: %d\n"
           "isSplash: %d\n"
           "isUtility: %d\n"
           "isComboBox: %d\n"
           "isDialog: %d\n"
           "isDropdownMenu: %d\n"
           "isTooltip: %d\n"
           "isNotification: %d\n"
           "isNormalWindow: %d\n"
           "isSpecialWindow: %d\n"
           "isPopupMenu: %d\n",
           w->caption().toStdString().c_str(),
           w->windowClass().toStdString().c_str(),
           w->windowRole().toStdString().c_str(),
           w->windowType(),
           w->isModal(),
           w->isManaged(),
           w->isPopupWindow(),
           w->isToolbar(),
           w->isMenu(),
           w->isSplash(),
           w->isUtility(),
           w->isComboBox(),
           w->isDialog(),
           w->isDropdownMenu(),
           w->isTooltip(),
           w->isNotification(),
           w->isNormalWindow(),
           w->isSpecialWindow(),
           w->isPopupMenu());
    printf("+++++++++++++++++++++++++++++++++\n");
}

}
