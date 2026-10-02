/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#include "desktop-window-manager.h"
#include "peony-desktop-application.h"
#include "advanced-desktop-item-model.h"
#include "desktop-menu-plugin-manager.h"
#include "desktop-widgets-manager.h"
#include <KWindowSystem>

#include "common.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-meta-info.h"
#include "global-settings.h"
#include "file-action-controler.h"
#include "global-settings-adapter.h"
#include "background-manager.h"
#include "screen-manager.h"
#include "theme-style.h"
#include "desktop-partition-dbus-service.h"

#include "windowmanager/windowmanager.h"
#include "desktop-item-proxy-model.h"
#include "platformwindow.h"
#include "desktop-widgets-menu.h"
#include "textureviewer.h"
#include "texturegrabber.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QJSEngine>
#include <QQuickItem>
#include <QScreen>
#include <QOpenGLContext>
#include <QDrag>
#include <QMimeData>
#include <QDataStream>
#include <QCursor>
#include <QApplication>
#include <QTimer>
#include <QGSettings>

#ifdef KY_SDK_SYSINFO
#include <kysdk/kysdk-system/libkysysinfo.h>
#endif

static int desktop_window_id = 0;
static DesktopWindowManager *global_instance = nullptr;

static const QByteArray WORKSTATION_SCHEMAS = QByteArrayLiteral("org.kylin.workstation.settings");
static const QString ENABLE_KEY = QStringLiteral("enable");

DesktopWindowManager *DesktopWindowManager::getInstance()
{
    if (!global_instance) {
        global_instance = new DesktopWindowManager;
    }
    return global_instance;
}

DesktopWindowManager::DesktopWindowManager(QObject *parent) : QObject(parent)
{
    m_isEDUPlat = isEDUPlatform();
    registerCppTypeToQml();
    initPeonyDbusService();
    m_model = PeonyDesktopApplication::getModel();
    connect(m_model, &AdvancedDesktopItemModel::emitFinish, this, &DesktopWindowManager::initModelFinish);
    connect(m_model, &AdvancedDesktopItemModel::sig_fileCreated, this, &DesktopWindowManager::fileCreated);
    connect(m_model, &AdvancedDesktopItemModel::setInfoPos, this, &DesktopWindowManager::setInfoPos);

    m_desktopAutoLayout = GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == DESKTOP_USE_AUTO_LAYOUT) {
            m_desktopAutoLayout = GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
        }
    });

    m_layoutTimeLine = new QTimeLine(500, this);
    m_modeTimeLine = new QTimeLine(500, this);
    connect(m_modeTimeLine, &QTimeLine::finished, this, &DesktopWindowManager::switchDisplayMode);
    connect(SCREEN_MANAGER, &ScreenManager::displayModeChanged,  this, [=](){
        m_layoutTimeLine->stop();
        if (m_modeTimeLine->state() != QTimeLine::Running) {
            m_modeTimeLine->start();
        } else {
            m_modeTimeLine->setCurrentTime(0);
        }
    });

    connect(SCREEN_MANAGER, &ScreenManager::windowsRelocateRequired, this, [=](){
        if (m_modeTimeLine->state() == QTimeLine::Running) {
            return;
        }
        if (m_layoutTimeLine->state() != QTimeLine::Running) {
            m_layoutTimeLine->start();
        } else {
            m_layoutTimeLine->setCurrentTime(0);
        }
    });

    connect(SCREEN_MANAGER, &ScreenManager::resetWindowIds, this, &DesktopWindowManager::resetWindowIds);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qDebug() << "信号创建失败:" << connection.lastError().message();
        return;
    }
    bool isDbusAvailable = connection.connect("org.ukui.ScreenSaver", "/", "org.ukui.ScreenSaver",
                       "unlock", this,SLOT(startInitAnima()));
    m_dbusInterface = new QDBusInterface("org.ukui.ScreenSaver",
                                        "/",
                                        "org.ukui.ScreenSaver",
                                        QDBusConnection::sessionBus());

    bool isQuickStart = qEnvironmentVariableIsSet("UKUI_QUICK_START") && qEnvironmentVariableIntValue("UKUI_QUICK_START") == 1;
    isQuickStartAvailable = isQuickStart && isDbusAvailable;
    initWorkStation();
}


DesktopWindowManager::~DesktopWindowManager()
{
    if (m_peonyDbusService) {
        delete m_peonyDbusService;
        m_peonyDbusService = nullptr;
    }
    if (m_partitionDbusService) {
        delete m_partitionDbusService;
        m_partitionDbusService = nullptr;
    }
}

void DesktopWindowManager::registerCppTypeToQml()
{
    qmlRegisterType<Peony::AdvancedDesktopItemModel>("org.ukui.desktop", 1, 0, "AdvancedDesktopItemModel");
    qmlRegisterType<Peony::DesktopItemProxyModel>("org.ukui.desktop", 1, 0, "DesktopItemProxyModel");
    qmlRegisterType<FileActionController>("org.ukui.desktop", 1, 0, "FileActionController");
    qmlRegisterType<GlobalSettingsAdapter>("org.ukui.desktop", 1, 0, "GlobalSettingsAdapter");
    qmlRegisterType<ThemeStyle>("org.ukui.peony.desktop", 1, 0, "ThemeStyle");
}

