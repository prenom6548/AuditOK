#include "systemstatemanager.h"
#include "waylanddisplay.h"
#include "loghelper.h"

#include <QDBusConnection>
#include <QGuiApplication>
#include <QTextStream>
#include <QScreen>
#include <QFile>
#include <QDebug>

namespace {
const char* SYS_CONF = "/etc/os-release";

const char *gnome_service = "org.gnome.SessionManager";
const char *gnome_path = "/org/gnome/SessionManager/Presence";
const char *gnome_interface = "org.gnome.SessionManager.Presence";

const char *daemon_dbus_service = "com.kylin.ProcessManagerDaemon";
const char *daemon_dbus_path = "/com/kylin/ProcessManagerDaemon";
const char *daemon_dbus_interface = "com.kylin.ProcessManagerDaemon";

const char *free_desktop_login_service = "org.freedesktop.login1";
const char *free_desktop_login_path = "/org/freedesktop/login1";
const char *free_desktop_login_interface = "org.freedesktop.login1.Manager";

}

SystemStateManager::SystemStateManager(const std::shared_ptr<StateManager> &stateManager)
    : m_systemVersion("")
    , m_currScreenMode(sched_policy::ScreenMode::UNKNOWN)
    , m_lastScreenMode(sched_policy::ScreenMode::UNKNOWN)
    , m_currInputMode(sched_policy::InputMode::InputBusy)
    , m_lastInputMode(sched_policy::InputMode::Unknown)
    , m_isWaylandPlatform(false)
    , m_stateManager(stateManager)
{
    initSystemVersion();
    initIsWaylandPlatform();
    m_resourceTimer.setSingleShot(true);
    m_resourceTimer.setInterval(5000);
    m_stateManager->enableTarget(StateTarget::InputMode);
}

SystemStateManager::~SystemStateManager()
{
    PRINT_NOTICE("~SystemStateManager\n");
    stopScreenWatcher();
}

sched_policy::ScreenMode SystemStateManager::currScreenMode()
{
    return m_currScreenMode;
}

sched_policy::ScreenMode SystemStateManager::lastScreenMode()
{
    return m_lastScreenMode;
}

std::string SystemStateManager::systemVersion()
{
    return m_systemVersion;
}

bool SystemStateManager::isWaylandPlatform()
{
    return m_isWaylandPlatform;
}

sched_policy::StatusMode SystemStateManager::currStatusMode()
{
    return m_currStatusMode;
}

sched_policy::StatusMode SystemStateManager::lastStatusMode()
{
    return m_lastStatusMode;
}

sched_policy::InputMode SystemStateManager::currInputMode()
{
    return m_currInputMode;
}

sched_policy::InputMode SystemStateManager::lastInputMode()
{
    return m_lastInputMode;
}

std::unordered_map<std::string, int> &SystemStateManager::resourceThreshold()
{
    return m_resourceThreshold;
}

void SystemStateManager::startScreenWatcher()
{
    if (m_isWaylandPlatform) {
        screen_mode_conn = connect(waylandDisplay(), &WaylandDisplay::screenModeChanged, this, QOverload<int>::of(&SystemStateManager::onScreenModeChanged));
        onScreenModeChanged(waylandDisplay()->getScreenMode());
    } else {
        initScreenMode();
    }
}

void SystemStateManager::stopScreenWatcher()
{
    if (m_isWaylandPlatform) {
        if (screen_mode_conn) {
            disconnect(screen_mode_conn);
            screen_mode_conn = {};
        }
    } else {
        if (screen_add_conn) {
            disconnect(screen_add_conn);
            screen_add_conn = {};
        }

        if (screen_remove_conn) {
            disconnect(screen_remove_conn);
            screen_remove_conn = {};
        }
        for (auto &screen : m_screens) {
            disconnect(screen);
            screen = {};
        }
        m_screens.clear();
    }
}

void SystemStateManager::startFreezeWatcher()
{
    initStatusMode();
}

