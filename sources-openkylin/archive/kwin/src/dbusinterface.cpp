/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2012 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own
#include "dbusinterface.h"
#include "compositingadaptor.h"
#include "pluginsadaptor.h"
#include "virtualdesktopmanageradaptor.h"

// kwin
#include "abstract_client.h"
#include "atoms.h"
#include "backends/libinput/device.h"
#include "composite.h"
#include "debug_console.h"
#include "globalgestures.h"
#include "inputdevice.h"
#include "kwinadaptor.h"
#include "main.h"
#include "placement.h"
#include "platform.h"
#include "pluginmanager.h"
#include "renderbackend.h"
#include "unmanaged.h"
#include "virtualdesktops.h"
#include "workspace.h"
#ifdef KWIN_BUILD_ACTIVITIES
#include "activities.h"
#endif
#ifdef KWIN_BUILD_TABBOX
#include "tabbox.h"
#endif

// Qt
#include <QDBusServiceWatcher>
#include <QOpenGLContext>


#include <KGlobalAccel>

namespace KWin
{

DBusInterface::DBusInterface(QObject *parent)
    : QObject(parent)
    , m_serviceName(QStringLiteral("org.kde.KWin"))
{
    (void)new KWinAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/KWin"), this);
    const QByteArray dBusSuffix = qgetenv("KWIN_DBUS_SERVICE_SUFFIX");
    if (!dBusSuffix.isNull()) {
        m_serviceName = m_serviceName + QLatin1Char('.') + dBusSuffix;
    }
    if (!dbus.registerService(m_serviceName)) {
        QDBusServiceWatcher *dog = new QDBusServiceWatcher(m_serviceName, dbus, QDBusServiceWatcher::WatchForUnregistration, this);
        connect(dog, &QDBusServiceWatcher::serviceUnregistered, this, &DBusInterface::becomeKWinService);
    } else {
        announceService();
    }
    dbus.connect(QString(), QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"),
                 Workspace::self(), SLOT(slotReloadConfig()));
    connect(kwinApp(), &Application::x11ConnectionChanged, this, &DBusInterface::announceService);
    if (workspace() && workspace()->self()) {
        connect(workspace()->self(), &Workspace::clientActivated, this, [=](AbstractClient *c) {
            if (!c) {
                // the inactivated client is null
            } else {
                Q_EMIT activeWindowChanged(c->pid());
            }
        });
    }
}

void DBusInterface::becomeKWinService(const QString &service)
{
    // TODO: this watchdog exists to make really safe that we at some point get the service
    // but it's probably no longer needed since we explicitly unregister the service with the deconstructor
    if (service == m_serviceName && QDBusConnection::sessionBus().registerService(m_serviceName) && sender()) {
        sender()->deleteLater(); // bye doggy :'(
        announceService();
    }
}

DBusInterface::~DBusInterface()
{
    QDBusConnection::sessionBus().unregisterService(m_serviceName);
    // KApplication automatically also grabs org.kde.kwin, so it's often been used externally - ensure to free it as well
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.kde.kwin"));
    if (kwinApp()->x11Connection()) {
        xcb_delete_property(kwinApp()->x11Connection(), kwinApp()->x11RootWindow(), atoms->kwin_dbus_service);
    }
}

void DBusInterface::announceService()
{
    if (!kwinApp()->x11Connection()) {
        return;
    }
    const QByteArray service = m_serviceName.toUtf8();
    xcb_change_property(kwinApp()->x11Connection(), XCB_PROP_MODE_REPLACE, kwinApp()->x11RootWindow(), atoms->kwin_dbus_service,
                        atoms->utf8_string, 8, service.size(), service.constData());
}

// wrap void methods with no arguments to Workspace
#define WRAP(name)                 \
    void DBusInterface::name()     \
    {                              \
        Workspace::self()->name(); \
    }

WRAP(reconfigure)

#undef WRAP

void DBusInterface::killWindow()
{
    Workspace::self()->slotKillWindow();
}

#define WRAP(name)                 \
    void DBusInterface::name()     \
    {                              \
        Placement::self()->name(); \
    }

WRAP(cascadeDesktop)
WRAP(unclutterDesktop)

#undef WRAP

// wrap returning methods with no arguments to Workspace
#define WRAP(rettype, name)               \
    rettype DBusInterface::name()         \
    {                                     \
        return Workspace::self()->name(); \
    }

WRAP(QString, supportInformation)

#undef WRAP

bool DBusInterface::startActivity(const QString &in0)
{
#ifdef KWIN_BUILD_ACTIVITIES
    if (!Activities::self()) {
        return false;
    }
    return Activities::self()->start(in0);
#else

    return false;
#endif
}

bool DBusInterface::stopActivity(const QString &in0)
{
#ifdef KWIN_BUILD_ACTIVITIES
    if (!Activities::self()) {
        return false;
    }
    return Activities::self()->stop(in0);
#else

    return false;
#endif
}

int DBusInterface::currentDesktop()
{
    return VirtualDesktopManager::self()->current();
}

bool DBusInterface::setCurrentDesktop(int desktop)
{
    return VirtualDesktopManager::self()->setCurrent(desktop);
}

void DBusInterface::alwaysShowDesktop()
{
    Workspace::self()->slotAlwaysShowDesktop();
}

void DBusInterface::alwaysRestoreDesktop()
{
    Workspace::self()->slotAlwaysRestoreDesktop();
}

void DBusInterface::nextDesktop()
{
    VirtualDesktopManager::self()->moveTo<DesktopNext>();
}

void DBusInterface::previousDesktop()
{
    VirtualDesktopManager::self()->moveTo<DesktopPrevious>();
}

void DBusInterface::showDebugConsole()
{
    DebugConsole *console = new DebugConsole;
    console->show();
}

void DBusInterface::replace()
{
    QCoreApplication::exit(133);
}

namespace
{
QVariantMap clientToVariantMap(const AbstractClient *c)
{
    return {
        {QStringLiteral("resourceClass"), c->resourceClass()},
        {QStringLiteral("resourceName"), c->resourceName()},
        {QStringLiteral("desktopFile"), c->desktopFileName()},
        {QStringLiteral("role"), c->windowRole()},
        {QStringLiteral("caption"), c->captionNormal()},
        {QStringLiteral("clientMachine"), c->wmClientMachine(true)},
        {QStringLiteral("localhost"), c->isLocalhost()},
        {QStringLiteral("type"), c->windowType()},
        {QStringLiteral("x"), c->x()},
        {QStringLiteral("y"), c->y()},
        {QStringLiteral("width"), c->width()},
        {QStringLiteral("height"), c->height()},
        {QStringLiteral("desktops"), c->desktopIds()},
        {QStringLiteral("minimized"), c->isMinimized()},
        {QStringLiteral("shaded"), c->isShade()},
        {QStringLiteral("fullscreen"), c->isFullScreen()},
        {QStringLiteral("keepAbove"), c->keepAbove()},
        {QStringLiteral("keepBelow"), c->keepBelow()},
        {QStringLiteral("noBorder"), c->noBorder()},
        {QStringLiteral("skipTaskbar"), c->skipTaskbar()},
        {QStringLiteral("skipPager"), c->skipPager()},
        {QStringLiteral("skipSwitcher"), c->skipSwitcher()},
        {QStringLiteral("maximizeHorizontal"), c->maximizeMode() & MaximizeHorizontal},
        {QStringLiteral("maximizeVertical"), c->maximizeMode() & MaximizeVertical},
#ifdef KWIN_BUILD_ACTIVITIES
        {QStringLiteral("activities"), c->activities()},
#endif
    };
}
}

QVariantMap DBusInterface::queryWindowInfo()
{
    m_replyQueryWindowInfo = message();
    setDelayedReply(true);
    kwinApp()->platform()->startInteractiveWindowSelection(
        [this](Toplevel *t) {
        if (auto c = qobject_cast<AbstractClient *>(t)) {
            QDBusConnection::sessionBus().send(m_replyQueryWindowInfo.createReply(clientToVariantMap(c)));
        } else if (qobject_cast<Unmanaged *>(t)) {
            QDBusConnection::sessionBus().send(m_replyQueryWindowInfo.createErrorReply(
                QStringLiteral("org.kde.KWin.Error.InvalidWindow"),
                QStringLiteral("Tried to query information about an unmanaged window")));
        } else {
            QDBusConnection::sessionBus().send(m_replyQueryWindowInfo.createErrorReply(
                QStringLiteral("org.kde.KWin.Error.UserCancel"),
                QStringLiteral("User cancelled the query")));
        }
    });
    return QVariantMap{};
}

QVariantMap DBusInterface::getWindowInfo(const QString &uuid)
{
    const auto id = QUuid::fromString(uuid);
    const auto client = workspace()->findAbstractClient([&id](const AbstractClient *c) {
        return c->internalId() == id;
    });
    if (client) {
        return clientToVariantMap(client);
    } else {
        return {};
    }
}

/*
 * 获取激活窗口的WId
 */
quint32 DBusInterface::getActiveWindowId()
{
    if (!Workspace::self() || !Workspace::self()->activeClient())
        return 0;
    return Workspace::self()->activeClient()->window();
}

/*
 * 获取激活窗口的Pid
 */
quint32 DBusInterface::getActiveWindowPid()
{
    quint32 wid = getActiveWindowId();
    return wid ? getWindowPid(wid) : 0;
}

/*
 * 获取窗口的进程id
 */
quint32 DBusInterface::getWindowPid(quint32 wid)
{
    Toplevel *t = getToplevelFromWid(wid);
    return t ? t->pid() : 0;
}

/*
 * get Wid from Pid
 */
quint32 DBusInterface::getWidFromPid(pid_t pid)
{
    Toplevel *t = nullptr;
    const QList<Toplevel *> &stacking = Workspace::self()->stackingOrder();
    if (stacking.isEmpty()) {
        return 0;
    }
    auto it = stacking.end();
    do {
        --it;
        t = (*it);
        if (t->isDeleted())
            continue;
        if (t->pid() == pid)
            return t->window();
    } while (it != stacking.begin());
    return 0;
}

/*
 * activate window by pid
 */
bool DBusInterface::activeWindowByPid(pid_t pid)
{
    quint32 wid = getWidFromPid(pid);
    if (!wid)
        return false;
    AbstractClient *c = nullptr;
    const QList<AbstractClient *> &allClient = Workspace::self()->allClientList();
    if (allClient.isEmpty()) {
        return false;
    }
    auto it = allClient.end();
    do {
        --it;
        c = (*it);
        if (c->isDeleted())
            continue;
        if (c->window() == wid) {
            if (c->isMinimized()) {
                c->unminimize(true);
            }
            workspace()->activateClient(c);
            return true;
        }
    } while (it != allClient.begin());
    return false;
}

// tablet_blur_background is fullscreen window
// but DO NOT modify this function for it, because "have fullscreen window"
bool DBusInterface::haveFullScreenWindow()
{
    for (const auto c : Workspace::self()->allClientList()) {
        if (c->isFullScreen()) {
            return true;
        }
    }
    return false;
}

// isFullScreen && isActive window
bool DBusInterface::haveFullScreenActiveWindow()
{
    for (const auto c : Workspace::self()->allClientList()) {
        if (c->isActive() && c->isFullScreen()) {
            return true;
        }
    }
    return false;
}

QVariantList DBusInterface::getShortcutInfos()
{
    QVector<Workspace::shortcutInfo> shortcutInfos = workspace()->getShortcutInfos();
    QStringList showShortcutInfos = workspace()->getShowShortcutInfos();
    QVariantList res;
    for (auto s : showShortcutInfos) {
        for (int i = 0; i < shortcutInfos.size(); i++) {
            QStringList tmp;
            if (shortcutInfos[i].actionName == s) {
                tmp.append(shortcutInfos[i].actionName);
                tmp.append(shortcutInfos[i].description);
                QList<QKeySequence> keyList = KGlobalAccel::self()->shortcut(shortcutInfos[i].action);
                if (keyList.isEmpty()) {
                    QList<QKeySequence> defaultKeyList = KGlobalAccel::self()->defaultShortcut(shortcutInfos[i].action);
                    for (int j = 0; j < defaultKeyList.size(); j++) {
                        tmp.append("Null");
                    }
                } else {
                    for (int j = 0; j < keyList.size(); j++) {
                        if (keyList[j].toString() == "") {
                            tmp.append("Null");
                        } else {
                            tmp.append(keyList[j].toString());
                        }
                    }
                    QList<QKeySequence> defaultKeyList = KGlobalAccel::self()->defaultShortcut(shortcutInfos[i].action);
                    if (defaultKeyList.size() != keyList.size()) {
                        for (int j = 0; j < defaultKeyList.size() - keyList.size(); j++) {
                            tmp.append("Null");
                        }
                    }
                }
                res.append(tmp);
            }
        }
    }
    return res;
}

void DBusInterface::modifyShortcut(QString actionName, QString oldKey, QString newKey)
{
    QAction *a = nullptr;
    QList<QKeySequence> defaultShortcut;
    QList<QKeySequence> keyList;
    QVector<Workspace::shortcutInfo> shortcutInfos = workspace()->getShortcutInfos();
    for (int i = 0; i < shortcutInfos.size(); i++) {
        if (shortcutInfos[i].actionName == actionName) {
            a = shortcutInfos[i].action;
            defaultShortcut = shortcutInfos[i].defaultShortcut;
            keyList = KGlobalAccel::self()->shortcut(shortcutInfos[i].action);
            break;
        }
    }

    for (int i = 0; i < keyList.size(); i++) {
        if (keyList[i] == QKeySequence(tr(oldKey.toUtf8().constData()))) {
            keyList[i] = QKeySequence(tr(newKey.toUtf8().constData()));
        }
    }

    if (keyList.isEmpty() && !newKey.isEmpty()) {
        keyList.push_back(QKeySequence(tr(newKey.toUtf8().constData())));
    }

    KGlobalAccel::self()->removeAllShortcuts(a);

    KGlobalAccel::self()->setDefaultShortcut(a, defaultShortcut);
    KGlobalAccel::self()->setShortcut(a, keyList);

    if (!keyList.isEmpty()) {
        TabBox::TabBox::self()->updateGlobalShortcut(a, keyList.first());
    }
}

void DBusInterface::resetShortcut()
{
    // workspace()->readShowShortcutsName();
    QStringList showShortcutInfos = workspace()->getShowShortcutInfos();
    QVector<Workspace::shortcutInfo> shortcutInfos = workspace()->getShortcutInfos();
    for (auto s : showShortcutInfos) {
        for (int i = 0; i < shortcutInfos.size(); i++) {
            if (shortcutInfos[i].actionName == s) {
                KGlobalAccel::self()->removeAllShortcuts(shortcutInfos[i].action);
                KGlobalAccel::self()->setDefaultShortcut(shortcutInfos[i].action, shortcutInfos[i].defaultShortcut);
                KGlobalAccel::self()->setShortcut(shortcutInfos[i].action, shortcutInfos[i].defaultShortcut);
                TabBox::TabBox::self()->updateGlobalShortcut(shortcutInfos[i].action, shortcutInfos[i].defaultShortcut.first());
            }
        }
    }
}

void DBusInterface::blockShortcut(QString actionName, bool flag)
{
    QVector<Workspace::shortcutInfo> shortcutInfos = workspace()->getShortcutInfos();
    for (auto info : shortcutInfos) {
        if (info.actionName == actionName) {
            if (flag) {
                KGlobalAccel::self()->removeAllShortcuts(info.action);
            } else {
                KGlobalAccel::self()->setShortcut(info.action, info.defaultShortcut);
            }
        }
    }
}

void DBusInterface::setTapButtonMap(quint32 map)
{
    for (InputDevice *inputDevice : input()->devices()) {
        KWin::LibInput::Device *device = dynamic_cast<KWin::LibInput::Device *>(inputDevice);
        if (!device) {
            continue;
        }
        if (!device->isTouchpad()) {
            continue;
        }
        device->setTapButtonMap(map);
    }
}

/*
 * 根据wid获取到Toplevel指针
 */
Toplevel *DBusInterface::getToplevelFromWid(const quint32 wid)
{
    Toplevel *t = nullptr;
    const QList<Toplevel *> &stacking = Workspace::self()->stackingOrder();
    if (stacking.isEmpty()) {
        return nullptr;
    }
    auto it = stacking.end();
    do {
        --it;
        t = (*it);
        if (t->isDeleted())
            continue;
        if (t->window() == wid)
            return t;
    } while (it != stacking.begin());
    return nullptr;
}

CompositorDBusInterface::CompositorDBusInterface(Compositor *parent)
    : QObject(parent)
    , m_compositor(parent)
{
    connect(m_compositor, &Compositor::compositingToggled, this, &CompositorDBusInterface::compositingToggled);
    new CompositingAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/Compositor"), this);
    dbus.connect(QString(), QStringLiteral("/Compositor"), QStringLiteral("org.kde.kwin.Compositing"),
                 QStringLiteral("reinit"), this, SLOT(reinitialize()));
}

QString CompositorDBusInterface::compositingNotPossibleReason() const
{
    return kwinApp()->platform()->compositingNotPossibleReason();
}

QString CompositorDBusInterface::compositingType() const
{
    if (!m_compositor->compositing()) {
        return QStringLiteral("none");
    }
    switch (m_compositor->backend()->compositingType()) {
    case OpenGLCompositing:
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
            return QStringLiteral("gles");
        } else {
            return QStringLiteral("gl2");
        }
    case QPainterCompositing:
        return QStringLiteral("qpainter");
    case NoCompositing:
    default:
        return QStringLiteral("none");
    }
}