void DesktopWindowManager::addQmlDesktop(QScreen *screen)
{
    static bool pluginLoad = false;
    // 防止对同一屏幕创建多个桌面窗口（每多一个窗口就多一个 PAGRenderThread 持续消耗 CPU）
    for (auto win : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *pw = qobject_cast<PlatformWindow*>(win);
        if (pw && pw->bindScreen() == screen) {
            qWarning() << "[DesktopWindowManager] addQmlDesktop: window already exists for screen"
                       << screen->name() << "- skipping duplicate";
            return;
        }
    }
    int screenId = SCREEN_MANAGER->getScreenId(screen);
    PlatformWindow *window = new PlatformWindow(screenId, screen);
    if (!pluginLoad) {
        connect(window, &PlatformWindow::viewUpdated, DesktopMenuPluginManager::getInstance(), &DesktopMenuPluginManager::loadAsync);
        pluginLoad = true;
    }
    connect(this, &DesktopWindowManager::setSortType, window, &PlatformWindow::setSortType);
    connect(this, &DesktopWindowManager::setSortOrder, window, &PlatformWindow::setSortOrder);
    connect(this, &DesktopWindowManager::modeChanged, window, &PlatformWindow::updateView);
    connect(this, &DesktopWindowManager::emitFinish, window, &PlatformWindow::loadData);
    connect(m_layoutTimeLine, &QTimeLine::finished, window, &PlatformWindow::updateWindowGeometry);

    window->show();
    initializeWindowView(window);
    setupWindowConnections(window);

    if (m_initMetaInfo)
        window->loadData();
}

void DesktopWindowManager::singleScreenMode()
{
    qDebug() <<"change singleScreenMode: backgroundwindows count:" <<SCREEN_MANAGER->getBackgroundWindows().count()<<" screen count:"<<SCREEN_MANAGER->allScreens().count();

    bool isMainScreenOnlyData = true;
    for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
        if (!SCREEN_MANAGER->isPrimaryScreen(tmpWindow->bindScreen())) {
            tmpWindow->lower();
            qDebug()<<"lower screen window :"<<tmpWindow->dataCount()<<tmpWindow->id();
            if (0 != tmpWindow->dataCount()) {
                isMainScreenOnlyData = false;
            }
//            break;
        } else {
            qDebug()<<"raise screen window :"<<tmpWindow->dataCount()<<tmpWindow->id();
            SCREEN_MANAGER->getWindowById(0)->raise();
        }
    }
    qDebug() << "Has primary screen data " << isMainScreenOnlyData << "total item:" << m_model->rowCount();
    if (SCREEN_MANAGER->displayMode() == ScreenManager::Extend  && isMainScreenOnlyData) {
        m_model->clearExtendItemPos();
        return;
    }

    SCREEN_MANAGER->getWindowById(0)->requestActivate();

    m_model->getAllRestoreInfo();
    m_model->saveExtendItemInfo();
    Q_EMIT modeChanged();

    DesktopWidgetsManager::getInstance()->mergeWidgetsToPrimaryScreen();
    Q_EMIT singleScreeModeRequest();
}

void DesktopWindowManager::multiscreenMode()
{
    qDebug() <<"change multiscreen: backgroundwindows count:" <<SCREEN_MANAGER->getBackgroundWindows().count()<<" screen count:"<<SCREEN_MANAGER->allScreens().count();
    if (SCREEN_MANAGER->getBackgroundWindows().count() != SCREEN_MANAGER->allScreens().count()) {
        return;
    }

    QSet<int> idList;
    QList<QScreen*> screens = SCREEN_MANAGER->allScreens();
    for (QScreen *screen : screens) {
        int id = SCREEN_MANAGER->getScreenId(screen);
        idList.insert(id);
    }
    m_model->getAllRestoreInfo();
    m_model->resetExtendItemInfo(idList);
    Q_EMIT modeChanged();
    // if(oldMode == ScreenManager::Mirror ){
        Q_EMIT DesktopOrganizationManager::instance()->addinitWidgtRequest();
    //}
    DesktopWidgetsManager::getInstance()->restoreWidgetsFromPrimaryScreen();
}

