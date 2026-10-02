#include "screen-manager.h"
#include <QApplication>
#include <QDebug>
#include <QGuiApplication>

ScreenManager* ScreenManager::m_instance = nullptr;

ScreenManager* ScreenManager::getInstance()
{
    if (!m_instance) {
        m_instance = new ScreenManager();
        qAddPostRoutine([](){
            delete m_instance;
            m_instance = nullptr;
        });
    }
    return m_instance;
}

ScreenManager::ScreenManager(QObject *parent) : QObject(parent),
    m_displayMode(Unknown),
    m_lastScreenId(0)
{
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreenManager::handleScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreenManager::handleScreenRemoved);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &ScreenManager::handlePrimaryScreenChanged);

    initScreens();
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::initScreens()
{
    m_screenIds.clear();
    m_screens.clear();

    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen == QGuiApplication::primaryScreen()) {
            // 确保主屏幕ID为0
            m_screenIds[screen] = 0;
            m_screens[0] = screen;
        } else {
            // 分配自增ID
            int id = generateScreenId();
            m_screenIds[screen] = id;
            m_screens[id] = screen;
        }

        connect(screen, &QScreen::geometryChanged, this, [=](const QRect &geometry) {
            handleScreenGeometryChanged(screen, geometry);
        });
        qDebug() << "init screen "<<screen->name()<<screen->geometry()<<m_screenIds[screen] << "primaryScreen:"<<QGuiApplication::primaryScreen()->name();
    }

    updateDisplayMode();
    updateVirtualDesktopGeometry();
}

int ScreenManager::generateScreenId()
{
    // 找到一个未使用的ID
    m_lastScreenId = 0;
    while (m_screens.contains(m_lastScreenId)) {
        m_lastScreenId++;
    }
    return m_lastScreenId;
}

bool ScreenManager::updateDisplayMode()
{
    DisplayMode oldMode = m_displayMode;

    if (screenCount() == 1) {
        m_displayMode = Single;
    } else {
        // 检查所有屏幕是否有相同的几何形状（镜像）
        bool allSameGeometry = true;
        QRect firstGeometry = m_screens.value(0)->geometry();
        for (auto it = m_screens.begin(); it != m_screens.end(); ++it) {
            qDebug()<<"DisplayMode get screen Geometry"<<it.value()->geometry();
            if (it.value()->geometry() != firstGeometry) {
                allSameGeometry = false;
                break;
            }
        }

        m_displayMode = allSameGeometry ? Mirror : Extend;
    }

    qDebug()<<"update mode: oldmode"<<oldMode<<"display mode"<<m_displayMode;

    if (oldMode != m_displayMode) {
        Q_EMIT displayModeChanged();

        // 根据模式切换进行处理
        if (m_displayMode == Single || m_displayMode == Mirror) {
            switchToSingleScreenMode();
        } else if (m_displayMode == Extend) {
            switchToMultiScreenMode();
        }
    }
    return oldMode == m_displayMode;
}

void ScreenManager::updateVirtualDesktopGeometry()
{
    QRect oldGeometry = m_virtualDesktopGeometry;

    // 计算所有屏幕的边界矩形
    QRegion region;
    for (QScreen* screen : QGuiApplication::screens()) {
        region += screen->geometry();
    }

    m_virtualDesktopRegion = region;
    m_virtualDesktopGeometry = region.boundingRect();

    if (oldGeometry != m_virtualDesktopGeometry) {
        Q_EMIT virtualDesktopGeometryChanged();
    }
}

QScreen* ScreenManager::primaryScreen() const
{
    return QGuiApplication::primaryScreen();
}

bool ScreenManager::isPrimaryScreen(QScreen *screen) const
{
    return primaryScreen() == screen;
}

QList<QScreen*> ScreenManager::allScreens() const
{
    return QGuiApplication::screens();
}

int ScreenManager::screenCount() const
{
    return QGuiApplication::screens().count();
}

int ScreenManager::displayMode() const
{
    return m_displayMode;
}

QRect ScreenManager::virtualDesktopGeometry() const
{
    return m_virtualDesktopGeometry;
}

QRegion ScreenManager::virtualDesktopRegion() const
{
    return m_virtualDesktopRegion;
}

QVariantMap ScreenManager::getScreenInfo(int screenId) const
{
    QVariantMap info;
    QScreen* screen = getScreenById(screenId);

    if (!screen) return info;

    info["id"] = screenId;
    info["name"] = screen->name();
    info["geometry"] = screen->geometry();
    info["availableGeometry"] = screen->availableGeometry();
    info["size"] = screen->size();
    info["physicalSize"] = screen->physicalSize();
    info["logicalDotsPerInch"] = screen->logicalDotsPerInch();
    info["physicalDotsPerInch"] = screen->physicalDotsPerInch();
    info["refreshRate"] = screen->refreshRate();
    info["isPrimary"] = (screen == primaryScreen());

    return info;
}

