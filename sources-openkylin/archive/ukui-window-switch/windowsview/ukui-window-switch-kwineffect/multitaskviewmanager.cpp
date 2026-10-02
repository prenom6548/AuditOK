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

#include "multitaskviewmanager.h"

// kf5
#include <KF5/KGlobalAccel/KGlobalAccel>
#include <KF5/KWindowSystem/KWindowSystem>
// kysdk
#include <libkysysinfo.h>
#include <libkydatacollect.h>
// Qt
#include <QAction>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtDBus>
#include <QTranslator>
#include <QLocale>
#include <QtConcurrent>
#include <QGSettings>
//ukui-quick
#include <window-manager.h>
#include <window-helper.h>

#include "../desktopbackground.h"
#include "../windowmanagerinterface.h"
#include "../appwindowfiltermodel.h"
#include "../ukui-flow.h"
#include "../windowsmanager.h"

bool MultitaskViewManager::m_isOpenGLBackend = true;

MultitaskViewManager::MultitaskViewManager()
{
    initializeProjectType();
    registerQmlTypes();
    registerShortcut();
    initDBusService();
    installTranslator();

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
}

MultitaskViewManager::~MultitaskViewManager()
{
    m_dbusService.reset();
    close();
    if (m_translator) {
        m_translator->deleteLater();
        m_translator = nullptr;
    }
}

bool MultitaskViewManager::isCursorNavigationKey(QKeyEvent *e) const
{
    if (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Left ||
        e->key() == Qt::Key_Right || e->key() == Qt::Key_Up ||
        e->key() == Qt::Key_Down || e->key() == Qt::Key_Home ||
        e->key() == Qt::Key_End || e->key() == Qt::Key_PageUp ||
        e->key() == Qt::Key_PageDown) {
        return true;
    }
    return false;
}

void MultitaskViewManager::grabbedKeyboardEvent(QKeyEvent *e)
{
    if (m_taskViews.isEmpty()) {
        return;
    }

    if (e->type() == QEvent::KeyPress && isCursorNavigationKey(e)) {
        if (!m_isShowTabBorder) {
            m_isShowTabBorder = true;
        }
        //当该事件会作为走焦事件处理时，进行数据埋点
        QByteArray keyName = QKeySequence(e->key()).toString().toLatin1();
        QtConcurrent::run([keyName]() {
            const char* text = keyName.data();
            KCustomProperty property[1];
            property[0].key = (char*)"keyName";
            property[0].value = (char*)text;
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
            kdk_dia_append_custom_property(node, property, 1);
            kdk_dia_upload_default(node, (char*)"keyboard_navigation_event", (char*)"ukui-window-switch");
            kdk_dia_data_free(node);
        });
    }

    for (const auto &view : m_taskViews) {
        if (view->rootObject()->property("shouldHaveTabFocus").toBool() == true) {
            view->contentItem()->setFocus(true);
            QCoreApplication::sendEvent(view, e);
            break;
        }
    }
}

void MultitaskViewManager::activeAreaChangeDataCollect(const QString &areaName)
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

void MultitaskViewManager::activateNextTaskView(const int currentIndex)
{
    if (m_taskViews.isEmpty() || currentIndex >= m_taskViews.size()) {
        return;
    }

    // tab 走焦首尾循环
    // 非最后一个屏幕，则索引值+1；最后一个屏幕，则索引值置为0
    int nextIndex = 0;
    if (m_taskViews.size() - 1 > currentIndex) {
        nextIndex = currentIndex + 1;
    }

    for (const auto &view : m_taskViews) {
        if (view->rootObject()->property("screenIndex") == nextIndex) {
            view->rootObject()->setProperty("shouldHaveTabFocus", true);
            view->requestActivate();
        }
    }
}

void MultitaskViewManager::activatePreviousTaskView(const int currentIndex)
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
        }
    }
}