void DesktopWindowManager::relocateIconView()
{
    qInfo() << "start relocate icon view";
    //task#74174 更新多屏显示,根据id过滤元素
    int primaryId = SCREEN_MANAGER->getPrimaryScreenId();
    PlatformWindow *primaryWindow = nullptr;

    for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
        if (SCREEN_MANAGER->isPrimaryScreen(tmpWindow->bindScreen())) {
            primaryWindow = qobject_cast<PlatformWindow*>(window);
            break;
        }
    }
    qDebug() << "primary screen id:" << primaryId;

    if (primaryWindow) {
        QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
        for (int i = 0 ; i < list.count(); i++) {
            PlatformWindow *window = qobject_cast<PlatformWindow*>(list[i]);
            qDebug() <<"window param:" <<window->id()<<window->bindScreen()<<window->bindScreen()->geometry()<<window->geometry()<<window->screen();
            if (window->id() == 0 && window != primaryWindow) {
                m_model->getAllRestoreInfo();
                // 交换ID
                qDebug() << "切换主副屏:" << primaryId <<primaryWindow->id();
                window->setId(primaryWindow->id());
                primaryWindow->setId(0);
                DesktopWidgetsManager::getInstance()->swapWidgets(window, primaryWindow);
                window->updateView();
                primaryWindow->updateView();
                break;
            }
        }
        QTimer::singleShot(500, primaryWindow, [=]() {
            QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
            for (int i = 0 ; i < list.count(); i++) {
                PlatformWindow *window = qobject_cast<PlatformWindow*>(list[i]);
                window->setScreen(window->bindScreen());
                if (window != primaryWindow) {
                    window->lower();
                }
            }
             qDebug() <<"primaryWindow param:" <<primaryWindow->id()<<primaryWindow->bindScreen()<<primaryWindow->bindScreen()->geometry()<<primaryWindow->geometry()<<primaryWindow;
             primaryWindow->setGeometry(primaryWindow->bindScreen()->geometry());
             primaryWindow->requestActivate();
             primaryWindow->raise();
             primaryWindow->update();
        });
    }
}

void DesktopWindowManager::raiseWid()
{
    QTimer::singleShot(2000, this, [=]() {
        //kdk::WindowManager::activateWindow(SCREEN_MANAGER->getWindowById(0), nullptr);
        return;
    });
}

void DesktopWindowManager::layoutViewItems()
{
//    for (auto bgWindow : m_bgWindows) {
//        auto view = bgWindow->getIconView();
//        view->layoutItems();
//    }
}

void DesktopWindowManager::initModelFinish()
{
    if (m_initMetaInfo)
        return;

    switchDisplayMode();
    relocateIconView();
    m_initMetaInfo = true;
    Q_EMIT emitFinish();
}

void DesktopWindowManager::markCreateFilePos(const QPoint &pos)
{
    //如果当前创建文件的屏幕满了，则找到一个空的屏幕，按照顺序添加
    PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowAt(pos));
    int screenId = SCREEN_MANAGER->getScreenIdAt(pos);
    QPoint localPos = SCREEN_MANAGER->mapFromGlobal(screenId, pos);
    if (QGuiApplication::layoutDirection() == Qt::RightToLeft) {
        int newLocalX = SCREEN_MANAGER->getScreenSize(screenId).isValid()? SCREEN_MANAGER->getScreenSize(screenId).width() - localPos.x() : localPos.x();
        localPos.setX(newLocalX);
    }
    //鼠标坐标点转化成网格标点；
    QPoint tmpPos = window->getGridPosFromMousePos(localPos);
    if (window->isFull()) {
        QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
        for (int i = 0 ; i < list.count(); i++) {
            PlatformWindow *window = qobject_cast<PlatformWindow*>(list[i]);
            if (!window->isFull()) {
                m_markMousePressPos = QPoint(0, 0);
                m_markMousePressId = window->id();
                return;
            }
        }
        m_markMousePressPos = QPoint(0, 0);
        m_markMousePressId = window->id();
    } else {
        m_markMousePressPos = window->checkGridPos(tmpPos);
        m_markMousePressId = window->id();
    }
}

void DesktopWindowManager::createIdAndPos(int *id, QPoint *pos)
{
    if (m_markMousePressPos == QPoint(-1, -1)) {
        PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowAt(QCursor::pos()));
        bool isFull = window? window->isFull() : true;
        if (!isFull) {
            *id = window->id();
            *pos = window->findNextEmptyGridPos();
            return;
        }

        QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
        for (int i = 0 ; i < list.count(); i++) {
            PlatformWindow *emptyPosWindow = qobject_cast<PlatformWindow*>(list[i]);
            isFull = emptyPosWindow? emptyPosWindow->isFull() : true;
            if (!isFull) {
                *id = emptyPosWindow->id();
                *pos = emptyPosWindow->findNextEmptyGridPos();
                return;
            }
        }
        *id = window->id();
        *pos = QPoint(-1, -1);
        return;
    }
    PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowById(m_markMousePressId));
    *id = m_markMousePressId;
    *pos = m_desktopAutoLayout? window->findNextEmptyGridPos(): m_markMousePressPos;//自动排列的情况下清空位置的记录
    m_markMousePressPos = QPoint(-1, -1);
    m_markMousePressId = 0;
    return;
}

void DesktopWindowManager::setInfoPos(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    QPoint currentPos;
    int id;
    createIdAndPos(&id, &currentPos);
    info->setProperty("createId", id);
    info->setProperty("createPos", currentPos);
    qDebug() << "Make the coordinate points of the newly creted file : "<<id<<currentPos ;
}

