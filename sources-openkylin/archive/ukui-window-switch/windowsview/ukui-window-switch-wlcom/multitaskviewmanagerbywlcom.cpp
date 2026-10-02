/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "multitaskviewmanagerbywlcom.h"

// kf5
#include <KF5/KGlobalAccel/KGlobalAccel>

// kysdk
#include <libkysysinfo.h>
#include <libkydatacollect.h>

// Qt
#include <QAction>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QtDBus>
#include <QTranslator>
#include <QLocale>
#include <QtConcurrent>
#include <QScreen>
#include <QPainterPath>
#include <QGSettings>

//ukui-quick
#include <window-manager.h>

#include "../windowmanagerinterface.h"
#include "../desktopbackground.h"
#include "../appwindowfiltermodel.h"
#include "../ukui-flow.h"
#include "../globalsettings.h"
#include "../windowsmanager.h"
#include "../desktopmanager.h"

bool MultitaskViewManagerByWlcom::m_isOpenGLBackend = true;
bool MultitaskViewManagerByWlcom::isOpen = false;

MultitaskViewManagerByWlcom::MultitaskViewManagerByWlcom()
{
    initializeProjectType();
    registerQmlTypes();
    registerShortcut();
    initDBusService();
    installTranslator();
    UkuiWindowSwitch::GlobalSettings::globalInstance(this);
    WindowManagerInterface::getInstance();

    //init shared qmlEngine
    m_qmlEngine = std::make_unique<QQmlEngine>();

    const QByteArray id("org.mate.background");
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * backgSetting = new QGSettings(id, QByteArray(), this);
        connect(backgSetting, &QGSettings::changed,[=](QString key) {
            if ("pictureFilename" == key) {
                updateDesktopBackground();
            }
        });
    }
    updateDesktopBackground();
    initModel();
    initView();
    connectSignals();
}

MultitaskViewManagerByWlcom::~MultitaskViewManagerByWlcom()
{
    close();
    destroyView();
    if (m_translator) {
        m_translator->deleteLater();
        m_translator = nullptr;
    }
    m_qmlEngine.release()->deleteLater();
}

bool MultitaskViewManagerByWlcom::isCursorNavigationKey(const int &key) const
{
    if (key == Qt::Key_Tab || key == Qt::Key_Left ||
        key == Qt::Key_Right || key == Qt::Key_Up ||
        key == Qt::Key_Down || key == Qt::Key_Home ||
        key == Qt::Key_End || key == Qt::Key_PageUp ||
        key == Qt::Key_Enter || key == Qt::Key_Return ||
        key == Qt::Key_PageDown) {
        return true;
    }
    return false;
}

