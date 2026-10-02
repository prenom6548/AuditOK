/*
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tablet_manager.h"
#include "abstract_wayland_output.h"
#include "composite.h"
#include "effectloader.h"
#include "effects.h"
#include "group.h"
#include "internal_client.h"
#include "main.h"
#include "scene.h"
#include "screens.h"
#include "unmanaged.h"
#include "wayland_server.h"
#include "workspace.h"
#include "x11client.h"
#include <kwineffects.h>

// Qt
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFile>
#include <QPainter>
#include <QStyleOption>
#include <QX11Info>

namespace KWin
{

static const QString BLUR_EFFECT_NAME = "blur";
static const QString TRANSLUCENCY_EFFECT_NAME = "kwin4_effect_translucency";
static const QString SCALEDESKTOP_EFFECT_NAME = "kwin4_effect_scaledesktop";
static const QString KYLIN_STATUSMANAGER_SERVICE = "com.kylin.statusmanager.interface";
static const QString KYLIN_STATUSMANAGER_PATH = "/";
static const QString KYLIN_STATUSMANAGER_INTERFACE = "com.kylin.statusmanager.interface";
static bool isBlurBackgroundInitialized = false;

static bool isWindowShouldNotBeProcessed(const AbstractClient *client)
{
    // 前面这一堆可不可以换成 isSpecialWindow()?
    return client->isSpecialWindow() || client->isUtility() || client->isNotification()
        || client->isInternal()
        || client->caption() == QStringLiteral("tablet-taskbar")
        || client->caption().contains(QStringLiteral("sogouImeService"));
}

/**
 * 当窗口的 WM_TRANSIENT_FOR 不存在或者为自己的窗口句柄时，会将同 group 内的所有主界面
 * 加到自己的主界面窗口列表中，此时也就只会出现一层主界面窗口，不再需要递归遍历
 */
static QList<AbstractClient *> findFamilyWindows(AbstractClient *client)
{
    if (client == nullptr) {
        return QList<AbstractClient *>();
    }

    QList<AbstractClient *> mainWindowList;
    // for Wayland
    if (!client->group()) {
        mainWindowList.append(client->transients());
        mainWindowList.append(client);
        AbstractClient *mainWindow = client;
        while (mainWindow->mainClients().size() > 0) {
            mainWindowList.append(client->mainClients());
            mainWindow = mainWindow->mainClients().at(0);
        }
        return mainWindowList;
    }
    // for X11
    QList<X11Client *> groupWindows = client->group()->members();
    for (auto *index : groupWindows) {
        mainWindowList.append(index);
    }
    return mainWindowList;
}

KWIN_SINGLETON_FACTORY(TabletManager)