void DesktopWindowManager::fileCreated(int id, const QString &uri)
{
    auto index = m_model->indexFromUri(uri);
    QPoint metaPos = m_model->data(index,PositionRole).toPoint();
    bool isRenaming =  qApp->property("rename").toBool() && metaPos.x() >= 0 && metaPos.x() >= 0;
    if (isRenaming) {
        qApp->setProperty("rename", false);
    }

    bool isHidden = false;
    auto info = FileInfo::fromUri(uri);
    if (!GlobalSettings::getInstance()->getValue(SHOW_HIDDEN_PREFERENCE).toBool()) {
        auto metainfo = FileMetaInfo::fromUri(uri);
        if (metainfo->hasHiddenInfo()) {
            isHidden = metainfo->isHidden();
        } else {
            isHidden = info->isHiddenFile();
        }
    }

    if (isHidden) return;

    if (isRenaming) {
        int screenId = m_model->data(index, ScreenIdRole).toInt();
        Q_EMIT m_model->idChanged(screenId, -1,  QStringList(uri));
    } else {
        int oldId = m_model->data(index, ScreenIdRole).toInt();
        id = SCREEN_MANAGER->displayMode() == ScreenManager::Mirror? 0 : id;
        m_model->setData(index, id, ScreenIdRole);
        Q_EMIT m_model->idChanged(id, oldId, QStringList(uri));
    }
}

void DesktopWindowManager::clearAllRestoreInfo()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto index = m_model->index(i, 0);
        m_model->setData(index, QStringList(""), ExceptedPositionRole);
    }
}

void DesktopWindowManager::initializeWithScreens()
{
    for (auto screen : SCREEN_MANAGER->allScreens()) {
        addQmlDesktop(screen);
    }

    connect(SCREEN_MANAGER, &ScreenManager::screenAdded, this, [this](QScreen* screen, int id) {
        this->addQmlDesktop(screen);
    });

    connect(SCREEN_MANAGER, &ScreenManager::primaryScreenChanged, this, &DesktopWindowManager::relocateIconView);

/*    connect(SCREEN_MANAGER, &ScreenManager::singleScreenModeActivated, this, &DesktopWindowManager::singleScreenMode);
    connect(SCREEN_MANAGER, &ScreenManager::multiScreenModeActivated, this, &DesktopWindowManager::multiscreenMode)*/;
}

bool DesktopWindowManager::getQuickStartAvailable()
{
    return isQuickStartAvailable;
}

void DesktopWindowManager::initializeWindowView(QWindow* window)
{
    if (!window) return;

    if (m_initMetaInfo) {
        switchDisplayMode();
        relocateIconView();
    } else if (qobject_cast<PlatformWindow*>(window)->bindScreen() == SCREEN_MANAGER->primaryScreen()) {
        m_model->refresh();
    }
}

void DesktopWindowManager::setupWindowConnections(QWindow* window)
{
    if (!window) return;

    QScreen* screen = qobject_cast<PlatformWindow*>(window)->bindScreen();

    connect(SCREEN_MANAGER, &ScreenManager::screenRemoved, window, [=](QScreen* removedScreen, int id) {
        qDebug() << "removedScreen "<<id<<"primay screen geometry:"<<SCREEN_MANAGER->primaryScreen()->geometry();
        if (screen != removedScreen) return;
        int currentMode = SCREEN_MANAGER->displayMode();
        if (currentMode == ScreenManager::Extend) {
            m_model->saveExtendItemInfo(id);
            PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowById(0));
            window->updateView();
        } else if (currentMode == ScreenManager::Mirror) {
            SCREEN_MANAGER->getWindowById(0)->requestActivate();
        }

        //SCREEN_MANAGER会发送显示模式切换信号
        PlatformWindow *p =qobject_cast<PlatformWindow*>(window);
        p->destoryScreen();

        window->disconnect();
        SCREEN_MANAGER->removeBackgroundWindow(window);
        delete window;
    });
}

void DesktopWindowManager::initPeonyDbusService()
{
    m_peonyDbusService = new PeonyDbusService;
    m_peonyDbusService->DbusServerRegister();

    m_partitionDbusService = new PeonyDesktop::DesktopPartitionDbusService(this);
    m_partitionDbusService->registerService();
}

void DesktopWindowManager::switchDisplayMode()
{
    int newMode = SCREEN_MANAGER->displayMode();
    qDebug()<<"switch display mode" <<newMode;

    if (newMode == ScreenManager::Extend) {
        multiscreenMode();
    } else {
        singleScreenMode();
    }
}

void DesktopWindowManager::startInitAnima()
{
    Q_EMIT startInitAnimaRequest();
    QTimer::singleShot(100, this, [this]() {
        Q_EMIT initAnima();
        QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
        for (int i = 0 ; i < list.count(); i++) {
            PlatformWindow *window = qobject_cast<PlatformWindow*>(list[i]);
            window->update();
        }
    });
}

bool DesktopWindowManager::getLockState()
{
    QDBusReply<bool> reply = m_dbusInterface->call("GetLockState");
    return reply.value();
}

void DesktopWindowManager::resetWindowIds(QScreen* oldScreen, int oldId, QScreen* newScreen, int newId)
{
    QList<QWindow*> list = SCREEN_MANAGER->getBackgroundWindows();
    qDebug()<<"the initialization process has not been completed, update id:"<<oldScreen<<oldId<<newScreen<<newId<<list;
    for (int i = 0 ; i < list.count(); i++) {
        PlatformWindow *window = qobject_cast<PlatformWindow*>(list[i]);
        qDebug()<<"Traverse the window info:"<<window->id()<<window->bindScreen() ;
        if (window->bindScreen() == oldScreen && window->id() != oldId) {
            // 交换ID
            window->setId(oldId);
            window->updateView();
        } else if (window->bindScreen() == newScreen && window->id() != newId) {
            window->setId(newId);
            window->updateView();
        }
    }
}