void MultitaskViewManager::open()
{
    qDebug() << "开启多任务视图" << __FUNCTION__ << __LINE__ << isOpen << m_isKeyboardGrab;
    // Alt + Tab 快捷键快速连续触发时，xcb_grab_keyboard_reply 函数返回的 grab 状态为 GrabInvalidTime，然后
    // 导致 grabKeyboard 失败。大致原因为：xcb_grab_keyboard_unchecked() 指定的时间早于 last-keyboard-grab
    // 的时间或晚于当前 X server 的时间。具体原因待确认。
    // 因此在 grabKeyboard 失败后需要再次进行 grabKeyboard 操作。
    if (isOpen && !m_isKeyboardGrab) {
        m_isKeyboardGrab = KWin::effects->grabKeyboard(this);
        KWin::effects->setActiveFullScreenEffect(this);
        return;
    }
    // 可旋转的机器，多任务视图打开的时候，禁用旋转
    if (m_isLoadTabletModeQML && getRotationModeStatus()) {
        setRotationModeStatus(!m_isAutoRotateBeforeShowMultiTask);
    }
    initModel();
    m_isShowTabBorder = false;
    initView();
    if (!m_isKeyboardGrab) {
        m_isKeyboardGrab = KWin::effects->grabKeyboard(this);
        KWin::effects->setActiveFullScreenEffect(this);
    }
    isOpen = true;
    QtConcurrent::run([]() {
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_upload_default(node, (char*)"ukui_window_switch_start_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

void MultitaskViewManager::close()
{
    if (!isOpen) {
        return;
    }
    //可旋转的机器，多任务视图打开前 是打开了旋转的 那么关闭多任务视图的同时  打开旋转
    if (m_isLoadTabletModeQML && m_isAutoRotateBeforeShowMultiTask) {
        setRotationModeStatus(m_isAutoRotateBeforeShowMultiTask);
    }
    if (m_isKeyboardGrab) {
        KWin::effects->ungrabKeyboard();
        m_isKeyboardGrab = false;
    }
    KWin::effects->setActiveFullScreenEffect(nullptr);
    destroyView();
    isOpen = false;
    QtConcurrent::run([]() {
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_upload_default(node, (char*)"ukui_window_switch_exit_event", (char*)"ukui-window-switch");
        kdk_dia_data_free(node);
    });
}

bool MultitaskViewManager::getRotationModeStatus()
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

void MultitaskViewManager::setRotationModeStatus(bool rotationModeStatus)
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

QQuickView *MultitaskViewManager::createTaskView(Screen *screen)
{
    if (screen) {
        auto taskView = new QQuickView(m_qmlEngine.get(), nullptr);
        // 不设置BypassWindowManagerHint则无法显示
        taskView->setFlags(Qt::BypassWindowManagerHint | Qt::FramelessWindowHint);
        taskView->setScreen(screen->internal());

        auto taskWindowProxy = new UkuiQuick::WindowProxy(taskView);
        taskWindowProxy->setWindowType(UkuiQuick::WindowType::Type::Switcher);
        taskWindowProxy->setWindowStates(UkuiQuick::WindowState::Focusable | UkuiQuick::WindowState::Activatable, true);
        taskWindowProxy->setGeometry(screen->geometry());

        int screenIndex = QGuiApplication::screens().indexOf(taskView->screen());
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

void MultitaskViewManager::showTaskViews()
{
    if (m_taskViews.empty()) {
        qWarning() << "ukui-window-switch win-tab view not exist, open failed!";
        return;
    }

    for (const auto &view : m_taskViews) {
        if (QGuiApplication::screens().size() > 1) {
            view->rootObject()->setProperty("screenIndex", QGuiApplication::screens().indexOf(view->screen()));
        } else {
            view->rootObject()->setProperty("screenIndex", 0);
        }
        if (view->screen() == QGuiApplication::primaryScreen()) {
            // 初始Tab焦点目标为主屏，通过grabbedKeyboard设置初始焦点
            view->rootObject()->setProperty("shouldHaveTabFocus", true);
        }
        view->show();
    }
}

void MultitaskViewManager::hideTaskViews()
{
    for (const auto &view : m_taskViews) {
        view->hide();
    }
}

void MultitaskViewManager::updateDesktopBackground()
{
    QString filePath = DesktopBackground::getBackgroundFile();
    if (filePath.isEmpty()) {
        return;
    }

    auto loadImageFinishedCallback = [this](const QImage &image) {
        DesktopBackground::setNonBlurBackground(image);
        DesktopBackground::setBlurBackground(image, 0.7);
    };

    BackgroundImageHandler::loadBackgroundImageAsync(filePath, DesktopBackground::getSystemScalingFactor(),
                                                     DesktopBackground::getBackgroundFillStyle(),
                                                     this, loadImageFinishedCallback);
}

void MultitaskViewManager::setShowingDesktop(bool showing)
{
    KWin::effects->setShowingDesktop(showing);
}

void MultitaskViewManager::setShouldShowTabBorder(bool isShow)
{
    if (m_isShowTabBorder == isShow) {
        return;
    }
    m_isShowTabBorder = isShow;
    emit shouldShowTabBorderChanged();
}

void MultitaskViewManager::initModel()
{
    qDebug() << "创建model数据" << __FUNCTION__;
    m_desktopListModel.reset(new DesktopListModel());
    m_appWindowListModel.reset(new AppWindowListModel());
}

void MultitaskViewManager::initializeProjectType()
{
    if ( getProjectCodeName() == QStringLiteral("V10SP1-edu")
      || getProjectSubName() == QStringLiteral("mavis")
      || getProductFeatures() == 2 ) {
        m_projectType = MultitaskViewManager::TABLET;
    } else if (getProjectCodeName() == QStringLiteral("V10SP1") && getProductFeatures() == '\u0000') {
        // '\u0000' 空格的ASCII码
        m_projectType = MultitaskViewManager::MASTER;
        m_isViewVerticalLayout = false;
    } else {
        m_projectType = MultitaskViewManager::TWOINONE;
        m_isViewVerticalLayout = true;
    }
}

void MultitaskViewManager::setQmlUrl()
{
    if (m_projectType == MultitaskViewManager::TABLET
        || (m_projectType == MultitaskViewManager::TWOINONE && WindowManagerInterface::getInstance()->isTabletMode())) {
        m_qmlUrl = QUrl("qrc:/qml/TabletViewMain.qml");
        m_isLoadTabletModeQML = true;
    } else if (m_projectType == MultitaskViewManager::MASTER
        || (m_projectType == MultitaskViewManager::TWOINONE && !WindowManagerInterface::getInstance()->isTabletMode())) {
        m_qmlUrl = QUrl("qrc:/qml/MultitaskView.qml");
        m_isLoadTabletModeQML = false;
    }
}

void MultitaskViewManager::registerQmlTypes()
{
    qmlRegisterType<DesktopBackground>("org.kylin.background", 3, 0, "DesktopBackground");
    qmlRegisterType<AppWindowFilterModel>("AppWindowFilterModel", 1, 0, "AppWindowFilterModel");
    qmlRegisterType<DesktopListModel>("DesktopListModel", 1, 0, "DesktopListModel");
    qmlRegisterUncreatableType<UkuiPositionerAttached>("org.ukui.positioner", 1, 0, "UkuiPositioner", "UkuiPositioner is an attached property");
    qmlRegisterUncreatableType<UkuiBasePositioner>("org.ukui.positioner", 1, 0, "UkuiBasePositioner", "UkuiBasePositioner is an abstract type that is only available as an attached property.");
    qmlRegisterType<UkuiFlow>("org.ukui.positioner", 1, 0, "UkuiFlow");
}

void MultitaskViewManager::registerShortcut()
{
    auto* action = new QAction(this);
    action->setObjectName("ShowMultitaskView");
    action->setText("Show Multitask View");

    QKeySequence keySeq(Qt::META + Qt::Key_Tab);
    KGlobalAccel::self()->setDefaultShortcut(action, QList<QKeySequence>() << keySeq);
    KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>() << keySeq);
    connect(action, &QAction::triggered, this, &MultitaskViewManager::open);
}

void MultitaskViewManager::initDBusService()
{
    m_dbusService.reset(new DBusService(this));
}

void MultitaskViewManager::installTranslator()
{
    m_translator = new QTranslator(qApp);
    m_translator->load(QLocale::system(), QLatin1String("ukui-window-switch"),
                       QLatin1String("_"), TRANSLATION_DIR);
    QGuiApplication::installTranslator(m_translator);
}

void MultitaskViewManager::initView()
{
    qDebug() << " 初始化多任务视图 " << __FUNCTION__;

    // init shared qmlEngine
    // 作为插件，每次加载时动态创建，退出后可释放内存
    m_qmlEngine.reset(new QQmlEngine());
    // 共享数据：注册 C++ 的对象实例到 QML 中
    m_qmlEngine->rootContext()->setContextProperty("multitaskViewManager", this);
    m_qmlEngine->rootContext()->setContextProperty("multitaskViewModel", WindowManagerInterface::getInstance());
    m_qmlEngine->rootContext()->setContextProperty("appWindowListModel", m_appWindowListModel.get());
    m_qmlEngine->rootContext()->setContextProperty("desktopListModel", m_desktopListModel.get());
    m_qmlEngine->rootContext()->setContextProperty("devicePixelRatio", qApp->primaryScreen()->logicalDotsPerInch() / 96.0);

    setQmlUrl();
    for(Screen *screen : ScreensManager::instance()->screens()) {
        auto taskView = createTaskView(screen);
        m_taskViews.insert(screen, taskView);
    }
    // 通过将active设空使其他窗口失去active状态
    // 否则开始菜单等窗口打开时，打开多任务视图会闪一下
    KWin::effects->activateWindow(nullptr);
    showTaskViews();
    connectSignals();
}

void MultitaskViewManager::destroyView()
{
    for (const auto &view : m_taskViews) {
        view->close();
    }

    // 1. 由于可以在QML中通过JS调用到close函数，
    // 因此，不能直接销毁 m_view 对象；必须延迟
    // 销毁该对象。
    // 2. m_model 对象可以立刻销毁。但是，为了对称
    // 和简化逻辑，需要保证 m_view 对象和 m_model 对象的
    // 互相依赖的关系。因此，同时延迟销毁 m_model 对象。
    // 3. MultitaskViewManager的析构函数需要确保
    // m_view 对象和 m_model 对象完成销毁。为了简化逻辑，
    // MultitaskViewManager的析构函数选择直接调用
    // destroyView函数，即采用了延迟销毁的做法。

    QList<QQuickView*> views = m_taskViews.values();
    m_taskViews.clear();
    for (QQuickView* view : views) {
        if (view) {
            view->deleteLater();
        }
    }
    m_desktopListModel.release()->deleteLater();
    m_appWindowListModel.release()->deleteLater();
    m_qmlEngine.release()->deleteLater();
}

void MultitaskViewManager::connectSignals()
{
    qDebug() << "创建信号" << __FUNCTION__;
    connect(WindowsManager::getInstance(), &WindowsManager::appWindowAdd, this, &MultitaskViewManager::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenCountChanged, this, &MultitaskViewManager::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::screenSizeChanged, this, &MultitaskViewManager::close);
    connect(WindowManagerInterface::getInstance(), &WindowManagerInterface::tabletModeChanged, this, &MultitaskViewManager::close);
    // 作为kwin插件，多任务视图view需要设置Qt::BypassWindowManagerHint，否则无法显示（原因未知）
    // 由于不受窗管管控，需要监听锁屏信号主动退出
    QDBusConnection::sessionBus().connect("org.ukui.ScreenSaver", "/", "org.ukui.ScreenSaver", "lock", this, SLOT(close()));
}

QString MultitaskViewManager::getProjectCodeName()
{
    return QString(kdk_system_get_projectName());
}

QString MultitaskViewManager::getProjectSubName()
{
    return QString(kdk_system_get_projectSubName());
}

QString MultitaskViewManager::getProductFeatures()
{
    return QString(kdk_system_get_productFeatures());
}