void SystemStateManager::stopFreezeWatcher()
{
    QDBusConnection::sessionBus().disconnect(
        gnome_service,gnome_path, gnome_interface,
        "StatusChanged", this, SLOT(onStatusModeChangedAcpuired(uint)));
    QDBusConnection::systemBus().disconnect(
        daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface,
        "IdleStatus", this, SLOT(onInputIdleChangeAcpuired(bool)));
    QDBusConnection::systemBus().disconnect(
        free_desktop_login_service, free_desktop_login_path, free_desktop_login_interface,
        "PrepareForSleep", this, SLOT(onPrepareForSleepAcpuired(bool)));
    QDBusConnection::systemBus().disconnect(
        free_desktop_login_service, free_desktop_login_path, free_desktop_login_interface,
        "PrepareForShutdown", this, SLOT(onPrepareForShutdownAcpuired(bool)));
}

void SystemStateManager::resourceWarning(const std::string &resource, int level)
{
    if (m_resourceTimer.isActive()) {
        m_resourceTimer.start();
        return;
    } else {
        m_resourceTimer.start();
    }
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceThreshold));
    m_resourceThreshold[resource] = level;
    m_stateManager->enableTarget(StateTarget::ResourceThreshold);
}

void SystemStateManager::onScreenModeChanged()
{
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ScreenMode));
    m_lastScreenMode = m_currScreenMode;
    m_currScreenMode = sched_policy::ScreenMode::UNKNOWN;
    QList<QScreen *> screens = QGuiApplication::screens();
    //qDebug() << screens.count();
    if (screens.count() == 0) {
        // qDebug() << "No screens detected.";
        PRINT_DBG("No screens detected.\n");
        m_currScreenMode = sched_policy::ScreenMode::NO_SCREEN;
    }

    if (screens.count() == 1) {
        // qDebug() << "Single screen detected.";
        PRINT_DBG("Single screen detected.\n");
        m_currScreenMode = sched_policy::ScreenMode::SINGLE_SCREEN;
    }
    
    if (screens.count() > 1) {
        // 检查屏幕的几何位置和大小
        bool isExtendedMode = true;
        QRect primaryScreenGeometry = screens[0]->geometry();
        for (int i = 1; i < screens.count(); ++i) {
            QRect currentScreenGeometry = screens[i]->geometry();
            if (primaryScreenGeometry.intersects(currentScreenGeometry)) {
                isExtendedMode = false;
                break;
            }
        }

        if (isExtendedMode) {
            // qDebug() << "Extended mode detected.";
            PRINT_DBG("Extended mode detected.");
            m_currScreenMode = sched_policy::ScreenMode::EXTEND_SCREEN;
        } else {
            // qDebug() << "Copy mode detected.";
            PRINT_DBG("Copy mode detected.\n");
            m_currScreenMode = sched_policy::ScreenMode::COPY_SCREEN;
        }
    }

    // qDebug() << "Current Screen Mode:" << static_cast<int>(m_currScreenMode);
    PRINT_DBG("Current Screen Mode: %d\n", static_cast<int>(m_currScreenMode));
    m_stateManager->enableTarget(StateTarget::ScreenMode);
}

void SystemStateManager::onScreenModeChanged(int screenMode)
{
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ScreenMode));
    m_lastScreenMode = m_currScreenMode;
    m_currScreenMode = sched_policy::ScreenMode(screenMode);

    m_stateManager->enableTarget(StateTarget::ScreenMode);
}

void SystemStateManager::onStatusModeChangedAcpuired(uint status)
{
    sched_policy::StatusMode statusMode = sched_policy::StatusMode::Unknown;
    switch (status)
    {
    case 0:
        statusMode = sched_policy::StatusMode::Available;
        break;
    case 1:
        statusMode = sched_policy::StatusMode::Invisible;
        break;
    case 2:
        statusMode = sched_policy::StatusMode::Busy;
        break;
    case 3:
        statusMode = sched_policy::StatusMode::Idle;
        break;
    default:
        break;
    }

    PRINT_NOTICE("SystemStateManager::onStatusModeChangedAcpuired: %d\n", (int)statusMode);
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::StatusMode));
    m_lastStatusMode = m_currStatusMode;
    m_currStatusMode = statusMode;
    m_stateManager->enableTarget(StateTarget::StatusMode);
}