bool DesktopWindowManager::initWorkStation()
{
    if (QGSettings::isSchemaInstalled (WORKSTATION_SCHEMAS)) {
        //workstation monitor
        m_workStationSetting = new QGSettings (WORKSTATION_SCHEMAS, QByteArray(), this);
        if (m_workStationSetting->keys().contains(ENABLE_KEY)) {
            m_workStationEnable = m_workStationSetting->get(ENABLE_KEY).toBool();
        }
        connect (m_workStationSetting, &QGSettings::changed, this, &DesktopWindowManager::changeStatusOfWorkStation);
    }
    return m_workStationEnable;
}

bool DesktopWindowManager::changeStatusOfWorkStation(const QString &key)
{
    if (key == ENABLE_KEY) {
        bool enabled = m_workStationSetting->get(ENABLE_KEY).toBool();
        m_workStationEnable = enabled;
        Q_EMIT workstationEnabledChanged(enabled);
    }
    return m_workStationEnable;
}

bool DesktopWindowManager::isWorkstationEnabled()
{
    return m_workStationEnable;
}

bool DesktopWindowManager::isEDUPlatform()
{
    bool isEDUPlat = false;
#ifdef KY_SDK_SYSINFO
    g_autofree char *platType = kdk_system_get_appScene();
    if (platType != nullptr) {
        qDebug() << "platType is " << platType;
        if (strcmp(platType, "EDU") == 0) {
            isEDUPlat = true;
        }
        qDebug() << "[DesktopWindowManager::isEDUPlatform] type " << platType;
    }
#endif
    qDebug() << "[DesktopWindowManager::isEDUPlatform] is platform edu " << isEDUPlat;
    return isEDUPlat;
}


static DesktopOrganizationManager *global_instance_org_manager = nullptr;

DesktopOrganizationManager *DesktopOrganizationManager::instance()
{
    if (!global_instance_org_manager) {
        global_instance_org_manager = new DesktopOrganizationManager;
    }
    return global_instance_org_manager;
}

bool DesktopOrganizationManager::isOriganizationToolConfigExist()
{
    return GlobalSettings::getInstance()->getValue("isOriganizationToolConfigExist").toBool();
}

void DesktopOrganizationManager::setConfigStatus(bool isExits)
{
    // 是否需要清空整理桌面配置？
    GlobalSettings::getInstance()->setValue("isOriganizationToolConfigExist", isExits);
}

void DesktopOrganizationManager::RestoreDesktopInfo()
{
    auto model = PeonyDesktopApplication::getModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto modelIndex = model->index(i, 0);
        auto uri = modelIndex.data(Qt::UserRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            int preinstanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID);
            //metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
            //metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, preinstanceid);
            if(!metaInfo->getMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION)){
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, -1);
            }
            ///metaInfo->setMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM,-1);
        }
    }
    QTimer::singleShot(100, this, [=](){
        for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
            PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
            tmpWindow->ajustIconsOnScreenSize();
            tmpWindow->checkItemsOver();
        }
    });
}

void DesktopOrganizationManager::CreatePartition(QStringList uris)
{
    QPoint cursorPos = QCursor::pos();
    auto window  = SCREEN_MANAGER->getWindowAt(cursorPos);
    int id = SCREEN_MANAGER->getScreenIdAt(cursorPos);
    int instanceid = DesktopWidgetsManager::getInstance()->getNextInstanceId();
    PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
    DesktopOrganizationManager::instance()->addDesktopOrganizationWidget(tmpWindow, DesktopItemProxyModel::OriganizationType::Others, tr("New Partition"), uris); //这里的uri需要根据类型获取
    for(auto uri:uris){
        auto metaInfo = FileMetaInfo::fromUri(uri);
        metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION,1);
    }
    Q_EMIT renameRequest(instanceid,tr("New Partition"),true);
}

void DesktopOrganizationManager::doSort(int instanceId, int type,int order)
{
    Q_EMIT requestSort(instanceId, type,order);
    // todo: 在proxymodel中实现
}

void DesktopOrganizationManager::disbandPartition(int instanceId)
{
    Q_EMIT requestDisbandPartation(instanceId);
    // todo: 在proxymodel中实现数据清空，另外清空完成后需要调用widgetsmanager::removewidget()移除插件界面和配置
    QPoint cursorPos = QCursor::pos();
    auto window  = SCREEN_MANAGER->getWindowAt(cursorPos);
    PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
    DesktopWidgetsManager::getInstance()->removeWidget(instanceId, tmpWindow);
    Q_EMIT updateDesktopView();
    tmpWindow->reloadData();
}

void DesktopOrganizationManager::initOrganizationWidgets()
{
    // 如果已经存在配置则退出
    if (isOriganizationToolConfigExist()) {
        return;
    }

    auto windows = PlatformWindow::allWindows();

    // 创建整理工具
    for (auto window : windows) {
        initOrganizationWidgetsForWindow(window);
    }

    // 更新配置
    setConfigStatus(true);
}