QScreen* ScreenManager::getScreenById(int screenId) const
{
    return m_screens.value(screenId, nullptr);
}

int ScreenManager::getScreenId(QScreen* screen) const
{
    return m_screenIds.value(screen, -1);
}

int ScreenManager::getPrimaryScreenId() const
{
    return m_screenIds.value(primaryScreen(), 0);
}

QRect ScreenManager::getScreenGeometry(int screenId) const
{
    QScreen* screen = getScreenById(screenId);
    return screen ? screen->geometry() : QRect();
}

QPoint ScreenManager::getScreenPosition(int screenId) const
{
    QScreen* screen = getScreenById(screenId);
    return screen ? screen->geometry().topLeft() : QPoint();
}

QSize ScreenManager::getScreenSize(int screenId) const
{
    QScreen* screen = getScreenById(screenId);
    return screen ? screen->size() : QSize();
}

qreal ScreenManager::getScreenScaleFactor(int screenId) const
{
    QScreen* screen = getScreenById(screenId);
    return screen ? screen->devicePixelRatio() : 1.0;
}

int ScreenManager::getScreenIdAt(const QPoint& pos) const
{
    if (m_displayMode == Mirror) return 0;
    for (auto it = m_screens.begin(); it != m_screens.end(); ++it) {
        if (it.value()->geometry().contains(pos)) {
            return it.key();
        }
    }
    return -1;
}

QPoint ScreenManager::mapToGlobal(int screenId, const QPoint& pos) const
{
    QScreen* screen = getScreenById(screenId);
    if (!screen) return pos;

    return screen->geometry().topLeft() + pos;
}

QPoint ScreenManager::mapFromGlobal(int screenId, const QPoint& pos) const
{
    QScreen* screen = getScreenById(screenId);
    if (!screen) return pos;

    return pos - screen->geometry().topLeft();
}

QList<QWindow*> ScreenManager::getBackgroundWindows() const
{
    return m_backgroundWindows.values();
}

void ScreenManager::relocateWindows()
{
    Q_EMIT windowsRelocateRequired();
}

void ScreenManager::switchToSingleScreenMode()
{
    Q_EMIT singleScreenModeActivated();
}

void ScreenManager::switchToMultiScreenMode()
{
    Q_EMIT multiScreenModeActivated();
}

int ScreenManager::checkScreenMode(const QRect& geometry)
{
    if (screenCount() == 1) {
        return Single;
    }

    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen->geometry() != geometry) {
            return Extend;
        }
    }

    return Mirror;
}

void ScreenManager::handleScreenAdded(QScreen* screen)
{
    if (!screen) return;
    if (screen->name().isEmpty()) return;
    // 防止同一屏幕因 Wayland compositor 初始化时序问题被多次注册
    if (m_screenIds.contains(screen)) {
        qWarning() << "[ScreenManager] handleScreenAdded: screen already registered, skip:" << screen->name();
        return;
    }
    qDebug() << "Screen added:" << screen->name()<<m_screens;

    //screen的信号不及时，暂时先校验screen大小是否与桌面大小一致
    for (auto it = m_backgroundWindows.begin(); it != m_backgroundWindows.end(); ++it) {
        QWindow *view = it.value();
        int screenId = it.key();
        QScreen *screen = m_screens.value(screenId);
        if (view && screen && view->geometry() != screen->geometry()) {
            view->setGeometry(screen->geometry());
        }
    }

    // 为新屏幕分配ID
    int id = generateScreenId();
    m_screenIds[screen] = id;
    m_screens[id] = screen;

    // 连接屏幕几何变化信号
    connect(screen, &QScreen::geometryChanged, this, [=](const QRect &geometry) {
        handleScreenGeometryChanged(screen, geometry);
    });

    // 更新显示模式和虚拟桌面几何形状
    updateDisplayMode();
    updateVirtualDesktopGeometry();

    Q_EMIT screenAdded(screen, id);
    Q_EMIT screenCountChanged();
}

void ScreenManager::handleScreenRemoved(QScreen* screen)
{
    if (!screen) return;

    qDebug() << "Screen removed:" << screen->name();

    // 获取屏幕ID，并从映射中删除
    int id = m_screenIds.value(screen, -1);
    if (id >= 0) {
        m_screenIds.remove(screen);
        m_screens.remove(id);

        if (m_backgroundWindows.contains(id)) {
            QWindow* window = m_backgroundWindows.take(id);
            Q_EMIT windowRemoved(window, id);
        }
    }

    // 更新显示模式和虚拟桌面几何形状
    DisplayMode oldmode = m_displayMode;
    updateDisplayMode();
    updateVirtualDesktopGeometry();
    if(oldmode == Extend && m_displayMode == Single){
        Q_EMIT screenRemovedFromExtend(screen, id);
    }
    Q_EMIT screenRemoved(screen, id);
    Q_EMIT screenCountChanged();
}