void MultitaskViewManagerByWlcom::activeAreaChangeDataCollect(const QString &areaName)
{
    QByteArray name = areaName.toLatin1();
    QtConcurrent::run([name]() {
        const char* text = name.data();
        KCustomProperty property[1];
        property[0].key = (char*)"activeAreaName";
        property[0].value = (char*)text;
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, 1);
        kdk_dia_upload_default(node, (char*)"keyboard_navigation_active_area_changed_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

bool MultitaskViewManagerByWlcom::isAltKeyPressed()
{
    return QGuiApplication::queryKeyboardModifiers() & Qt::KeyboardModifier::AltModifier;
}

void MultitaskViewManagerByWlcom::keyNavigationDataCollect(const int &keyValue)
{
    if (!isCursorNavigationKey(keyValue)) {
        return;
    }

    QByteArray name = QKeySequence(keyValue).toString().toLatin1();
    QtConcurrent::run([name]() {
        const char* text = name.data();
        KCustomProperty property[1];
        property[0].key = (char*)"keyName";
        property[0].value = (char*)text;
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
        kdk_dia_append_custom_property(node, property, 1);
        kdk_dia_upload_default(node, (char*)"keyboard_navigation_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

void MultitaskViewManagerByWlcom::activateNextTaskView(const int currentIndex)
{
    if (m_taskViews.isEmpty() || currentIndex >= m_taskViews.size()) {
        return;
    }

    // Tab走焦首尾循环
    // 非最后一个屏幕，则索引值+1；最后一个屏幕，则索引值置为0
    int nextIndex = 0;
    if (m_taskViews.size() - 1 > currentIndex) {
        nextIndex = currentIndex + 1;
    }

    for (const auto &view : m_taskViews) {
        if (view->rootObject()->property("screenIndex") == nextIndex) {
            view->rootObject()->setProperty("shouldHaveTabFocus", true);
            view->requestActivate();
            break;
        }
    }
}

void MultitaskViewManagerByWlcom::activatePreviousTaskView(const int currentIndex)
{
    if (m_taskViews.isEmpty() || currentIndex >= m_taskViews.size()) {
        return;
    }

    int preIndex = 0;
    if (currentIndex > 0) {
        preIndex = currentIndex - 1;
    } else {
        preIndex = m_taskViews.size() - 1;
    }

    for (const auto &view : m_taskViews) {
        if (view->rootObject()->property("screenIndex") == preIndex) {
            view->rootObject()->setProperty("shouldHaveTabFocus", true);
            view->requestActivate();
            break;
        }
    }
}

void MultitaskViewManagerByWlcom:: open()
{
    if (isOpen) {
        close();
        return;
    }
    qDebug() << "开启多任务视图" << __FUNCTION__ << __LINE__ << isOpen;

    // 可旋转的机器，多任务视图打开的时候，禁用旋转
    if(m_isLoadTabletModeQML && getRotationModeStatus()) {
        setRotationModeStatus(!m_isAutoRotateBeforeShowMultiTask);
    }
    m_isShowTabBorder = false;

    showTaskViews();
    isOpen = true;

    QtConcurrent::run([]() {
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_upload_default(node, (char*)"ukui_window_switch_start_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

void MultitaskViewManagerByWlcom::openTab()
{
    if (isOpen) {
        int currentIndex = m_tabView->rootObject()->property("currentIndex").toInt();
        int allcount = m_appWindowListModel->rowCount();
        if (currentIndex + 1 == allcount) {
            currentIndex = 0;
        } else {
            currentIndex++;
        }
        m_tabView->rootObject()->setProperty("currentIndex", currentIndex);
        return;
    }

    qDebug() << "正向遍历窗口" << __FUNCTION__ << __LINE__ << isOpen;
    m_isBackTab = false;

    // 可旋转的机器，多任务视图打开的时候，禁用旋转
    if(m_isLoadTabletModeQML && getRotationModeStatus()) {
        setRotationModeStatus(!m_isAutoRotateBeforeShowMultiTask);
    }
    if (m_appWindowListModel->rowCount() == 0) {
        return;
    }

    showTabView();

    isOpen = true;
}

void MultitaskViewManagerByWlcom::openBackTab()
{
    if (isOpen) {
        int currentIndex = m_tabView->rootObject()->property("currentIndex").toInt();
        if (currentIndex == 0) {
            currentIndex = m_appWindowListModel->rowCount() - 1;
        } else {
            currentIndex--;
        }
        m_tabView->rootObject()->setProperty("currentIndex", currentIndex);
        return;
    }

    qDebug() << "反向遍历窗口" << __FUNCTION__ << __LINE__ << isOpen;
    m_isBackTab = true;

    // 可旋转的机器，多任务视图打开的时候，禁用旋转
    if(m_isLoadTabletModeQML && getRotationModeStatus()) {
        setRotationModeStatus(!m_isAutoRotateBeforeShowMultiTask);
    }
    if (m_appWindowListModel->rowCount() == 0) {
        return;
    }

    showTabView();

    isOpen = true;
}

void MultitaskViewManagerByWlcom::close()
{
    if (!isOpen) {
        return;
    }
    //可旋转的机器，多任务视图打开前 是打开了旋转的 那么关闭多任务视图的同时  打开旋转
    if (m_isLoadTabletModeQML && m_isAutoRotateBeforeShowMultiTask) {
        setRotationModeStatus(m_isAutoRotateBeforeShowMultiTask);
    }

    hideTabView();
    hideTaskViews();
    isOpen = false;
    QtConcurrent::run([]() {
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_upload_default(node, (char*)"ukui_window_switch_exit_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

bool MultitaskViewManagerByWlcom::getRotationModeStatus()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.kylin.statusmanager.interface",
                                                          "/",
                                                          "com.kylin.statusmanager.interface",
                                                          "get_auto_rotation");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);

    if (response.type() == QDBusMessage::ReplyMessage) {
        // 参数校验：防止空参数列表导致的未定义行为
        const auto &args = response.arguments();
        if (args.isEmpty()) {
            qWarning() << "get_auto_rotation returned empty arguments";
            return m_isAutoRotateBeforeShowMultiTask;
        }
        m_isAutoRotateBeforeShowMultiTask = args.first().toBool();
    } else {
        qDebug() << "get Rotation mode failed" << response.type();
    }
    return m_isAutoRotateBeforeShowMultiTask;
}

void MultitaskViewManagerByWlcom::setRotationModeStatus(bool rotationModeStatus)
{
    QDBusInterface iface("com.kylin.statusmanager.interface",
                         "/",
                         "com.kylin.statusmanager.interface");

    if (!iface.isValid()) {
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }
    QDBusReply<void> reply = iface.call("set_auto_rotation",rotationModeStatus,"","");
    if (reply.isValid()) {
        qDebug() << "set Rotation mode success";
        return;
    }
    qDebug() << "set Rotation mode failed" << reply.error();
}

QScreen* MultitaskViewManagerByWlcom::getCurrentScreen()
{
    return UkuiQuick::WindowProxy::currentScreen();
}

QRect MultitaskViewManagerByWlcom::currentScreenRect()
{
    return UkuiQuick::WindowProxy::currentScreen()->geometry();
}

QList<Screen *> MultitaskViewManagerByWlcom::getScreens()
{
    return ScreensManager::instance()->screens();
}

bool MultitaskViewManagerByWlcom::isBackTab()
{
    return m_isBackTab;
}

bool MultitaskViewManagerByWlcom::altTabVisible()
{
    return m_tabView->isVisible();
}

bool MultitaskViewManagerByWlcom::winTabVisible()
{
    if (m_taskViews.size() <= 0) {
        return false;
    } else {
        return m_taskViews.first()->isVisible();
    }
}

int MultitaskViewManagerByWlcom::desktopTipPos()
{
    return UkuiWindowSwitch::GlobalSettings::globalInstance()->panelPosition();
}

void MultitaskViewManagerByWlcom::showDesktopBallonTip()
{
    emit updateCurrentScreen();
    if (winTabVisible()) {
        m_showDesktopTipFlag = true;
    } else {
        emit showDesktopTipSignal();
    }
}

void MultitaskViewManagerByWlcom::onScreenAdd(Screen *screen)
{
    close();
    if (!m_taskViews.contains(screen)) {
        auto taskView = createTaskView(screen);
        m_taskViews.insert(screen, taskView);
    }
}

void MultitaskViewManagerByWlcom::onScreenRemove(Screen *screen)
{
    close();
    if (m_taskViews.contains(screen)) {
        QQuickView* view = m_taskViews.take(screen);
        if (view) {
            view->deleteLater();
        }
    }
}

void MultitaskViewManagerByWlcom::updateDesktopBackground()
{
    QString filePath = DesktopBackground::getBackgroundFile();
    if (filePath.isEmpty()) {
        return;
    }

    auto loadImageFinishedCallback = [this](const QImage &image) {
        DesktopBackground::setNonBlurBackground(image);
        DesktopBackground::setBlurBackground(image, 0.7);
        emit this->backgroundChanged();
    };

    BackgroundImageHandler::loadBackgroundImageAsync(filePath, DesktopBackground::getSystemScalingFactor(),
                                                     DesktopBackground::getBackgroundFillStyle(),
                                                     this, loadImageFinishedCallback);
}

void MultitaskViewManagerByWlcom::setShowingDesktop(bool showing)
{
    Q_UNUSED(showing)
    UkuiQuick::WindowManager::setShowingDesktop(true);
}

void MultitaskViewManagerByWlcom::initModel()
{
    qDebug() << "创建model数据" << __FUNCTION__;
    m_desktopListModel.reset(new DesktopListModel());
    m_appWindowListModel.reset(new AppWindowListModel());
}

void MultitaskViewManagerByWlcom::initializeProjectType()
{
    if ( getProjectCodeName() == QStringLiteral("V10SP1-edu")
      || getProjectSubName() == QStringLiteral("mavis")
      || getProductFeatures() == 2 ) {
        m_projectType = MultitaskViewManagerByWlcom::TABLET;
    } else if (getProjectCodeName() == QStringLiteral("V10SP1") && getProductFeatures() == '\u0000') {
        // '\u0000' 空格的ASCII码
        m_projectType = MultitaskViewManagerByWlcom::MASTER;
        m_isViewVerticalLayout = false;
    } else {
        m_projectType = MultitaskViewManagerByWlcom::TWOINONE;
        m_isViewVerticalLayout = true;
    }
}

void MultitaskViewManagerByWlcom::setQmlUrl()
{
    if (m_projectType == MultitaskViewManagerByWlcom::TABLET
        || (m_projectType == MultitaskViewManagerByWlcom::TWOINONE && WindowManagerInterface::getInstance()->isTabletMode())) {
        m_qmlUrl = QUrl("qrc:/qml/TabletViewMain.qml");
        m_isLoadTabletModeQML = true;
    } else if (m_projectType == MultitaskViewManagerByWlcom::MASTER
        || (m_projectType == MultitaskViewManagerByWlcom::TWOINONE && !WindowManagerInterface::getInstance()->isTabletMode())) {
        m_qmlUrl = QUrl("qrc:/qml/MultitaskView.qml");
        m_isLoadTabletModeQML = false;
    }
}

void MultitaskViewManagerByWlcom::registerQmlTypes()
{
    qmlRegisterType<DesktopBackground>("org.kylin.background", 3, 0, "DesktopBackground");
    qmlRegisterType<AppWindowFilterModel>("AppWindowFilterModel", 1, 0, "AppWindowFilterModel");
    qmlRegisterType<DesktopListModel>("DesktopListModel", 1, 0, "DesktopListModel");
    qmlRegisterUncreatableType<UkuiPositionerAttached>("org.ukui.positioner", 1, 0, "UkuiPositioner", "UkuiPositioner is an attached property");
    qmlRegisterUncreatableType<UkuiBasePositioner>("org.ukui.positioner", 1, 0, "UkuiBasePositioner", "UkuiBasePositioner is an abstract type that is only available as an attached property.");
    qmlRegisterType<UkuiFlow>("org.ukui.positioner", 1, 0, "UkuiFlow");
}

void MultitaskViewManagerByWlcom::registerShortcut()
{
    auto* action = new QAction(this);
    action->setObjectName("ShowMultitaskView");
    action->setText("Show Multitask View");

    QKeySequence keySeq(Qt::META + Qt::Key_Tab);
    KGlobalAccel::self()->setDefaultShortcut(action, QList<QKeySequence>() << keySeq);
    KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>() << keySeq);
    connect(action, &QAction::triggered, this, &MultitaskViewManagerByWlcom::open);

    auto* tabaction = new QAction(this);
    tabaction->setObjectName("ShowTabbox");
    tabaction->setText("Show Tabbox");

    QKeySequence tabKeySeq(Qt::ALT + Qt::Key_Tab);
    KGlobalAccel::self()->setDefaultShortcut(tabaction, QList<QKeySequence>() << tabKeySeq);
    KGlobalAccel::self()->setShortcut(tabaction, QList<QKeySequence>() << tabKeySeq);
    connect(tabaction, &QAction::triggered, this, &MultitaskViewManagerByWlcom::openTab);

    auto* backtabaction = new QAction(this);
    backtabaction->setObjectName("ShowBackTabbox");
    backtabaction->setText("Show Back Tabbox");

    QKeySequence backtabKeySeq(Qt::ALT + Qt::SHIFT + Qt::Key_Backtab);
    KGlobalAccel::self()->setDefaultShortcut(backtabaction, QList<QKeySequence>() << backtabKeySeq);
    KGlobalAccel::self()->setShortcut(backtabaction, QList<QKeySequence>() << backtabKeySeq);
    connect(backtabaction, &QAction::triggered, this, &MultitaskViewManagerByWlcom::openBackTab);
}

void MultitaskViewManagerByWlcom::initDBusService()
{
   m_dbusService.reset(new DBusService(this));
}

void MultitaskViewManagerByWlcom::installTranslator()
{
    m_translator = new QTranslator(qApp);
    m_translator->load(QLocale::system(), QLatin1String("ukui-window-switch"),
                       QLatin1String("_"), TRANSLATION_DIR);
    QGuiApplication::installTranslator(m_translator);
}

void MultitaskViewManagerByWlcom::initView()
{
    qDebug() << " 初始化多任务视图 " << __FUNCTION__;

    //init alt-tab view
    m_tabView.reset(new QQuickView());
    m_tabView->setFlags(Qt::FramelessWindowHint);
    m_tabView->setColor(Qt::transparent);
    m_tabWindowProxy = new UkuiQuick::WindowProxy(m_tabView.get());
    m_tabWindowProxy->setWindowType(UkuiQuick::WindowType::Type::Switcher);
    m_tabWindowProxy->setWindowStates(UkuiQuick::WindowState::Focusable | UkuiQuick::WindowState::Activatable, true);

    qDebug() << "[DEBUG]: multitask view uses " << m_tabView->sceneGraphBackend();
    if (m_tabView->rendererInterface()->graphicsApi() < 2) {
        m_isOpenGLBackend = false;
        WindowManagerInterface::getInstance()->setIsOpenGLBackend(m_isOpenGLBackend);
    }

    // 通过 QQmlContext::setContextProperty() 注册 C++ 的对象实例到 QML 中
    m_tabView->rootContext()->setContextProperty("multitaskViewManager", this);
    m_tabView->rootContext()->setContextProperty("multitaskViewModel", WindowManagerInterface::getInstance());
    m_tabView->rootContext()->setContextProperty("appWindowListModel", m_appWindowListModel.get());
    m_tabView->rootContext()->setContextProperty("devicePixelRatio", 1.0); //wlcom环境为固定值

    m_tabView->setSource(m_tabQmlUrl);
    m_tabView->setResizeMode(QQuickView::ResizeMode::SizeViewToRootObject);
    connect(m_tabView.get(), &QQuickView::heightChanged, this, &MultitaskViewManagerByWlcom::updateAltTabView);
    updateAltTabView(); //避免首次打开卡顿

    // 共享数据：注册 C++ 的对象实例到 QML 中
    m_qmlEngine->rootContext()->setContextProperty("multitaskViewManager", this);
    m_qmlEngine->rootContext()->setContextProperty("multitaskViewModel", WindowManagerInterface::getInstance());
    m_qmlEngine->rootContext()->setContextProperty("appWindowListModel", m_appWindowListModel.get());
    m_qmlEngine->rootContext()->setContextProperty("desktopListModel", m_desktopListModel.get());
    m_qmlEngine->rootContext()->setContextProperty("devicePixelRatio", 1.0); //wlcom环境为固定值

    //init win-tab view
    setQmlUrl();
    for(Screen *screen : getScreens()) {
        auto taskView = createTaskView(screen);
        m_taskViews.insert(screen, taskView);
    }
    // 失焦退出
    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this]() {
        if (!winTabVisible()) {
            return;
        }
        for (const auto &view : m_taskViews) {
            if (view->activeFocusItem()) {
                return;
            }
        }
        close();
    }, Qt::ConnectionType::UniqueConnection);

    //init DesktopBallonTip
    QQmlComponent desktopTip(m_qmlEngine.get(),  QUrl(QStringLiteral("qrc:/qml/DesktopBallonTip.qml")), this);
    if (desktopTip.status() == QQmlComponent::Error) {
        qWarning() << "QML Component Error: " << desktopTip.errorString();
        return; // 提前返回，避免后续空指针崩溃
    }
    QObject *tipObject = desktopTip.create();
    if (!tipObject) {
        qCritical() << "Failed to create DesktopBallonTip instance. Check parent context or QML syntax.";
        return;
    }
}

void MultitaskViewManagerByWlcom::destroyView()
{
    hideTabView();
    hideTaskViews();

    // 1. 由于可以在QML中通过JS调用到close函数，
    // 因此，不能直接销毁 m_view 对象；必须延迟
    // 销毁该对象
    // 2. m_model 对象可以立刻销毁。但是，为了对称
    // 和简化逻辑，需要保证 m_view 对象和 m_model 对象的
    // 互相依赖的关系。因此，同时延迟销毁 m_model 对象。
    // 3. MultitaskViewManagerByWlcom的析构函数需要确保
    // m_view 对象和 m_model 对象完成销毁。为了简化逻辑，
    // MultitaskViewManagerByWlcom的析构函数选择直接调用
    // destroyView函数，即采用了延迟销毁的做法。
    m_tabView.release()->deleteLater();
    m_desktopListModel.release()->deleteLater();
    m_appWindowListModel.release()->deleteLater();

    // 提取所有视图指针
    QList<QQuickView*> views = m_taskViews.values();
    // 清空 Map，解除键值关联
    m_taskViews.clear();
    // 对所有视图调用 deleteLater
    for (QQuickView* view : views) {
        if (view) {
            view->deleteLater();
        }
    }
}

void MultitaskViewManagerByWlcom::showTabView()
{
    if (m_tabView == nullptr || m_tabWindowProxy == nullptr) {
        qWarning() << "ukui-window-switch alt-tab view is null, open failed!";
        return;
    }

    //显示alt-tab界面
    updateAltTabView();
    m_tabView->show();
    emit altTabVisibleChanged();
}

void MultitaskViewManagerByWlcom::hideTabView()
{
    if (m_tabView == nullptr) {
        return;
    }
    if (m_tabView->isVisible()) {
        m_tabView->hide();
        emit altTabVisibleChanged();
    }
}

QQuickView *MultitaskViewManagerByWlcom::createTaskView(Screen *screen)
{
    if (screen) {
        auto taskView = new QQuickView(m_qmlEngine.get(), nullptr);
        taskView->setFlags(Qt::FramelessWindowHint);
        taskView->setColor(Qt::transparent);
        taskView->setScreen(screen->internal());

        auto taskWindowProxy = new UkuiQuick::WindowProxy(taskView);
        taskWindowProxy->setWindowType(UkuiQuick::WindowType::Type::Switcher);
        taskWindowProxy->setWindowStates(UkuiQuick::WindowState::Focusable | UkuiQuick::WindowState::Activatable, true);
        taskWindowProxy->setGeometry(screen->geometry());

        int screenIndex = getScreens().indexOf(screen);
        connect(taskView, &QQuickView::statusChanged, taskView, [taskView, screenIndex](const QQuickView::Status status) {
            if (status == QQuickView::Status::Ready) {
                taskView->rootObject()->setProperty("screenIndex", screenIndex);
            }
        });
        connect(screen, &Screen::geometryChanged, taskView, [taskWindowProxy](const QRect geometry) {
            taskWindowProxy->setGeometry(geometry);
        });

        taskView->setResizeMode(QQuickView::ResizeMode::SizeRootObjectToView);
        taskView->setSource(m_qmlUrl);
        return taskView;
    } else {
        return nullptr;
    }
}

void MultitaskViewManagerByWlcom::showTaskViews()
{
    if (m_taskViews.empty()) {
        qWarning() << "ukui-window-switch win-tab view not exist, open failed!";
        return;
    }

    for (auto it = m_taskViews.constBegin(); it != m_taskViews.constEnd(); ++it) {
        if (getScreens().size() > 1) {
            it.value()->rootObject()->setProperty("screenIndex", getScreens().indexOf(it.key()));
        } else {
            it.value()->rootObject()->setProperty("screenIndex", 0);
        }

        if (it.value()->screen() == QGuiApplication::primaryScreen()) {
            it.value()->rootObject()->setProperty("shouldHaveTabFocus", true);
        } else {
            it.value()->show();
        }
    }

    // 初始Tab焦点目标为主屏
    for (const auto &view : m_taskViews) {
        if (view->screen() == QGuiApplication::primaryScreen()) {
            view->show();
            break;
        }
    }

    emit taskViewShowed();

    emit winTabVisibleChanged();
}

void MultitaskViewManagerByWlcom::hideTaskViews()
{
    for (const auto &view : m_taskViews) {
        view->hide();
        if (view->rootObject()->property("shouldHaveTabFocus").toBool() == true) {
            view->rootObject()->setProperty("shouldHaveTabFocus", false);
        }
    }
    emit winTabVisibleChanged();

    if (m_showDesktopTipFlag) {
        m_showDesktopTipFlag = false;
        emit showDesktopTipSignal();
    }
}

void MultitaskViewManagerByWlcom::updateAltTabView()
{
    if (m_tabView == nullptr) {
        return;
    }
    QRect bounding = UkuiQuick::WindowProxy::currentScreen()->geometry();
    if (bounding.width() != m_tabView->width()) {
        // 修复主副屏尺寸不同，alt-tab界面宽度不更新问题
        emit currentScreenRectChanged();
    }
    QPainterPath path;
    qreal yOffset = m_tabView->rootObject()->property("yOffset").toInt();
    int tabviewHeight = m_tabView->height();
    if (m_tabView->height() != m_tabView->rootObject()->height()) {
        // 异常处理 fix bug 589319
        tabviewHeight = m_tabView->rootObject()->height();
    }
    path.addRect(0, yOffset, bounding.width(), tabviewHeight - (yOffset * 2));
    m_tabWindowProxy->setBlurRegion(true, QRegion(path.toFillPolygon().toPolygon())); //毛玻璃
    m_tabWindowProxy->setGeometry(QRect(bounding.x(),bounding.y() + bounding.height() * 0.5 - tabviewHeight * 0.5, bounding.width(), tabviewHeight));
}

void MultitaskViewManagerByWlcom::onTabletModeChanged()
{
    close();
    setQmlUrl();

    if (m_taskViews.isEmpty()) {
        return;
    }
    //load updated qml source
    for (auto &view : m_taskViews) {
        qDebug() << "TabletModeChanged: update qml source, isLoadTabletModeQML = " << m_isLoadTabletModeQML;
        view->setSource(m_qmlUrl);
    }
}

void MultitaskViewManagerByWlcom::setShouldShowTabBorder(bool isShow)
{
    if (m_isShowTabBorder == isShow) {
        return;
    }
    m_isShowTabBorder = isShow;
    emit shouldShowTabBorderChanged();
}

void MultitaskViewManagerByWlcom::connectSignals()
{
    qDebug() << "创建信号" << __FUNCTION__;
    connect(WindowsManager::getInstance(), &WindowsManager::appWindowAdd, this, &MultitaskViewManagerByWlcom::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenCountChanged, this, &MultitaskViewManagerByWlcom::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenSizeChanged, this, &MultitaskViewManagerByWlcom::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenAdd, this, &MultitaskViewManagerByWlcom::onScreenAdd);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenRemove, this, &MultitaskViewManagerByWlcom::onScreenRemove);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::tabletModeChanged, this, &MultitaskViewManagerByWlcom::onTabletModeChanged);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &MultitaskViewManagerByWlcom::currentScreenRectChanged);
    connect(DesktopManager::getInstance(), &DesktopManager::currentDesktopChanged, this, &MultitaskViewManagerByWlcom::showDesktopBallonTip);
    connect(UkuiWindowSwitch::GlobalSettings::globalInstance(), &UkuiWindowSwitch::GlobalSettings::panelPositionChanged, this, &MultitaskViewManagerByWlcom::desktopTipPosChanged);
}

QString MultitaskViewManagerByWlcom::getProjectCodeName()
{
    return QString(kdk_system_get_projectName());
}

QString MultitaskViewManagerByWlcom::getProjectSubName()
{
    return QString(kdk_system_get_projectSubName());
}

QString MultitaskViewManagerByWlcom::getProductFeatures()
{
    return QString(kdk_system_get_productFeatures());
}