void DesktopOrganizationManager::addDesktopOrganizationWidget(PlatformWindow *targetWindow, Peony::DesktopItemProxyModel::OriganizationType type, const QString &widgetName, const QStringList &fileUris)
{
    // 初始化配置，并且加载带配置的插件到ui
    auto parentConfig = targetWindow->mainView()->config();
    QVariantMap childData;
    childData.insert("id", "desktop-organization-tool");
    int newWidgetInstanceId = DesktopWidgetsManager::getInstance()->getNextInstanceId();
    if(!fileUris.isEmpty()){
        for(auto uri :fileUris){
            auto metaInfo = FileMetaInfo::fromUri(uri);
            if (metaInfo) {
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, newWidgetInstanceId);
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, newWidgetInstanceId);
            }
        }
    }

    childData.insert("instanceId", newWidgetInstanceId);
    childData.insert("name", widgetName);
    childData.insert("organizationType", int(type));
    // todo: 根据文件数量计算大小

    parentConfig->addChild("widgets", childData);
    auto newConfig = parentConfig->child("widgets", newWidgetInstanceId);
    targetWindow->mainView()->removeDisableInstance(newWidgetInstanceId);
    auto newWidget = UkuiQuick::WidgetLoader::globalLoader().loadWidget("desktop-organization-tool");
    newWidget->setConfig(newConfig);
    newWidget->setInstanceId(newWidgetInstanceId);
    targetWindow->mainView()->addWidget(newWidget);
    requestUpdateOrganizationView(newWidgetInstanceId);
    requestUpdateOrganizationView(-1);
}

void DesktopOrganizationManager::setDesktopOrganizationWidgetEnabled(PlatformWindow *targetWindow, bool enabled)
{
    if (enabled) {
        auto containerConfig = targetWindow->mainView()->config();

        // 从当前disabled的instances找到禁用的整理工具插件
        auto disabledInstances = targetWindow->mainView()->disableInstances();

        // 通过container config读取对应子widget的config，通过id和org type判断是否需要启用
        for (auto instanceId : disabledInstances) {
            auto config = containerConfig->child("widgets", instanceId);
            if(config==nullptr){
                continue;
            }
            if (config->getValue("id").toString() == "desktop-organization-tool") {
                int orgType = config->getValue("organizationType").toInt();
                if (orgType >= 0 && orgType <= 2) {
                    if(config->contains("oldScreenId") && config->getValue("oldScreenId").toInt() != targetWindow->id()){
                        continue;
                    }
                    auto urilist = getUrisByOrganizationType(orgType,targetWindow->id());
                    for (auto uri:urilist) {
                        auto metaInfo = FileMetaInfo::fromUri(uri);
                        if (metaInfo) {
                            auto partitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID_UNORDER);
                            bool isvaild = partitonVariant.isValid();
                            int preinstanceid = partitonVariant.toInt(); //metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER);
                            if(!isvaild || preinstanceid == -1){
                                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID,instanceId);
                            }else /*if( isvaild && preinstanceid == instanceId)*/{
                                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID,preinstanceid);
                            }
                        }
                    }
                    targetWindow->mainView()->removeDisableInstance(instanceId);
                    auto newWidget = UkuiQuick::WidgetLoader::globalLoader().loadWidget("desktop-organization-tool");
                    newWidget->setConfig(config);
                    newWidget->setInstanceId(instanceId);
                    targetWindow->mainView()->addWidget(newWidget);
                    QTimer::singleShot(100, this, [=](){
                        Q_EMIT DesktopOrganizationManager::requestUpdateOrganizationView(instanceId);
                    });
                }
            }
        }

        // 通过loader加载widget并赋予配置，再添加到mainview
    } else {
        // 根据当前container显示的widget id和config中的org type判断需要禁用的item
        auto widgets = targetWindow->mainView()->widgets();
        for (UkuiQuick::Widget *widget : widgets) {
            if (widget->id() == "desktop-organization-tool") {
                int orgType = widget->config()->getValue("organizationType").toInt();
                if (orgType >= 0 && orgType <= 2) {
                    // 自动排列类型，禁用 todo: 需要让桌面的proxyModel更新filter并且显示禁用分区的item
                    targetWindow->mainView()->addDisableInstance(widget->instanceId());
                }
            }
        }
    }
}

QStringList DesktopOrganizationManager::getUrisByOrganizationType(int type,int screenId)
{
    QStringList urilist;
    auto model = PeonyDesktopApplication::getModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto modelIndex = model->index(i, 0);
        auto uri = modelIndex.data(Qt::UserRole).toString();
        auto info = FileInfo::fromUri(uri);
        QString path = info->filePath();
        QString contentType = info->type();
        bool isDesktopFile = path.endsWith(".desktop") || (contentType == "application/x-desktop") || uri=="trash:///" || uri=="computer:///";
        bool isDir = info->isDir() && uri!="trash:///" && uri!="computer:///";
        bool flag = false;
        auto metaInfo = FileMetaInfo::fromUri(uri);
        int id = metaInfo->getMetaInfoInt(SCREEN_ID);
        if(id != screenId){
            continue;
        }
        switch (type) {
        case DesktopItemProxyModel::None:
            //flag = (instanceId == -1);
            break;
        case DesktopItemProxyModel::OrgByApplycation:
            flag = isDesktopFile;
            break;
        case DesktopItemProxyModel::OrgByDir:
            flag = isDir;
            break;
        case DesktopItemProxyModel::OrgByFile:
            flag = (!isDir) && (!isDesktopFile);
            break;
        default:         //others
            break;
        }
        if(flag){
            urilist.append(uri);
        }
    }
    return urilist;
}