TabletManager::TabletManager(QObject *parent)
    : QObject(parent)
    , m_background(new TabletBackground())
    , m_singleWinManager(nullptr)
    , m_bTabletMode(false)
    , m_existTabletDesktop(false)
    , m_effects(nullptr)
    , m_effectloader(nullptr)
{
    m_existTabletDesktop = QFile::exists(QStringLiteral("/usr/bin/ukui-tablet-desktop"));
    if (kwinApp()->operationMode() == Application::OperationMode::OperationModeX11) {
        m_background->setState(NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        // TODO: 在这块如果不设置窗口可见和背景模糊的话，X 下无法创建成功窗口并受窗管控制
        m_background->show();
        setBlurBehind();
    }
    // Workspace is created after the wayland server is initialized.
    connect(kwinApp(), &Application::workspaceCreated, this, &TabletManager::init);
}

TabletManager::~TabletManager()
{
    // TODO: 此处调用析构会导致double free
    // if (m_background) {
    //     delete m_background;
    //     m_background = nullptr;
    // }
    if (m_singleWinManager) {
        delete m_singleWinManager;
        m_singleWinManager = nullptr;
    }
    m_effectloader = nullptr;
    m_effects = nullptr;
}

void TabletManager::init()
{
    if (m_background) {
        m_background->init();
    }

    // 延迟一个事件循环再初始化单窗口管理模块类对象，因为混成类是在Workspace的构造函数中调用的，而混成在启动函数中
    // 调用了 Workspace::self() 函数 ，所以混成类延迟了一个事件循环来保证Worksapce构造完成。因此此处也需要延迟
    // 一个事件循环来确保在特效启动过程中模糊特效插件已经加载完成。不然在 X11 环境下太早对背景窗口最小化会导致模糊失败。
    // 在这里使用混成的信号 compositingToggled(bool) 不是很合适，因为混成无法开启时，不会触发此信号。
    QTimer::singleShot(0, [this]() {
        if (!m_singleWinManager) {
            m_singleWinManager = new SingleWindowManager(this);
            m_singleWinManager->setBlurBackground(m_background);
        }
    });
    connect(Compositor::self(), &Compositor::compositingToggled, this, [this](bool active) {
        if (active) {
            m_effects = static_cast<EffectsHandlerImpl *>(effects);
            m_effectloader = m_effects->findChild<AbstractEffectLoader *>();
            connect(m_effectloader, &AbstractEffectLoader::effectLoaded, this, [this](KWin::Effect *effect, const QString &name) {
                
                if (name == BLUR_EFFECT_NAME || name == TRANSLUCENCY_EFFECT_NAME) {
                    m_background->update();
                }
            });
        } else {
            m_effectloader = nullptr;
            m_effects = nullptr;
        }
    });

    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_STATUSMANAGER_SERVICE,
                                                          KYLIN_STATUSMANAGER_PATH,
                                                          KYLIN_STATUSMANAGER_INTERFACE,
                                                          QStringLiteral("get_current_tabletmode"));
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        bool bTabletMode = response.arguments().takeFirst().toBool();
        slotSwitchTabletMode(bTabletMode);
    } else {
        qDebug() << "TabletMode dbus is invalid !";
    }

    QDBusConnection::sessionBus().connect(KYLIN_STATUSMANAGER_SERVICE,
                                          KYLIN_STATUSMANAGER_PATH,
                                          KYLIN_STATUSMANAGER_INTERFACE,
                                          QStringLiteral("mode_change_signal"),
                                          this,
                                          SLOT(slotSwitchTabletMode(bool)));
}

void TabletManager::slotSwitchTabletMode(bool bSurfaceMode)
{
    if (m_bTabletMode == bSurfaceMode) {
        return;
    }
    m_bTabletMode = bSurfaceMode;
    Q_EMIT tabletModeChanged(m_bTabletMode);

    if (m_singleWinManager) {
        if (KWin::effects) {
            EffectsHandlerImpl *effect = static_cast<EffectsHandlerImpl *>(effects);
            bool isLoad = effect->isEffectLoaded(SCALEDESKTOP_EFFECT_NAME);
            qDebug() << __func__ << "kwin4_effect_scaledesktop is loaded: " << isLoad;
            if (isLoad && existTabletDesktop()) {
                // 桌面缩放动作需要 1000 ms
                QTimer::singleShot(1000, [this]() {
                    m_singleWinManager->switchTablet(m_bTabletMode);
                });
            } else {
                m_singleWinManager->switchTablet(m_bTabletMode);
            }
        } else {
            qDebug() << __func__ << "KWin::effects pointer is not initialized ! ! !";
            m_singleWinManager->switchTablet(m_bTabletMode);
        }
    }

    // 在平板模式启动 kwin 时，此时 effects 对象还未初始化，此时也没有必要进行窗口移动
    if (KWin::effects == nullptr) {
        return;
    }
    const auto stacking_order = workspace()->stackingOrder();
    for (auto it = stacking_order.rbegin(); it != stacking_order.rend(); ++it) {
        AbstractClient *c = qobject_cast<AbstractClient *>(*it);
        if (!c || c->isDock() || c->isDesktop()) {
            continue;
        }
        if (m_bTabletMode) {
            m_windowDesktopMap.insert(c->internalId(), c->desktop());
            // Tablet 模式下所有工作区的窗口合并到一起
            moveWindowBetweenVDesktop(c->internalId(), c->desktop(), KWin::effects->currentDesktop());
        } else {
            if (m_windowDesktopMap.contains(c->internalId())) {
                // PC 模式下窗口还原到原来所在的工作区
                moveWindowBetweenVDesktop(c->internalId(), KWin::effects->currentDesktop(), m_windowDesktopMap.value(c->internalId()));
            }
        }
    }
    if (!m_bTabletMode) {
        m_windowDesktopMap.clear();
    }
}