bool CompositorDBusInterface::isActive() const
{
    return m_compositor->isActive();
}

bool CompositorDBusInterface::isCompositingPossible() const
{
    return kwinApp()->platform()->compositingPossible();
}

bool CompositorDBusInterface::isOpenGLBroken() const
{
    return kwinApp()->platform()->openGLCompositingIsBroken();
}

bool CompositorDBusInterface::platformRequiresCompositing() const
{
    return kwinApp()->platform()->requiresCompositing();
}

void CompositorDBusInterface::resume()
{
    if (kwinApp()->operationMode() == Application::OperationModeX11) {
        static_cast<X11Compositor *>(m_compositor)->resume(X11Compositor::ScriptSuspend);
    }
}

void CompositorDBusInterface::suspend()
{
    if (kwinApp()->operationMode() == Application::OperationModeX11) {
        static_cast<X11Compositor *>(m_compositor)->suspend(X11Compositor::ScriptSuspend);
    }
}

void CompositorDBusInterface::reinitialize()
{
    m_compositor->reinitialize();
}

QStringList CompositorDBusInterface::supportedOpenGLPlatformInterfaces() const
{
    QStringList interfaces;
    bool supportsGlx = false;
#if HAVE_EPOXY_GLX
    supportsGlx = (kwinApp()->operationMode() == Application::OperationModeX11);
#endif
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        supportsGlx = false;
    }
    if (supportsGlx) {
        interfaces << QStringLiteral("glx");
    }
    interfaces << QStringLiteral("egl");
    return interfaces;
}