void DesktopOrganizationManager::expandStatusChanged(int instanceId,bool expand,int height)
{
    QPoint cursorPos = QCursor::pos();
    auto window  = SCREEN_MANAGER->getWindowAt(cursorPos);
    PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
    auto widget = tmpWindow->mainView()->widget(instanceId);
    widget->config()->setValue("IsExpand",expand);
    widget->config()->setValue("Resizable",expand);
    if(!expand){
        SpacesSettings::globalInstance()->setInitHeight(instanceId,height);
    }
    SpacesSettings::globalInstance()->setIsExpand(instanceId,expand);
    Q_EMIT expandStatusChangedRequest(instanceId,expand);
}

bool DesktopOrganizationManager::isExpand(int instanceId)
{
     return SpacesSettings::globalInstance()->getIsExpand(instanceId);
}

int DesktopOrganizationManager::getInitHeight(int instanceId)
{
    return SpacesSettings::globalInstance()->getInitHeight(instanceId);
}

void DesktopOrganizationManager::setInitHeight(int instanceId, int height)
{
    SpacesSettings::globalInstance()->setInitHeight(instanceId,height);
}

void DesktopOrganizationManager::updateOrganizationView(int instanceId)
{
    Q_EMIT requestUpdateOrganizationView(instanceId);
}

void DesktopOrganizationManager::renameByorgTool()
{
    qApp->setProperty("renamebyorg",true);
}

bool DesktopOrganizationManager::isManulCreatePartition(int instanceid)
{
    for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
        auto widgets = tmpWindow->mainView()->widgets();
        for (auto widget: widgets) {
            if(widget->instanceId() == instanceid){
                int orgtype =  widget->config()->getValue("organizationType").toInt();
                return !(orgtype>=0 && orgtype<=2) ;
            }
        }
    }
    return false;
}

void DesktopOrganizationManager::initOrganizationWidgetsForWindow(PlatformWindow *window)
{
    window->initDesktopOrgWidgets();
}

bool DesktopOrganizationManager::getIsListMode(int instanceid)
{
    return SpacesSettings::globalInstance()->getIsPartitionListMode(instanceid);
}

void DesktopOrganizationManager::renameSpace(int instanceId, QString name)
{
    int primaryId = SCREEN_MANAGER->getPrimaryScreenId();
    PlatformWindow *primaryWindow = nullptr;
    for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
        if (tmpWindow) {
            auto widget = tmpWindow->mainView()->widget(instanceId);
            if(widget!=nullptr){
                widget->config()->setValue("name",name);
            }
        }
    }
    Q_EMIT renameRequest(instanceId,name,false);
}

void DesktopOrganizationManager::cancelEdit()
{
     Q_EMIT cancelEditRequest();
}

void DesktopOrganizationManager::showMenuRequest(const QPoint &globalPos, int instanceId)
{
    WidgetMenu menu(instanceId);

    QTimer checkTimer;
    checkTimer.setInterval(150);
    bool menuDismissed = false;

    QObject::connect(&checkTimer, &QTimer::timeout, [&]() {
        if (menuDismissed) {
            checkTimer.stop();
            return;
        }

        if (!menu.isVisible()) {
            checkTimer.stop();
            return;
        }

        QPoint pos = QCursor::pos();

        // 检查鼠标是否在主菜单或任何子菜单范围内
        bool inMenu = menu.geometry().contains(pos);

        if (!inMenu) {
            // 检查所有子菜单
            for (QAction *action : menu.actions()) {
                QMenu *submenu = action->menu();
                if (submenu && submenu->isVisible() && submenu->geometry().contains(pos)) {
                    inMenu = true;
                    break;
                }
            }
        }

        if (!inMenu) {
            for (QAction *action : menu.actions()) {
                QMenu *submenu = action->menu();
                if (submenu && submenu->isVisible() && !submenu->geometry().contains(pos)) {
                    submenu->close();
                    break;
                }
            }
        }
    });

    checkTimer.start();
    menu.exec(globalPos);
    checkTimer.stop();
}