void TabletManager::moveWindowBetweenVDesktop(QUuid windowId, int srcDesktopIndex, int destDesktopIndex)
{
    auto *window = KWin::effects->findWindow(windowId);
    if (window == nullptr || window->isOnAllDesktops()) {
        return;
    }

    auto desktopIndexList = window->desktops();
    if (!desktopIndexList.contains(srcDesktopIndex)) {
        return;
    }

    desktopIndexList.remove(desktopIndexList.indexOf(srcDesktopIndex));

    if (desktopIndexList.contains(destDesktopIndex)) {
        return;
    }

    desktopIndexList.append(destDesktopIndex);

    KWin::effects->windowToDesktops(window, desktopIndexList);
}

void TabletManager::toggleTabletMode()
{
    QDBusMessage call = QDBusMessage::createMethodCall(KYLIN_STATUSMANAGER_SERVICE,
                                                       KYLIN_STATUSMANAGER_PATH,
                                                       KYLIN_STATUSMANAGER_INTERFACE,
                                                       QStringLiteral("set_tabletmode"));
    call.setArguments({!m_bTabletMode, "kwin", "debug"});
    QDBusMessage response = QDBusConnection::sessionBus().call(call);
    if (response.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "[=== checkmode ===]"
                   << "set_tabletmode dbus not work, change locally";
        slotSwitchTabletMode(!m_bTabletMode);
    }
}

void TabletManager::setBlurBehind(double radius)
{
    if (kwinApp()->operationMode() == Application::OperationMode::OperationModeX11) {
        xcb_connection_t *c = QX11Info::connection();
        if (!c) {
            return;
        }
        const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
        xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
        QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
        if (!atom) {
            return;
        }

        QVector<uint32_t> data;
        QRegion region;
        data.reserve(region.rectCount() * 4 + 1); // 1 means radius
        for (const QRect &r : region) {
            // kwin on X uses device pixels, convert from logical
            auto dpr = qApp->devicePixelRatio();
            data << r.x() * dpr << r.y() * dpr << r.width() * dpr << r.height() * dpr;
        }
        data << radius;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, m_background->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
    } else {
        if (m_background && m_background->windowHandle()) {
            m_background->windowHandle()->setProperty("kwin_blur_strength", radius);
        }
    }
}

void TabletManager::showBackground(double radius)
{
    if (!m_singleWinManager) {
        return;
    }
    m_singleWinManager->showBackground();
    setBlurBehind(radius);
}

void TabletManager::hideBackground(double radius)
{
    if (!m_singleWinManager) {
        return;
    }
    m_singleWinManager->hideBackground();
    setBlurBehind(radius);
}