void ScreenManager::handlePrimaryScreenChanged(QScreen* screen)
{
    if (!screen) return;
    if (screen->name().isEmpty()) return;

    qDebug() << "Primary screen changed to:" << screen->name();

    // 确保主屏幕ID为0
    int newPrimaryId = m_screenIds.value(screen, -1);

    if (newPrimaryId < 0) {
        // 新主屏幕还没有ID，分配ID 0
        m_screenIds[screen] = 0;
        m_screens[0] = screen;
    } else if (newPrimaryId != 0) {
        // 交换ID
        QScreen* oldPrimary = m_screens.value(0);
        if (oldPrimary) {
            m_screenIds[oldPrimary] = newPrimaryId;
            m_screens[newPrimaryId] = oldPrimary;
        }

        m_screenIds[screen] = 0;
        m_screens[0] = screen;

        // 交换窗口
        QWindow* oldPrimaryWindow = m_backgroundWindows.value(0);
        QWindow* newPrimaryWindow = m_backgroundWindows.value(newPrimaryId);

        if (oldPrimaryWindow && newPrimaryWindow) {
            // 两个窗口都存在，执行交换
            qDebug() << "Swapping background windows between position 0 and" << newPrimaryId<<newPrimaryWindow;
            m_backgroundWindows[0] = newPrimaryWindow;
            m_backgroundWindows[newPrimaryId] = oldPrimaryWindow;
            newPrimaryWindow->requestActivate();
        } else if (oldPrimaryWindow || newPrimaryWindow) {
            if (oldPrimaryWindow) {
                qDebug() << "Moving old primary window from position 0 to" << newPrimaryId;
                m_backgroundWindows[newPrimaryId] = oldPrimaryWindow;
                m_backgroundWindows.remove(0);
            } else {
                qDebug() << "Moving new primary window from position" << newPrimaryId << "to primary position 0";
                m_backgroundWindows[0] = newPrimaryWindow;
                m_backgroundWindows.remove(newPrimaryId);
            }

            Q_EMIT resetWindowIds(oldPrimary, newPrimaryId, screen, 0);
            return;
        } else {
            qCritical() << "Both windows are null! This should not happen.";
        }
    }

    Q_EMIT primaryScreenChanged();
}

void ScreenManager::handleScreenGeometryChanged(QScreen *screen, const QRect &geometry)
{
    qDebug() << "Screen geometry changed:" << geometry;
    if (!screen)
        return;

    int id = m_screenIds.value(screen);
    QWindow *view = m_backgroundWindows.value(id);
    bool geometryChanged = false;
    if (view && view->geometry() != geometry) {
        view->setGeometry(geometry);
        geometryChanged = true;
    }
    bool modeChanged = updateDisplayMode();
    if (modeChanged && geometryChanged) {
        windowsRelocateRequired();
    }
    updateVirtualDesktopGeometry();
}

void ScreenManager::addBackgroundWindow(QWindow* window, QScreen* screen, int id)
{
    if (!window || !screen) return;

    int screenId = (id >= 0) ? id : getScreenId(screen);

    if (screenId < 0) {
        screenId = generateScreenId();
        m_screenIds[screen] = screenId;
        m_screens[screenId] = screen;
    }

    m_backgroundWindows[screenId] = window;

    Q_EMIT windowAdded(window, screenId);
}

QWindow *ScreenManager::getWindowAt(const QPoint& pos) const
{
    int id = m_displayMode == Mirror? 0 : getScreenIdAt(pos);
    QWindow *window = m_backgroundWindows.value(id, nullptr);
    if (!window) {
        window = m_backgroundWindows.value(0);
    }
    return window;
}

QWindow *ScreenManager::getWindowById(int screenId) const
{
    QWindow *window = m_backgroundWindows.value(screenId, nullptr);
    if (!window) {
        window = m_backgroundWindows.value(0);
    }
    return window;
}

void ScreenManager::removeBackgroundWindow(QWindow* window)
{
    for (auto it = m_backgroundWindows.begin(); it != m_backgroundWindows.end(); ++it) {
        if (it.value() == window) {
            int screenId = it.key();
            m_backgroundWindows.remove(screenId);
            Q_EMIT windowRemoved(window, screenId);
            break;
        }
    }
}