void DesktopOrganizationManager::addFileFromDesktopToOrgTool(QStringList urilist, int instanceId)
{
    if(urilist.isEmpty()){
        return;
    }
    QPoint cursorPos = QCursor::pos();
    auto window  = SCREEN_MANAGER->getWindowAt(cursorPos);
    PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
    bool ismanulcreate =  isManulCreatePartition(instanceId);
    for (auto uri : urilist) {
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, instanceId);
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER,instanceId);
            if(ismanulcreate){
                metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION,1);
            }
        }
    }
    // auto proxymodel = getmodel(instanceId);
    //QTimer::singleShot(100, this, [=]() {
    // auto model = qobject_cast<Peony::AdvancedDesktopItemModel*>(proxymodel->sourceModel());
    tmpWindow->updateView();
    // proxymodel->updateFilter();
    updateOrganizationView(instanceId);
    auto config = SpacesSettings::globalInstance()->getSpacesSortConfig(instanceId);
    auto type  = config.value("partitionSortType").toInt();
    auto order = config.value("partitionSortAsc").toInt();

    doSort(instanceId,type,order);
    //auto selecturi = urilist.first();
    //Q_EMIT selectAndJumptoUriRequest(instanceId,selecturi);
    //});
}

static DesktopUKUIpluginnManager *global_instance_plugin_manager = nullptr;

DesktopUKUIpluginnManager *DesktopUKUIpluginnManager::instance()
{
    if (!global_instance_plugin_manager) {
        global_instance_plugin_manager = new DesktopUKUIpluginnManager;
    }
    return global_instance_plugin_manager;
}

void DesktopUKUIpluginnManager::setGirdSize(int col,int row,int instanceId){

    int primaryId = SCREEN_MANAGER->getPrimaryScreenId();
    PlatformWindow *primaryWindow = nullptr;

    for (auto window : SCREEN_MANAGER->getBackgroundWindows()) {
        PlatformWindow *tmpWindow = qobject_cast<PlatformWindow*>(window);
        if (SCREEN_MANAGER->isPrimaryScreen(tmpWindow->bindScreen())) {
            primaryWindow = qobject_cast<PlatformWindow*>(window);
            break;
        }
    }
    auto widget = primaryWindow->mainView()->widget(instanceId);
    auto config = widget->config();
    if(col>0 && row >0){
        QVariantList list = {col, row};
        config->setValue("SizeHint",list);
    }
}

bool DesktopUKUIpluginnManager::isNeedInit(int instanceid)
{
    auto windows = PlatformWindow::allWindows();
    for (auto window : windows) {
        auto config = window->mainView()->config();
        if (config->child("widgets", instanceid)) {
            return false;
        }
    }
    return true;
}

int DesktopUKUIpluginnManager::minSize(UkuiQuick::WidgetItem *widgetItem)
{
    int gridSizeUnit = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);
    int minsize = 2 * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
    return minsize;
}

void DesktopWindowManager::setQAppProperty(const QString &propertyName, const QVariant &value)
{
    qApp->setProperty(propertyName.toUtf8().constData(), value);
}

QWindow *DesktopWindowManager::createDragPreviewWithTexture(QQuickItem *textureSource, const QPoint &globalPos, int width, int height, float cornerRadius)
{
    QOpenGLContext *ctx = textureSource->window()->openglContext();
    if (!ctx) {
        qWarning() << "[createDragPreview] OpenGL context is null";
        return nullptr;
    }

    if (m_dragTexturePreviewCreated) {
        if (m_dragTextureViewer) {
            m_dragTextureViewer->setCornerRadius(cornerRadius);
        }
        return m_dragTextureViewer;
    }
    m_dragTexturePreviewCreated = true;

    m_dragTextureViewer = new TextureViewer(ctx);
    m_dragTextureViewer->resize(width, height);
    m_dragTextureViewer->setCornerRadius(cornerRadius);
    // globalPos 是 widget 左上角坐标，直接使用
    m_dragTextureViewer->setPosition(globalPos.x(), globalPos.y());
    m_dragTextureViewer->show();
    m_dragTextureViewer->enableBlurEffect();

    m_dragTextureGrabber = new TextureGrabber(textureSource->window());
    m_dragTextureGrabber->setSourceItem(textureSource);

    connect(m_dragTextureGrabber, &TextureGrabber::textureReady,
            m_dragTextureViewer, [this](quint32 textureId, const QSize &size, int key) {
                if (m_dragTextureViewer) {
                    m_dragTextureViewer->setSourceTexture(textureId, size.width(), size.height());
                }
            }, Qt::QueuedConnection);

    return m_dragTextureViewer;
}

QWindow *DesktopWindowManager::dragPreviewWindow()
{
    return m_dragTextureViewer;
}

bool DesktopWindowManager::isInExtendMode()
{
    return ScreenManager::getInstance()->displayMode() == ScreenManager::Extend;
}

void DesktopWindowManager::updateDragPreviewPosition(const QPoint &globalPos)
{
    if (m_dragTextureViewer) {
        // globalPos 是 widget 左上角坐标，直接使用
        m_dragTextureViewer->setPosition(globalPos.x(), globalPos.y());
    }
}

void DesktopWindowManager::closeDragPreviewWithTexture()
{
    if (m_dragTextureViewer) {
        m_dragTextureViewer->close();
        m_dragTextureViewer->deleteLater();
        m_dragTextureViewer = nullptr;
    }
    if (m_dragTextureGrabber) {
        m_dragTextureGrabber->setEnabled(false);
        m_dragTextureGrabber->disconnectFromSource();
        m_dragTextureGrabber->setSourceItem(nullptr);
        m_dragTextureGrabber->deleteLater();
        m_dragTextureGrabber = nullptr;
    }
    m_dragTexturePreviewCreated = false;
}