void SystemStateManager::onInputIdleChangeAcpuired(bool state)
{
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::InputMode));
    sched_policy::InputMode inputMode = state ? sched_policy::InputMode::InputIdle : sched_policy::InputMode::InputBusy;

    m_lastInputMode = m_currInputMode;
    m_currInputMode = inputMode;

    PRINT_NOTICE("SystemStateManager::onInputIdleChangeAcpuired: %d\n", (int)inputMode);
    m_stateManager->enableTarget(StateTarget::InputMode);
}

void SystemStateManager::onPrepareForSleepAcpuired(bool state)
{
    if (state) {
        PRINT_DBG("The system is about to enter a sleep/suspend state\n");
        m_stateManager->enableTarget(StateTarget::PrepareForSleep);
        m_cgroupControlInterface.setFreezeEnable(false);
    } else {
        m_login1DbusInterface.inhibit();
        m_cgroupControlInterface.setFreezeEnable(true);
        PRINT_DBG("The system has resumed from sleep/suspend state\n");
    }
}

void SystemStateManager::onPrepareForShutdownAcpuired(bool state)
{
    PRINT_NOTICE("onPrepareForShutdownAcpuired\n");
    m_cgroupControlInterface.setFreezeEnable(false);
    m_stateManager->enableTarget(StateTarget::PrepareForShutdown);
}

void SystemStateManager::initSystemVersion()
{
    QFile file(QString::fromStdString(SYS_CONF));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "Cannot open file for reading:" << SYS_CONF;
        PRINT_NOTICE("Cannot open file for reading: %s\n", SYS_CONF);
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // 去除行尾的空白字符
        line = line.trimmed();
        // 检查是否为注释行（以#开头）
        if (!line.startsWith("#")) {
            // 处理非注释行，例如分割并处理数据
            QStringList parts = line.split("="); // 这里可以根据需要使用其他分隔符或直接分割
            if (!parts.isEmpty()) {
                //qDebug() << "Processed line:" << parts.first() << " = " << parts.at(1); // 处理第一部分的逻辑
                if(parts.first() == "VERSION_ID") {
                    QString version = parts.at(1);
                    m_systemVersion = version.remove("\"").toStdString();
                }
            } 

        }
    }
}

void SystemStateManager::initIsWaylandPlatform()
{
    QByteArray xdgSessionTypeValue = qgetenv("XDG_SESSION_TYPE");
    m_isWaylandPlatform = xdgSessionTypeValue.compare("wayland", Qt::CaseInsensitive) == 0;
}

void SystemStateManager::initScreenMode()
{
    QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens){
        m_screens.insert(screen, QObject::connect(screen, &QScreen::geometryChanged, this, QOverload<>::of(&SystemStateManager::onScreenModeChanged)));
    }

    screen_add_conn = connect(qApp, &QGuiApplication::screenAdded, this, [this](QScreen *screen) {
        //qDebug() << "New screen added:" << screen->name();
        PRINT_DBG("New screen added: %s\n", screen->name().toStdString().c_str());
        if (!m_screens.contains(screen)) {
            m_screens.insert(screen, connect(screen, &QScreen::geometryChanged, this, QOverload<>::of(&SystemStateManager::onScreenModeChanged))); 
        }
        Q_EMIT onScreenModeChanged();
    });

    screen_remove_conn = QObject::connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen *screen) {
        if (m_screens.contains(screen))  {
            disconnect(m_screens[screen]);
            m_screens.remove(screen);
        }
        Q_EMIT onScreenModeChanged();
    });
    Q_EMIT onScreenModeChanged();
}

void SystemStateManager::initStatusMode()
{
    QDBusConnection::sessionBus().connect(
        gnome_service,gnome_path, gnome_interface,
        "StatusChanged", this, SLOT(onStatusModeChangedAcpuired(uint)));

    QDBusConnection::systemBus().connect(
        daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface,
        "IdleStatus", this, SLOT(onInputIdleChangeAcpuired(bool)));

    QDBusConnection::systemBus().connect(
        free_desktop_login_service, free_desktop_login_path, free_desktop_login_interface,
        "PrepareForSleep", this, SLOT(onPrepareForSleepAcpuired(bool)));

    QDBusConnection::systemBus().connect(
        free_desktop_login_service, free_desktop_login_path, free_desktop_login_interface,
        "PrepareForShutdown", this, SLOT(onPrepareForShutdownAcpuired(bool)));
}