VirtualDesktopManagerDBusInterface::VirtualDesktopManagerDBusInterface(VirtualDesktopManager *parent)
    : QObject(parent)
    , m_manager(parent)
{
    qDBusRegisterMetaType<KWin::DBusDesktopDataStruct>();
    qDBusRegisterMetaType<KWin::DBusDesktopDataVector>();

    new VirtualDesktopManagerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/VirtualDesktopManager"),
                                                 QStringLiteral("org.kde.KWin.VirtualDesktopManager"),
                                                 this);

    connect(m_manager, &VirtualDesktopManager::currentChanged, this,
            [this](uint previousDesktop, uint newDesktop) {
        
        
        Q_EMIT currentChanged(m_manager->currentDesktop()->id());
    });

    connect(m_manager, &VirtualDesktopManager::countChanged, this,
            [this](uint previousCount, uint newCount) {
        
        Q_EMIT countChanged(newCount);
        Q_EMIT desktopsChanged(desktops());
    });

    connect(m_manager, &VirtualDesktopManager::navigationWrappingAroundChanged, this,
            [this]() {
        Q_EMIT navigationWrappingAroundChanged(isNavigationWrappingAround());
    });

    connect(m_manager, &VirtualDesktopManager::rowsChanged, this, &VirtualDesktopManagerDBusInterface::rowsChanged);

    const QVector<VirtualDesktop *> allDesks = m_manager->desktops();
    for (auto *vd : allDesks) {
        connect(vd, &VirtualDesktop::x11DesktopNumberChanged, this,
                [this, vd]() {
            DBusDesktopDataStruct data{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
            Q_EMIT desktopDataChanged(vd->id(), data);
            Q_EMIT desktopsChanged(desktops());
        });
        connect(vd, &VirtualDesktop::nameChanged, this,
                [this, vd]() {
            DBusDesktopDataStruct data{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
            Q_EMIT desktopDataChanged(vd->id(), data);
            Q_EMIT desktopsChanged(desktops());
        });
    }
    connect(m_manager, &VirtualDesktopManager::desktopCreated, this,
            [this](VirtualDesktop *vd) {
        connect(vd, &VirtualDesktop::x11DesktopNumberChanged, this,
                [this, vd]() {
            DBusDesktopDataStruct data{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
            Q_EMIT desktopDataChanged(vd->id(), data);
            Q_EMIT desktopsChanged(desktops());
        });
        connect(vd, &VirtualDesktop::nameChanged, this,
                [this, vd]() {
            DBusDesktopDataStruct data{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
            Q_EMIT desktopDataChanged(vd->id(), data);
            Q_EMIT desktopsChanged(desktops());
        });
        DBusDesktopDataStruct data{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
        Q_EMIT desktopCreated(vd->id(), data);
        Q_EMIT desktopsChanged(desktops());
    });
    connect(m_manager, &VirtualDesktopManager::desktopRemoved, this,
            [this](VirtualDesktop *vd) {
        Q_EMIT desktopRemoved(vd->id());
        Q_EMIT desktopsChanged(desktops());
    });
}

uint VirtualDesktopManagerDBusInterface::count() const
{
    return m_manager->count();
}

void VirtualDesktopManagerDBusInterface::setRows(uint rows)
{
    if (static_cast<uint>(m_manager->grid().height()) == rows) {
        return;
    }

    m_manager->setRows(rows);
    m_manager->save();
}

uint VirtualDesktopManagerDBusInterface::rows() const
{
    return m_manager->rows();
}

void VirtualDesktopManagerDBusInterface::setCurrent(const QString &id)
{
    if (m_manager->currentDesktop()->id() == id) {
        return;
    }

    auto *vd = m_manager->desktopForId(id);
    if (vd) {
        m_manager->setCurrent(vd);
    }
}

QString VirtualDesktopManagerDBusInterface::current() const
{
    return m_manager->currentDesktop()->id();
}

void VirtualDesktopManagerDBusInterface::setNavigationWrappingAround(bool wraps)
{
    if (m_manager->isNavigationWrappingAround() == wraps) {
        return;
    }

    m_manager->setNavigationWrappingAround(wraps);
}

bool VirtualDesktopManagerDBusInterface::isNavigationWrappingAround() const
{
    return m_manager->isNavigationWrappingAround();
}

DBusDesktopDataVector VirtualDesktopManagerDBusInterface::desktops() const
{
    const auto desks = m_manager->desktops();
    DBusDesktopDataVector desktopVect;
    desktopVect.reserve(m_manager->count());

    std::transform(desks.constBegin(), desks.constEnd(),
                   std::back_inserter(desktopVect),
                   [](const VirtualDesktop *vd) {
        return DBusDesktopDataStruct{.position = vd->x11DesktopNumber() - 1, .id = vd->id(), .name = vd->name()};
    });

    return desktopVect;
}

void VirtualDesktopManagerDBusInterface::createDesktop(uint position, const QString &name)
{
    m_manager->createVirtualDesktop(position, name);
}

void VirtualDesktopManagerDBusInterface::setDesktopName(const QString &id, const QString &name)
{
    VirtualDesktop *vd = m_manager->desktopForId(id);
    if (!vd) {
        return;
    }
    if (vd->name() == name) {
        return;
    }

    vd->setName(name);
    m_manager->save();
}

void VirtualDesktopManagerDBusInterface::removeDesktop(const QString &id)
{
    m_manager->removeVirtualDesktop(id);
}

PluginManagerDBusInterface::PluginManagerDBusInterface(PluginManager *manager)
    : QObject(manager)
    , m_manager(manager)
{
    new PluginsAdaptor(this);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Plugins"),
                                                 QStringLiteral("org.kde.KWin.Plugins"),
                                                 this);
}

QStringList PluginManagerDBusInterface::loadedPlugins() const
{
    return m_manager->loadedPlugins();
}

QStringList PluginManagerDBusInterface::availablePlugins() const
{
    return m_manager->availablePlugins();
}

bool PluginManagerDBusInterface::LoadPlugin(const QString &name)
{
    return m_manager->loadPlugin(name);
}

void PluginManagerDBusInterface::UnloadPlugin(const QString &name)
{
    m_manager->unloadPlugin(name);
}
TouchGestureManagerDBusInterface *TouchGestureManagerDBusInterface::s_self = nullptr;
TouchGestureManagerDBusInterface::TouchGestureManagerDBusInterface(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/TouchGestureManager"),
                                                 QStringLiteral("org.ukui.KWin.TouchGestureManager"),
                                                 this,
                                                 QDBusConnection::ExportAllSignals);
}

TouchGestureManagerDBusInterface::~TouchGestureManagerDBusInterface()
{
    if (s_self) {
        delete s_self;
        s_self = nullptr;
    }
}

TouchGestureManagerDBusInterface *
TouchGestureManagerDBusInterface::self()
{
    if (!s_self)
        s_self = new TouchGestureManagerDBusInterface();
    return s_self;
}
void TouchGestureManagerDBusInterface::sendGestureSig(GestureType gesture,
                                                      GlobalGesture::GestureDirection direction,
                                                      int fingerCount)
{
    qDebug() << "Gesture emit signal:" << gestureTypeToStr(gesture)
             << "direction: " << gestureDirectionToStr(direction)
             << "fingerCount: " << fingerCount;
    switch (gesture) {
    case GestureType::EDGESWIPE:
        Q_EMIT TouchGestureManagerDBusInterface::edgeSwipeGesture(gestureDirectionToStr(direction), fingerCount);
        break;
    case GestureType::SWIPE:
        Q_EMIT TouchGestureManagerDBusInterface::swipeGesture(gestureDirectionToStr(direction), fingerCount);
        break;
    case GestureType::PINCH:
        Q_EMIT TouchGestureManagerDBusInterface::pinchGesture(gestureDirectionToStr(direction), fingerCount);
        break;
    case GestureType::LONGPRESS:
        Q_EMIT TouchGestureManagerDBusInterface::longpressGesture();
        break;
    default:
        break;
    }
}

} // namespace