TabletBackground::TabletBackground()
    : QWidget()
{
    setWindowFlags(Qt::WindowStaysOnBottomHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    setWindowTitle("tablet_blur_background");
}

void TabletBackground::init()
{
    slotGeometryChanged();
    connect(screens(), &Screens::geometryChanged, this, &TabletBackground::slotGeometryChanged);
}

void TabletBackground::setState(NET::States state)
{
    NETWinInfo info(QX11Info::connection(), winId(), QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    info.setState(state, state);
}

void TabletBackground::paintEvent(QPaintEvent *event)
{

    if (!isBlurBackgroundInitialized) {
        return QWidget::paintEvent(event);
    }
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    // QColor color = qApp->palette().color(QPalette::Base);
    QColor color(64, 64, 64, 0);
    // color.setAlpha(m_alpha);
    bool isShouldBlur = false;
    if (effects) {
        EffectsHandlerImpl *effect = static_cast<EffectsHandlerImpl *>(effects);
        bool isLoadBlurEffect = effect->isEffectLoaded(BLUR_EFFECT_NAME);
        qDebug() << __func__ << isLoadBlurEffect;
        if (isLoadBlurEffect) {
            isShouldBlur = true;
        } else {
            isShouldBlur = false;
        }
    }
    if (isShouldBlur) {
        color.setAlpha(0);
    } else {
        color.setAlpha(255);
    }
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void TabletBackground::slotGeometryChanged()
{
    QRect screenGeo = screens()->geometry();
    setGeometry(screenGeo);
    setFixedSize(screenGeo.size());
}

SingleWindowManager::SingleWindowManager(QObject *parent)
    : QObject(parent)
    , m_blurHelperWindow(nullptr)
    , m_blurBackground(nullptr)
    , m_ignorePlaceCenterList()
    , m_transientWindowList()
    , m_bTabletMode(false)
{
    // 蓝牙托盘、网络工具托盘、搜索不需要进行居中处理
    m_ignorePlaceCenterList << "ukui-bluetooth"
                            << "kylin-nm"
                            << "ukui-search";
    m_transientWindowList << "ukui-bluetooth"
                          << "kylin-nm"
                          << "ukui-volume-control-applet-qt"
                          << "ukui-power-manager-tray"
                          << "kylin-activation";
    m_authenticWindowList << "polkit-ukui-authentication-agent-1"
                          << "polkit-kde-authentication-agent-1"
                          << "kylin-installer"
                          << "kylin-activation";
    connect(workspace(), &Workspace::clientAdded, this, &SingleWindowManager::handleClientAdded);
    connect(workspace(), &Workspace::clientRemoved, this, &SingleWindowManager::handleClientRemoved);
    connect(workspace(), &Workspace::clientActivated, this, &SingleWindowManager::handleClientActivated);
    connect(workspace(), &Workspace::clientMinimizedChanged, this, &SingleWindowManager::handleClientMinimizedChanged);
    // connect(workspace(), &Workspace::clientDemandsAttentionChanged, this, &SingleWindowManager::handleClientAdded);
    connect(screens(), &Screens::geometryChanged, this, &SingleWindowManager::handleScreenGeometryChanged);
}

void SingleWindowManager::setBlurBackground(TabletBackground *blurBackground)
{
    if (!blurBackground) {
        return;
    }
    m_blurBackground = blurBackground;
    if (!m_blurHelperWindow && m_blurBackground) {
        const auto stacking_order = workspace()->stackingOrder();
        for (int i = stacking_order.count() - 1; i > -1; --i) {
            AbstractClient *c = qobject_cast<AbstractClient *>(stacking_order.at(i));
            if (c && c->keepBelow() && c->caption().contains("tablet_blur_background")) {
                // only on X11
                m_blurHelperWindow = c;
                // 设置毛玻璃背景窗口为全屏，防止在updateClientArea()->checkWorkspacePosition()时毛玻璃窗口被修改位置
                // m_blurHelperWindow->setFullScreen(true, false);
                // m_blurHelperWindow->setDisableActive();
                m_blurHelperWindow->minimize(true, true);
                connect(Compositor::self(), &Compositor::compositingToggled, this, [this](bool active) {
                    if (active) {
                        if (m_blurHelperWindow->effectWindow()) {
                            m_blurHelperWindow->effectWindow()->setData(WindowForceBlurRole, QVariant(true));
                        }
                    }
                });
                break;
            }
        }
    }

    // kwin 启动时如果存在已经存在的窗口，需要进行判断是否需要将顶层最大化或者显示桌面
    switchTablet(TabletManager::self()->tabletMode());
    isBlurBackgroundInitialized = true;
}

void SingleWindowManager::switchTablet(bool bTabletMode)
{
    m_bTabletMode = bTabletMode;
    const auto stacking_order = workspace()->stackingOrder();
    if (stacking_order.isEmpty()) {
        return;
    }

    bool isTopWindowMaximized = true;
    AbstractClient *client = nullptr;
    AbstractClient *mainWindow = nullptr;
    for (int i = stacking_order.count() - 1; i > -1; --i) {
        client = qobject_cast<AbstractClient *>(stacking_order.at(i));
        if (!client) {
            continue;
        }
        if (TabletManager::self()->existTabletDesktop() && client->isDesktop()) {
            updateDesktopVisibleState(client);
        }
        if (isWindowShouldNotBeProcessed(client)) {
            continue;
        }
#ifndef QT_NO_DEBUG
        printClientInformation(client, __func__);
#endif
        if (m_bTabletMode && !mainWindow && !client->isMinimized() && isWindowBelongToWorkingWindow(client)) {
            mainWindow = client;
            isTopWindowMaximized = maximizeWindow(client);
        }
    }
    if (m_bTabletMode) {
        QList<AbstractClient *> familyWindows = findFamilyWindows(mainWindow);
        minimizeWindowsExceptForFamilyWindow(familyWindows);
    }
    if (m_bTabletMode && !isTopWindowMaximized) {
        showBackground();
    } else {
        hideBackground();
    }
}

template<class T>
void SingleWindowManager::addTransient(QVector<T *> &clients, T *transientClient)
{
    clients.append(transientClient);
}

template<class T>
bool SingleWindowManager::removeTransient(QVector<T *> &clients, T *transientClient)
{
    const int remove = clients.indexOf(transientClient);
    if (remove == -1) {
        return false;
    }
    clients.removeAt(remove);
    // #168430 关闭安全弹窗后，桌面会被毛玻璃覆盖无法点击；
    // 安全弹窗包含半透明全屏幕窗口（父窗口）和弹窗（子窗口）两个窗口。
    // 关闭时父窗口会先关闭，然后子窗口关闭。对于此类父子窗口关闭顺序异常的程序，
    // 若在子窗口关闭时，依旧认为还有父窗口，会导致后面逻辑中毛玻璃窗口不被隐藏。
    // 因此，如果被关闭的子窗口已无父窗口，则不再认为是移除了一个子窗口。
    return transientClient->mainClients().size() > 0;
}

void SingleWindowManager::printClientInformation(const AbstractClient *client, const char *funcName) const
{
    if (funcName) {
        qDebug() << "*** " << funcName << " *** :";
    }
    qDebug() << "Caption: " << client->caption() << "WindowType: " << client->windowType()
             << "resourceName: " << client->resourceName() << "resourceClass: " << client->resourceClass()
             << "isDialog: " << client->isDialog() << "isUtility: " << client->isUtility()
             << "isMaximizable: " << client->isMaximizable() << "transients:" << client->transients()
             << "mainClients: " << client->mainClients();
}

bool SingleWindowManager::isBackgroundWindow(const AbstractClient *client) const
{
    return client == m_blurHelperWindow || client->caption() == QStringLiteral("tablet_blur_background");
}

bool SingleWindowManager::isClientShouldBeMaximized(const AbstractClient *client) const
{
    if (client->isDialog() && client->mainClients().size() > 0) {
        return false;
    }
    if (client->isUtility() && client->mainClients().size() > 0) {
        return false;
    }
    if (client->caption() == QStringLiteral("EmojiFloatWnd")) {
        return false;
    }
    if (client->isMaximizable()) {
        return true;
    }
    return false;
}

bool SingleWindowManager::isWindowNeedBlurBackground(const AbstractClient *client) const
{
    if (client->isDesktop() || client->isDock() || isBackgroundWindow(client) || client->isWatermark() || client->isSystemWindow() || client->isInputPanel()) {
        return false;
    }
    return true;
}

bool SingleWindowManager::isWindowBelongToWorkingWindow(const AbstractClient *client) const
{
    if (m_transientWindowList.contains(client->resourceClass())) {
        return false;
    }
    return true;
}

bool SingleWindowManager::isWindowBelongToAuthenticWindow(const AbstractClient *client) const
{
    if (m_authenticWindowList.contains(client->resourceClass())) {
        return true;
    }
    return false;
}

bool SingleWindowManager::isClientShouldNotBePlacedCenter(const AbstractClient *client) const
{
    if (m_ignorePlaceCenterList.contains(client->resourceClass())) {
        return true;
    } else if (client->caption() == QStringLiteral("EmojiFloatWnd")) {
        return true;
    } else {
        return false;
    }
}

void SingleWindowManager::placeWindowCenter(AbstractClient *client) const
{
    // utility 表示一个小的持久的实用程序窗口，例如调色板或工具箱，用户可能在工作时保持该窗口的打开状态，所以这种类型窗口也不应该被居中
    if (!client->isUtility()) {
        Placement::self()->placeCentered(client, workspace()->activeOutput()->geometry());
        // 屏幕输出大小改变时更新一下还原状态的geometry是为了防止居中后，在updateClientArea()时又被移动到PC模式保存的位置（不再居中显示）
        client->updateGeometryRestore();
    }
}

bool SingleWindowManager::maximizeCurrentMainWindowOnly(AbstractClient *mainWindow)
{
    bool isWindowMaximized = false;
    if (mainWindow->isMaximizable() || mainWindow->isFullScreen()) {
        mainWindow->maximize(MaximizeFull);
        isWindowMaximized = true;
    } else {
        placeWindowCenter(mainWindow);
    }
    return isWindowMaximized;
}

bool SingleWindowManager::maximizeWindow(AbstractClient *client)
{
    bool hasMaximizedWindow = false;
    AbstractClient *operateWindow = client;
    if (!isClientShouldBeMaximized(operateWindow)) {
        if (!isClientShouldNotBePlacedCenter(operateWindow)) {
            placeWindowCenter(operateWindow);
            hasMaximizedWindow = false;
        }
    } else {
        client->maximize(MaximizeFull);
        hasMaximizedWindow = true;
    }
    while (operateWindow->mainClients().size() > 0) {
        if (operateWindow->mainClients().size() == 1) {
            AbstractClient *mainWindow = operateWindow->mainClients().at(0);
            // 如果 mainWindow 之前被最小化过，则需要先取消最小化，否则 mainWindow 并不会显示出来
            mainWindow->unminimize(true);
            if (maximizeCurrentMainWindowOnly(mainWindow)) {
                hasMaximizedWindow = true;
            }
            operateWindow = mainWindow;
        } else {
            for (auto *mainWindow : operateWindow->mainClients()) {
                if (maximizeCurrentMainWindowOnly(mainWindow)) {
                    hasMaximizedWindow = true;
                }
            }
            break;
        }
    }
    if (client->resourceClass() == QStringLiteral("wemeetapp")) {
        if (!client->group()) {
            return hasMaximizedWindow;
        }
        QList<X11Client *> groupWindows = client->group()->members();
        // 分组里的窗口按照创建顺序存储，第一个即为主窗口
        X11Client *mainWindow = groupWindows.at(0);
        if (maximizeCurrentMainWindowOnly(mainWindow)) {
            hasMaximizedWindow = true;
        }
    }
    return hasMaximizedWindow;
}

void SingleWindowManager::updateDesktopVisibleState(AbstractClient *desktop)
{
    if (!desktop || !desktop->isDesktop()) {
        return;
    }
    if (effects) {
        EffectsHandlerImpl *effect = static_cast<EffectsHandlerImpl *>(effects);
        bool isModeSwitchEffectLoad = effect->isEffectLoaded(SCALEDESKTOP_EFFECT_NAME);
        // 桌面缩放特效已经成功加载，由特效插件去处理桌面的显示与隐藏
        if (isModeSwitchEffectLoad) {
            qDebug() << __func__ << __LINE__ << "Mode switch effect plugin is loaded, return directly !";
            return;
        }
    }
    // 桌面缩放特效未加载，由 SingleWindowManager 管理桌面的显示与隐藏
    qDebug() << __func__ << __LINE__ << "Mode switch effect plugin is not loaded !";
    if (desktop->caption().contains(QStringLiteral("ukui-tablet-desktop"))) {
        m_tabletDesktop = desktop;
        if (!m_bTabletMode) {
            m_tabletDesktop->hideClient();
        } else {
            m_tabletDesktop->showClient();
        }
    } else {
        m_pcDesktop = desktop;
        if (m_bTabletMode) {
            m_pcDesktop->hideClient();
        } else {
            m_pcDesktop->showClient();
        }
    }
}

void SingleWindowManager::showBackground()
{
    if (kwinApp()->operationMode() == Application::OperationMode::OperationModeX11) {
        if (m_blurHelperWindow) {
            m_blurHelperWindow->unminimize(true);
        }
        m_blurBackground->update();
    } else {
        m_blurBackground->show();
        // 对于 Internal window, 直接设置属性即可，没有必要去调用 KWindowEffects 的接口
        m_blurBackground->windowHandle()->setProperty("kwin_blur", QRegion());
    }
}

void SingleWindowManager::hideBackground()
{
#ifndef QT_NO_DEBUG
    qDebug() << __func__ << "Hide Background ! ! !";
#endif
    if (kwinApp()->operationMode() == Application::OperationMode::OperationModeX11) {
        if (m_blurHelperWindow) {
            m_blurHelperWindow->minimize(true, true);
        }
    } else {
        m_blurBackground->hide();
    }
}

void SingleWindowManager::handleClientAdded(AbstractClient *client)
{
    if (!client) {
        return;
    }
    // 存在部分窗口，它的主界面窗口还没有 show 出来或者隐藏了，找不到主界面窗口指针，但是该窗口仍为瞬态窗口，这
    // 种窗口我们暂时不将其加到瞬态窗口列表里。
    if (client->isTransient() && client->mainClients().size() > 0) {
        addTransient(m_transientClients, client);
    }
    if (TabletManager::self()->existTabletDesktop() && client->isDesktop()) {
        if (client->caption().contains(QStringLiteral("ukui-tablet-desktop"))) {
            qDebug() << __func__ << __LINE__ << "ukui-tablet-desktop add" << m_bTabletMode;
            m_tabletDesktop = client;
            if (!m_bTabletMode) {
                m_tabletDesktop->hideClient();
            }
        } else {
            qDebug() << __func__ << __LINE__ << "peony-qt-desktop add" << m_bTabletMode;
            m_pcDesktop = client;
            if (m_bTabletMode) {
                m_pcDesktop->hideClient();
            }
        }
    }
    if (!m_bTabletMode) {
        return;
    }
}

void SingleWindowManager::handleClientRemoved(AbstractClient *client)
{
    if (!client) {
        return;
    }
    bool isTransientRemoved = removeTransient(m_transientClients, client);
    if (!m_bTabletMode) {
        return;
    }
#ifndef QT_NO_DEBUG
    printClientInformation(client, __func__);
#endif
    // 该窗口存在主界面窗口，移除此窗口后还有主界面窗口，直接return即可，不需要处理。但是 client->isTransient() 返回的值永远都
    // 是 false，因为 workspace 在发送 clientRemoved 信号前，已经将父子窗口全部清空关系了。所以要将瞬态窗口提前保存下来，利用自
    // 定义的 removeTransient() 函数来判断是否移除的是瞬态窗口。瞬态窗口不直接return的话可能会导致毛玻璃背景先隐藏再被显示出来。
    // wayland 环境下任务栏隐藏也会触发此槽函数，直接return即可。
    if (isTransientRemoved || isWindowShouldNotBeProcessed(client)) {
        return;
    }
    // 有窗口关闭后需要重新遍历所有窗口来决定是否需要显示或者隐藏毛玻璃背景，因为最大化窗口可能覆盖到同一进程的非最大化窗口上方，而
    // 且可能多个非最大化窗口相互覆盖，此时关闭顶层窗口后不需要隐藏毛玻璃窗口。
    const auto &stacking_order = workspace()->stackingOrder();
    bool isBackgroundShouldShow = false;
    AbstractClient *tmpClient = nullptr;
    for (int i = stacking_order.count() - 1; i > -1; --i) {
        tmpClient = qobject_cast<AbstractClient *>(stacking_order.at(i));
        if (!tmpClient || isWindowShouldNotBeProcessed(tmpClient) || isBackgroundWindow(tmpClient)) {
            continue;
        }
        if (!tmpClient->isMinimized() && isWindowBelongToWorkingWindow(tmpClient)) {
            isBackgroundShouldShow = !(tmpClient->maximizeMode() == MaximizeFull);
        }
    }
    if (isBackgroundShouldShow) {
        showBackground();
    } else {
        hideBackground();
    }
}

void SingleWindowManager::handleClientActivated(AbstractClient *client)
{
    // 在 X11 中，锁屏为 unmanaged 窗口，workspace() 并不会转发 clientActivated() 信号
    // wayland 中不存在不受窗管管理的窗口，所以也会拿到锁屏的激活信号
    // 2023年计划扩展窗口类型，暂时只能通过标题名判断
    if (!m_bTabletMode || !client || isBackgroundWindow(client)
        || isWindowShouldNotBeProcessed(client)
        || !isWindowBelongToWorkingWindow(client)) {
        return;
    }
#ifndef QT_NO_DEBUG
    // transientFor() 拿到的返回值是不准确的， mainClients() 可以拿到所有的父窗口
    // 先不考虑 mainClients() 返回多个父窗口的情况，因为目前不确定是否会存在这种情况
    printClientInformation(client, __func__);
#endif
    if (!isWindowNeedBlurBackground(client)) {
        hideBackground();
        return;
    }
    // 鉴权窗口，不需要对其他窗口进行任何处理
    if (isWindowBelongToAuthenticWindow(client)) {
        placeWindowCenter(client);
        // 某些应用打开前必须鉴权，在显示鉴权窗口时，将毛玻璃背景显示出来。
        // 在应用内部操作时激活鉴权窗口时，显示出毛玻璃背景窗口也无可厚非。
        showBackground();
        return;
    }

    bool hasMaximizedWindow = maximizeWindow(client);
    QList<AbstractClient *> familyWindows = findFamilyWindows(client);
    minimizeWindowsExceptForFamilyWindow(familyWindows);
    if (hasMaximizedWindow) {
        hideBackground();
    } else {
        showBackground();
    }
}

void SingleWindowManager::handleClientMinimizedChanged(AbstractClient *client)
{
    if (!m_bTabletMode || !client || !client->isMinimized()) {
        return;
    }
    // 该窗口存在主界面窗口，最小化此窗口后还有主界面窗口，直接return即可，不需要处理。
    // 存在窗口的主界面窗口还没有 show 出来或者隐藏了，该窗口为瞬态窗口，但是找不到主界面窗口
    if ((client->isTransient() && client->mainClients().size() > 0) || client->isSpecialWindow()
        || isBackgroundWindow(client)) {
        return;
    }
#ifndef QT_NO_DEBUG
    printClientInformation(client, __func__);
#endif
    const auto stacking_order = workspace()->stackingOrder();
    if (stacking_order.isEmpty()) {
        return;
    }

    for (int i = stacking_order.count() - 1; i > -1; --i) {
        AbstractClient *c = qobject_cast<AbstractClient *>(stacking_order.at(i));
        if (!c || c->isSpecialWindow() || isWindowShouldNotBeProcessed(c)) {
            continue;
        }
        if (c == client && !c->isResizable()) {
            hideBackground();
        }
        break;
    }
}

void SingleWindowManager::handleScreenGeometryChanged()
{
    if (!m_bTabletMode) {
        return;
    }
    m_blurHelperWindow->moveResize(screens()->geometry());

    AbstractClient *operateWindow = workspace()->activeClient();
    if (!operateWindow)
        operateWindow = workspace()->allClientList().last();

    if (!operateWindow || isWindowShouldNotBeProcessed(operateWindow) || isClientShouldNotBePlacedCenter(operateWindow)) {
        return;
    }
    if (!operateWindow->isMaximizable()) {
        placeWindowCenter(operateWindow);
    }
    while (operateWindow->mainClients().size() > 0) {
        AbstractClient *mainWindow = operateWindow->mainClients().back();
        if (!mainWindow->isMaximizable() && !isClientShouldNotBePlacedCenter(mainWindow)) {
            placeWindowCenter(mainWindow);
        }
        operateWindow = mainWindow;
    }
}

void SingleWindowManager::minimizeWindowsExceptForFamilyWindow(QList<AbstractClient *> familyWindowList)
{
#ifndef QT_NO_DEBUG
    for (auto *topMainWindow : familyWindowList) {
        printClientInformation(topMainWindow, __func__);
    }
#endif

    const auto stacking_order = workspace()->stackingOrder();
    // 选择从上向下最小化是因为在存在父子窗口的堆叠顺序时，如果先小化父窗口可能会在桌面留下父窗口的残影
    // 所以选择从上向下最下化
    for (int i = stacking_order.size() - 1; i > -1; --i) {
        auto *client = qobject_cast<AbstractClient *>(stacking_order.at(i));
        if (!client || isWindowShouldNotBeProcessed(client) || familyWindowList.contains(client)) {
            continue;
        } else if (isWindowBelongToAuthenticWindow(client)) {
            placeWindowCenter(client);
            continue;
        }
        qDebug() << __func__ << "exec minimize" << client->caption()
                 << client->window() << client->windowType();
        client->minimize(true);
    }
}

}
