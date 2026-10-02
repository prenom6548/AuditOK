/*
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 */

#include "panel.h"
#include <QUrl>
#include <windowmanager/windowmanager.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <kx11extras.h>
#else
#include <KWindowSystem>
#endif
#include <QMapIterator>
#include <utility>
#include <QMenu>
#include <QGuiApplication>
#include <QPair>

#include <widget.h>
#include <widget-item.h>
#include <config-loader.h>
#include <app-launcher.h>
#include <window-manager.h>
#include <function-control.h>
#include <QJsonArray>

#include "general-config-define.h"
#include "widget-model.h"
#include "shell.h"
#include "data-collector.h"

namespace UkuiPanel {
#define DEFAULT_PANEL_SIZE       48
#define UKUI_PANEL_SETTINGS_ID   "org.ukui.panel.settings"
#define UKUI_PANEL_SETTINGS_PATH "/org/ukui/panel/settings/"
Q_LOGGING_CATEGORY(PANEL_PANEL, "org.ukui.panel.panel")
// 如果需要不同Panel加载不同的Config,将id设置为不同即可
Panel::Panel(Screen *screen, const QString &id, QWindow *parent)
  : UkuiQuick::IslandView(QStringLiteral("panel"), QStringLiteral("ukui-panel")),
    m_id(id)
{
    qRegisterMetaType<QList<int>>();
    qmlRegisterType<UkuiPanel::WidgetSortModel>("org.ukui.panel.impl", 1, 0, "WidgetSortModel");

    rootContext()->setContextProperty("panel", this);
    m_lastConfigVersion = config()->getValue(QStringLiteral("panelConfigVersion")).toString();
    initIsland();

    setColor(Qt::transparent);
    setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);

    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::Dock);

    // contents
    setPanelScreen(screen);
    //基础配置
    initConfig();
    // 加载widgets配置
    loadWidgetsConfig();

    setShowSystemTrayStatus();
    loadMainViewItem();
    // 加载任务栏额外设置项
    initPanelConfig();
    // m_autoHideAction需要先同步配置文件，再加载
    loadActions();

    // 用于执行隐藏的定时器
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        setHidden(true);
    });
    updateHideState();

    // 初始化gsettings,将已经初始化完成的值更新到gsettings中
    initGsettings();

    // 自定义拖拽大小，更新毛玻璃区域
    connect(this, &QQuickView::widthChanged, this, [this]() {
        if (m_isResizingPanel && (mainView()->orientation() == UkuiQuick::Types::Vertical)) {
            calculatePanelRectFromSize(width() - mainView()->margin()->right() - mainView()->margin()->left());
            updateMask();
            setGSettings(QStringLiteral("panelsize"), m_windowRegion.width());
        }
    });
    connect(this, &QQuickView::heightChanged, this, [this]() {
        if (m_isResizingPanel && (mainView()->orientation() == UkuiQuick::Types::Horizontal)) {
            calculatePanelRectFromSize(height() - mainView()->margin()->bottom() - mainView()->margin()->top());
            updateMask();
            setGSettings(QStringLiteral("panelsize"), m_windowRegion.height());
        }
    });

    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), true);
    });
    m_startUp = false;
    qCDebug(PANEL_PANEL) << "panel init finished with id:" << m_id;
}

Panel::~Panel()
{
    mainView()->setScreen(nullptr);
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-v11"));
}

void Panel::initIsland()
{
    const QString defaultViewId = QStringLiteral("org.ukui.panel");
    // 添加panel包所在的qrc路径
    UkuiQuick::WidgetContainer::widgetLoader().addWidgetSearchPath(QStringLiteral(":/ukui-panel"));

    // TODO: 配置错误检查
    if (!loadMainViewWithoutItem(defaultViewId)) {
        // 全部使用默认配置
        auto metadata = UkuiQuick::WidgetMetadata(QStringLiteral(":/ukui-panel/org.ukui.panel"));
        auto cont = new UkuiQuick::WidgetContainer(metadata, this);
        cont->setConfig(UkuiQuick::ConfigLoader::getConfig(QStringLiteral("panel")));
        setMainView(cont);
    }
}

void Panel::setPanelScreen(Screen *screen)
{
    if (!screen || screen == m_screen) {
        return;
    }

    if (m_screen) {
        m_screen->disconnect(this);
    }

    m_screen = screen;
    setScreen(screen->internal());
    mainView()->setScreen(screen->internal());
    updateGeometry();
    connect(screen, &Screen::geometryChanged, this, &Panel::onScreenGeometryChanged);
}

void Panel::onScreenGeometryChanged(const QRect &geometry)
{
    Q_UNUSED(geometry)
    updateLimitSize();
    updateGeometry();
}

void Panel::onConfigChanged(const QString& key)
{
    auto configValue = mainView()->config()->getValue(key);

    if (key == QStringLiteral("position")) {
        setPosition(UkuiQuick::Types::Pos(configValue.toInt()), false);
    } else if (key == QStringLiteral("panelSize")) {
        setPanelSize(configValue.value<int>(), false);
    } else if (key == QStringLiteral("panelSizePolicy")) {
        setPanelPolicy(configValue.value<GeneralConfigDefine::PanelSizePolicy>(), false);
    } else if (key == QStringLiteral("panelAutoHide")) {
        setAutoHide(configValue.toBool(), false);
    } else if (key == QStringLiteral("lockPanel")) {
        setLockPanel(configValue.toBool(), false);
    } else if (key == QStringLiteral("showSystemTrayOnAllPanels")) {
        //根据托盘是否显示在任务栏上面的值更新托盘的启用状态
        setShowSystemTrayOnAllPanels(configValue.toBool());
    } else if (key == QStringLiteral("disabledWidgets")) {
        QStringList disabledWidgets = configValue.toStringList();
        if (disabledWidgets == m_disabledWidgets) {
            return;
        }

        QStringList tmp;
        tmp.append(disabledWidgets);
        tmp.append(m_disabledWidgets);
        tmp.removeDuplicates();

        for (const auto&item : tmp) {
            if (m_controlDisableWidgets.contains(item) || m_controlEnableWidgets.contains(item)) {
                continue;
            }
            disableWidget(item, disabledWidgets.contains(item));
        }
    }
}

void Panel::initPanelConfig()
{
    auto config = mainView()->config();
    QVariant value = config->getValue(QStringLiteral("lockPanel"));
    if (!value.isValid()) {
        value = false;
        config->setValue(QStringLiteral("lockPanel"), value);
    }
    setLockPanel(value.toBool(), false);

    value = config->getValue(QStringLiteral("panelAutoHide"));
    if (!value.isValid()) {
        value = false;
        config->setValue(QStringLiteral("panelAutoHide"), value);
    }
    m_autoHide = value.toBool();
    resetWorkArea(!m_autoHide);

    connect(config, &UkuiQuick::Config::configChanged, this, &Panel::onConfigChanged);
}

void Panel::initConfig()
{
    // border, margin
    QMap<QString, int> defaultList;
    defaultList.insert(QStringLiteral("leftMargin"), 0);
    defaultList.insert(QStringLiteral("topMargin"), 0);
    defaultList.insert(QStringLiteral("rightMargin"), 0);
    defaultList.insert(QStringLiteral("bottomMargin"), 0);
    defaultList.insert(QStringLiteral("leftPadding"), 0);
    defaultList.insert(QStringLiteral("topPadding"), 0);
    defaultList.insert(QStringLiteral("rightPadding"), 0);
    defaultList.insert(QStringLiteral("bottomPadding"), 0);
    defaultList.insert(QStringLiteral("panelMinimumSize"), 48);
    defaultList.insert(QStringLiteral("panelMaximumSize"), 92);
    defaultList.insert(QStringLiteral("panelSize"), DEFAULT_PANEL_SIZE);
    defaultList.insert(QStringLiteral("panelDefaultSize"), DEFAULT_PANEL_SIZE);
    defaultList.insert(QStringLiteral("radius"), 0);
    defaultList.insert(QStringLiteral("position"), UkuiQuick::Types::Pos::Bottom);
    defaultList.insert(QStringLiteral("panelSizePolicy"), GeneralConfigDefine::Small);
    defaultList.insert(QStringLiteral("lockPanel"), false);
    defaultList.insert(QStringLiteral("panelAutoHide"), false);

    auto viewConfig = mainView()->config();

    // 当前版本号不等于PANEL_CONFIG_VERSION时，配置文件内容更新
    if (CONFIG_VERSION != m_lastConfigVersion) {
        if (m_lastConfigVersion.isEmpty()) {
            for (const auto&key : defaultList.keys()) {
                if (!config()->getValue(key).isNull()) {
                    defaultList.insert(key, config()->getValue(key).toInt());
                    config()->removeKey(key);
                }

                if (viewConfig->getValue(key).isNull()) {
                    viewConfig->setValue(key, defaultList.value(key));
                }
            }
            config()->setValue(QStringLiteral("panelConfigVersion"), CONFIG_VERSION);
        } else if (m_lastConfigVersion == "1.0") {
            // 1.0 -> 1.1版本更改默认padding值为0(原来默认值为2），如果用户之前设置过padding将不会修改
            QStringList paddingList = {"bottomPadding", "leftPadding", "topPadding", "rightPadding"};
            for (const auto padding : paddingList) {
                if (viewConfig->getValue(padding).toInt() == 2 || viewConfig->getValue(padding).isNull()) {
                    viewConfig->setValue(padding, defaultList.value(padding));
                }
            }
            config()->setValue(QStringLiteral("panelConfigVersion"), CONFIG_VERSION);

        } else {
            for (const auto&key : defaultList.keys()) {
                viewConfig->setValue(key, defaultList.value(key));
            }
        }
    }

    for (const auto &key : defaultList.keys()) {
        if(viewConfig->getValue(key).isNull()) {
            viewConfig->setValue(key, defaultList.value(key));
        }
    }

    // panelSize
    m_panelMaxSize = viewConfig->getValue(QStringLiteral("panelMaximumSize")).isNull()? 92 : viewConfig->getValue(QStringLiteral("panelMaximumSize")).toInt();
    m_panelMinSize = viewConfig->getValue(QStringLiteral("panelMinimumSize")).isNull()? 48 : viewConfig->getValue(QStringLiteral("panelMinimumSize")).toInt();

    int policy = viewConfig->getValue(QStringLiteral("panelSizePolicy")).isNull()? GeneralConfigDefine::Medium : viewConfig->getValue(QStringLiteral("panelSizePolicy")).toInt();
    if(policy == GeneralConfigDefine::Custom) {
        m_panelSize = viewConfig->getValue(QStringLiteral("customPanelSize")).isNull()? DEFAULT_PANEL_SIZE : viewConfig->getValue(QStringLiteral("customPanelSize")).toInt();
    } else {
        m_panelSize = viewConfig->getValue(QStringLiteral("panelSize")).isNull()? DEFAULT_PANEL_SIZE : viewConfig->getValue(QStringLiteral("panelSize")).toInt();
    }
    setPanelPolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(policy), false);

    // container
    auto cont = mainView();
    cont->setHost(UkuiQuick::WidgetMetadata::Host::Panel);
    cont->setRadius(viewConfig->getValue(QStringLiteral("radius")).toInt());

    auto margin = cont->margin();
    auto padding = cont->padding();
    margin->setLeft(viewConfig->getValue(QStringLiteral("leftMargin")).toInt());
    margin->setTop(viewConfig->getValue(QStringLiteral("topMargin")).toInt());
    margin->setRight(viewConfig->getValue(QStringLiteral("rightMargin")).toInt());
    margin->setBottom(viewConfig->getValue(QStringLiteral("bottomMargin")).toInt());
    padding->setLeft(viewConfig->getValue(QStringLiteral("leftPadding")).toInt());
    padding->setTop(viewConfig->getValue(QStringLiteral("topPadding")).toInt());
    padding->setRight(viewConfig->getValue(QStringLiteral("rightPadding")).toInt());
    padding->setBottom(viewConfig->getValue(QStringLiteral("bottomPadding")).toInt());

    // position
    int position = viewConfig->getValue(QStringLiteral("position")).isNull()? UkuiQuick::Types::Pos::Bottom : viewConfig->getValue(QStringLiteral("position")).toInt();
    setPosition(static_cast<UkuiQuick::Types::Pos>(position), false);
    connect(cont, &UkuiQuick::WidgetContainer::activeChanged, this, &Panel::updateHideState);
}

void Panel::setPosition(UkuiQuick::Types::Pos position, bool updateConfig)
{
    auto container = Panel::mainView();
    if (position != container->position()) {
        switch (position) {
            case UkuiQuick::Types::Left:
                m_panelPosition = 2;
                break;
            case UkuiQuick::Types::Top:
                m_panelPosition = 1;
                break;
            case UkuiQuick::Types::Right:
                m_panelPosition = 3;
                break;
            default:
            case UkuiQuick::Types::Bottom:
                m_panelPosition = 0;
                position = UkuiQuick::Types::Pos::Bottom;
                break;
        }

        container->setPosition(position);
        if (position == UkuiQuick::Types::Left || position == UkuiQuick::Types::Right) {
            container->setOrientation(UkuiQuick::Types::Vertical);
        } else {
            container->setOrientation(UkuiQuick::Types::Horizontal);
        }
        updateLimitSize();
        hide();
        updateGeometry();
        applyWindowVisibility();
        setGSettings(QStringLiteral("panelposition"), m_panelPosition);

        if (updateConfig) {
            mainView()->config()->setValue(QStringLiteral("position"), position);
            DataCollector::setPanelPositionEvent(m_panelPosition);
        }
    }

    if (m_positionAction) {
        updatePositionAction();
    }
}

void Panel::setPanelSize(int size, bool updateConfig)
{
    if (size == m_panelSize) {
        return;
    }

    if (size < m_panelMinSize) {
        size = m_panelMinSize;
    } else if (size > m_panelMaxSize) {
        size = m_panelMaxSize;
    }

    m_panelSize = size;

    updateGeometry();
    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("panelSize"), m_panelSize);
        setGSettings(QStringLiteral("panelsize"), mainView()->orientation() == UkuiQuick::Types::Horizontal
                                                      ? m_windowRegion.height()
                                                      : m_windowRegion.width());
        DataCollector::setPanelSizeEvent(m_panelSize);
    }
}

void Panel::loadWidgetsConfig()
{
    auto containerConfig = mainView()->config();
    containerConfig->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
    QVector<QPair<QString, int>> positionedWidgets {
        {QStringLiteral("org.ukui.menu.starter"), 0},
        {QStringLiteral("org.ukui.panel.search"), 0},
        {QStringLiteral("org.ukui.panel.taskView"), 0},
        {QStringLiteral("org.ukui.panel.separator"), 0},
        {QStringLiteral("org.ukui.panel.taskManager"), 0},
        {QStringLiteral("org.ukui.systemTray"), 1},
        {QStringLiteral("org.ukui.panel.calendar"), 1},
        {QStringLiteral("org.ukui.ai.assistant"), 1},
        {QStringLiteral("org.ukui.panel.showDesktop"), 1}
    };
    if (containerConfig->numberOfChildren(QStringLiteral("widgets")) == 0 ) {
        // 默认加载widgets
        QVariantList order;
        for (const auto& widgetPair : positionedWidgets) {
        int instanceId = order.count();
            order << instanceId;

            QVariantMap wData;
            wData.insert(QStringLiteral("id"), widgetPair.first);
            wData.insert(QStringLiteral("instanceId"), instanceId);
            wData.insert(QStringLiteral("layoutIndex"), widgetPair.second);

            containerConfig->addChild(QStringLiteral("widgets"), wData);
            if(widgetPair.first == QStringLiteral("org.ukui.systemTray")) {
                m_systemTrayInstance = instanceId;
            }
            mainView()->addWidget(widgetPair.first, instanceId);
        }

        containerConfig->setValue(QStringLiteral("widgetsOrder"), order);
        containerConfig->setValue(QStringLiteral("disabledWidgets"), QStringList());
    }

    //1.0增加AI助手按钮 放在右起第二个
    if (containerConfig->getValue(QStringLiteral("panelConfigVersion")).toString() != PANEL_CONFIG_VERSION) {
        auto configList = containerConfig->children("widgets");

        bool needAdd = true;
        for (auto widgetConfig : configList) {
            if (widgetConfig->getValue(QStringLiteral("id")).toString() == QStringLiteral("org.ukui.ai.assistant")) {
                needAdd = false;
                break;
            }
        }
        //增加AI助手按钮
        if (needAdd) {
            QVariantMap wData;
            int instanceId = configList.count();
            wData.insert(QStringLiteral("id"), QStringLiteral("org.ukui.ai.assistant"));
            wData.insert(QStringLiteral("instanceId"), instanceId);
            wData.insert(QStringLiteral("layoutIndex"), 1);
            containerConfig->addChild(QStringLiteral("widgets"), wData);
            mainView()->addWidget(QStringLiteral("org.ukui.ai.assistant"), instanceId);
            QVariantList order = containerConfig->getValue(QStringLiteral("widgetsOrder")).toList();
            order.insert(order.count() - 1, instanceId);
            containerConfig->setValue(QStringLiteral("widgetsOrder"), order);
        }
        containerConfig->setValue(QStringLiteral("panelConfigVersion"), PANEL_CONFIG_VERSION);
    }
    // 添加layoutIndex
    for (auto widgetConfig : containerConfig->children("widgets")) {
        if (!widgetConfig->contains("layoutIndex")) {
            QString id = widgetConfig->getValue("id").toString();
            auto it = std::find_if(positionedWidgets.begin(), positionedWidgets.end(), [&](const QPair<QString, int> &pair) {
                return pair.first == id;});

            if (it != positionedWidgets.end()) {
                widgetConfig->setValue("layoutIndex", it->second);
            } else {
                widgetConfig->setValue("layoutIndex", -1);
            }
        }
    }

    m_disabledWidgets = containerConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
    // 卸载插件选项
    QStringList canRemove;
    canRemove << QStringLiteral("org.ukui.panel.search");
    canRemove << QStringLiteral("org.ukui.panel.taskView");

    for (const auto &widgetPair : positionedWidgets) {
        QString wid = widgetPair.first;
        if (canRemove.contains(wid)) {
            UkuiQuick::WidgetMetadata metadata = UkuiQuick::WidgetContainer::widgetLoader().loadMetadata(wid);
            if (!metadata.isValid()) {
                continue;
            }
            QString actionName = QString(tr("Show")) + metadata.name();
            auto action = new QAction(actionName);
            action->setProperty("widget", wid);
            action->setCheckable(true);
            action->setChecked(!m_disabledWidgets.contains(wid));
            if (wid == "org.ukui.panel.search") {
                action->setIcon(QIcon::fromTheme("kylin-search"));
            } else if (wid == "org.ukui.panel.taskView") {
                action->setIcon(QIcon::fromTheme("ukui-taskview-symbolic"));
            }

            m_widgetActions << action;
            connect(action, &QAction::triggered, this, [this, action] {
                widgetActionTriggered(action);
            });
        }
    }
}

void Panel::updateGeometry()
{
    calculatePanelRectFromSize(m_panelSize);
    setGeometry();
    updateMask();
    resetWorkArea(!m_autoHide);
}

void Panel::setGeometry()
{
    switch (Panel::mainView()->position()) {
        default:
        case UkuiQuick::Types::Pos::Bottom:
            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::BottomEdge);
            break;
        case UkuiQuick::Types::Pos::Left:
            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::LeftEdge);
            break;
        case UkuiQuick::Types::Pos::Top:
            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::TopEdge);
            break;
        case UkuiQuick::Types::Pos::Right:
            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::RightEdge);
            break;
    }

    // TODO: 使用WindowProxy2
    m_windowProxy->setGeometry(m_windowRegion);
}

void Panel::initGsettings()
{
    if (QGSettings::isSchemaInstalled(UKUI_PANEL_SETTINGS_ID)) {
        m_settings = new QGSettings(UKUI_PANEL_SETTINGS_ID, UKUI_PANEL_SETTINGS_PATH, this);

        setGSettings(QStringLiteral("panelposition"), m_panelPosition);
        setGSettings(QStringLiteral("panelsize"), mainView()->orientation() == UkuiQuick::Types::Horizontal
                                                      ? m_windowRegion.height()
                                                      : m_windowRegion.width());
        setGSettings(QStringLiteral("hidepanel"), m_autoHide);
        setGSettings(QStringLiteral("lockpanel"), m_lockPanel);
    }
}

void Panel::calculatePanelRectFromSize(int panelSize)
{
    auto container = Panel::mainView();
    const auto margin = container->margin();

    // panelRect 为实际占用的区域，包括外边距（margin）
    QRect screenGeometry = container->screen()->geometry(), panelRect;
    if (screenGeometry.width() <= 0 || screenGeometry.height() <= 0) {
        screenGeometry.setSize({1920, 1080});
    }
    UnavailableArea unavailableArea;

    switch (container->position()) {
        default:
        case UkuiQuick::Types::Pos::Bottom:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, screenGeometry.height() - panelSize, 0, 0);

            unavailableArea.bottomWidth = panelRect.height();
            unavailableArea.bottomStart = panelRect.left();
            unavailableArea.bottomEnd   = panelRect.right();
            break;
        case UkuiQuick::Types::Pos::Left:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(0, 0, panelSize - screenGeometry.width(), 0);

            unavailableArea.leftWidth = panelRect.width();
            unavailableArea.leftStart = panelRect.top();
            unavailableArea.leftEnd   = panelRect.bottom();
            break;
        case UkuiQuick::Types::Pos::Top:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, 0, 0, panelSize - screenGeometry.height());

            unavailableArea.topWidth = panelRect.height();
            unavailableArea.topStart = panelRect.left();
            unavailableArea.topEnd   = panelRect.right();
            break;
        case UkuiQuick::Types::Pos::Right:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(screenGeometry.width() - panelSize, 0, 0, 0);

            unavailableArea.rightWidth = panelRect.width();
            unavailableArea.rightStart = panelRect.top();
            unavailableArea.rightEnd   = panelRect.bottom();
            break;
    }

    QRect rect = panelRect.adjusted(margin->left(), margin->top(), -margin->right(), -margin->bottom());
    container->setGeometry(rect);
    m_windowRegion = panelRect;
    m_unavailableArea = unavailableArea;
}

void Panel::updateLimitSize()
{
    auto container = Panel::mainView();
    const auto margin = container->margin();
    if (container->orientation() == UkuiQuick::Types::Vertical) {
        setMaximumWidth(m_panelMaxSize + (margin->left() + margin->right()));
        setMinimumWidth(m_panelMinSize + (margin->left() + margin->right()));
        setMaximumHeight(container->screen()->geometry().height());

    } else {
        setMaximumHeight(m_panelMaxSize + (margin->top() + margin->bottom()));
        setMinimumHeight(m_panelMinSize + (margin->top() + margin->bottom()));
        setMaximumWidth(container->screen()->geometry().width());
    }
}

void Panel::setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy, bool updateConfig)
{
    if (sizePolicy != m_sizePolicy) {
        switch (sizePolicy) {
            default:
            case GeneralConfigDefine::Small:
                m_sizePolicy = GeneralConfigDefine::Small;
                setPanelSize(48);
                break;
            case GeneralConfigDefine::Medium:
                m_sizePolicy = GeneralConfigDefine::Medium;
                setPanelSize(72);
                break;
            case GeneralConfigDefine::Large:
                m_sizePolicy = GeneralConfigDefine::Large;
                setPanelSize(92);
                break;
            case GeneralConfigDefine::Custom:
                m_sizePolicy = GeneralConfigDefine::Custom;
                setPanelSize(mainView()->config()->getValue(QStringLiteral("customPanelSize")).toInt());
                break;
        }

        if (updateConfig) {
            mainView()->config()->setValue(QStringLiteral("panelSizePolicy"), m_sizePolicy);
        }
        Q_EMIT enableCustomSizeChanged();
    }

    if (m_sizeAction) {
        updateSizeAction();
    }
}

void Panel::widgetActionTriggered(const QAction *action)
{
    QString id = action->property("widget").toString();
    if (id.isEmpty()) {
        return;
    }

    disableWidget(id, !action->isChecked());
    mainView()->config()->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgets);
}

void Panel::disableWidget(const QString &id, bool disable)
{
    if (disable) {
        // 禁用
        if (m_disabledWidgets.contains(id)) {
            return;
        }
        m_disabledWidgets.append(id);

        // 卸载全部widget
        QVector<UkuiQuick::Widget *> widgets = mainView()->widgets();
        QList<int> instances;
        for (const auto &item: widgets) {
            if (item->id() == id) {
                instances.append(item->instanceId());
            }
        }
        instances.append(mainView()->disableInstances());
        mainView()->setDisableInstances(instances);
    } else {
        // 启用
        if (m_disabledWidgets.removeAll(id) == 0) {
            return;
        }

        // 重新激活全部widget
        UkuiQuick::ConfigList children = mainView()->config()->children(QStringLiteral("widgets"));
        for (const auto &child : children) {
            const QString widgetId = child->getValue(QStringLiteral("id")).toString();
            if (widgetId == id) {
                mainView()->removeDisableInstance(child->id().toInt());
                mainView()->addWidget(widgetId, child->id().toInt());
            }
        }
    }

    for (const auto &action: m_widgetActions) {
        if (action->property("widget").toString() == id) {
            action->setChecked(!disable);
            break;
        }
    }
}

void Panel::loadActions()
{
    QList<QAction *> actions;
    // widget操作
    actions << m_widgetActions;

    // 分割线
    auto *separator = new QAction(this);
    separator->setSeparator(true);
    actions << separator;

    auto showDesktop = new QAction(QIcon::fromTheme("desktop-symbolic"), tr("Show Desktop"), this);
    showDesktop->setProperty("id", "show-desktop");
    connect(showDesktop, &QAction::triggered, this, [] {
        UkuiQuick::WindowManager::setShowingDesktop(!UkuiQuick::WindowManager::showingDesktop());
    });
    actions << showDesktop;

    auto systemMonitor = new QAction(QIcon::fromTheme("ukui-system-monitor"), tr("System Monitor"), this);
    systemMonitor->setProperty("id", "system-monitor");
    connect(systemMonitor, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchApp(QStringLiteral("/usr/share/applications/ukui-system-monitor.desktop"));
    });
    actions << systemMonitor;

    // 分割线
    auto *separator2 = new QAction(this);
    separator2->setSeparator(true);
    actions << separator2;

    // actions
    initSizeAction();
    initPositionAction();
    initLockPanelAction();

    // 调整大小
    actions << m_sizeAction;
    // 调整位置
    actions << m_positionAction;
    actions << m_lockPanelAction;

    // 自动隐藏
    m_autoHideAction = new QAction(QIcon::fromTheme("ukui-taskbar-hide-symbolic"), tr("Auto Hide"), this);
    m_autoHideAction->setProperty("id", "panelAutoHide");
    m_autoHideAction->setCheckable(true);
    m_autoHideAction->setChecked(m_autoHide);
    connect(m_autoHideAction, &QAction::triggered, this, [this] {
        setAutoHide(m_autoHideAction->isChecked());
    });

    actions << m_autoHideAction;

    auto switchPanel = new QAction(tr("Switch to New Panel"), this);
    switchPanel->setProperty("id", "switch-to-new-panel");
    connect(switchPanel, &QAction::triggered, this, [] {
        QTimer::singleShot(1, [](){
            Shell::self()->switchTo(Shell::PanelType::Version5);
        });
    });
    actions << switchPanel;

    // 分割线
    auto *separator3 = new QAction(this);
    separator3->setSeparator(true);
    actions << separator3;

    auto panelSetting = new QAction(QIcon::fromTheme("settings-app-symbolic"), tr("Panel Setting"), this);
    panelSetting->setProperty("id", "panel-setting");
    connect(panelSetting, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchAppWithArguments(QStringLiteral("/usr/share/applications/ukui-control-center.desktop"), {"-m", "Panel"});
    });
    actions << panelSetting;

    mainView()->setActions(actions);
}

void Panel::initSizeAction()
{
    auto *sizeAction = new QAction(QIcon::fromTheme("ukui-taskbar-size-symbolic"), tr("Panel Size"), this);
    sizeAction->setMenu(new QMenu());

    QAction *action = sizeAction->menu()->addAction(tr("Large"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Large);

    action = sizeAction->menu()->addAction(tr("Medium"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Medium);

    action = sizeAction->menu()->addAction(tr("Small"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Small);

    auto customAction = new QAction(tr("Custom"));
    sizeAction->menu()->addAction(customAction);
    customAction->setProperty("sizePolicy", GeneralConfigDefine::Custom);
    customAction->setEnabled(mainView()->config()->contains(QStringLiteral("customPanelSize")));

    connect(sizeAction->menu(), &QMenu::triggered, this, [this](QAction *action) {
        auto sizePolicy = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        setPanelPolicy(sizePolicy);
    });

    m_sizeAction = sizeAction;
    m_sizeAction->setProperty("id", "panelSizePolicy");
    updateSizeAction();
}

void Panel::updateSizeAction()
{
    for (const auto &action: m_sizeAction->menu()->actions()) {
        action->setCheckable(true);
        auto sp = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        if (sp == GeneralConfigDefine::PanelSizePolicy::Custom) {
            action->setEnabled(mainView()->config()->contains(QStringLiteral("customPanelSize")));
        }
        action->setChecked(sp == m_sizePolicy);
    }

    m_sizeAction->setEnabled(!m_lockPanel);
}

void Panel::initPositionAction()
{
    auto *positionAction = new QAction(QIcon::fromTheme("ukui-taskbar-place-symbolic"), tr("Panel Position"), this);
    positionAction->setMenu(new QMenu());

    QAction *action = positionAction->menu()->addAction(tr("Top"));
    action->setProperty("position", UkuiQuick::Types::Top);

    action = positionAction->menu()->addAction(tr("Bottom"));
    action->setProperty("position", UkuiQuick::Types::Bottom);

    action = positionAction->menu()->addAction(tr("Left"));
    action->setProperty("position", UkuiQuick::Types::Left);

    action = positionAction->menu()->addAction(tr("Right"));
    action->setProperty("position", UkuiQuick::Types::Right);

    connect(positionAction->menu(), &QMenu::triggered, this, [this](QAction *action) {
        auto pos = action->property("position").value<UkuiQuick::Types::Pos>();
        setPosition(pos);
    });

    m_positionAction = positionAction;
    m_positionAction->setProperty("id", "panelLocation");

    updatePositionAction();
}

void Panel::updatePositionAction()
{
    for (const auto &action: m_positionAction->menu()->actions()) {
        action->setCheckable(true);
        action->setChecked(action->property("position").value<UkuiQuick::Types::Pos>() == mainView()->position());
    }

    m_positionAction->setEnabled(!m_lockPanel);
}

void Panel::initLockPanelAction()
{
    m_lockPanelAction = new QAction(QIcon::fromTheme("ukui-taskbar-lock-symbolic"), tr("Lock Panel"), this);
    m_lockPanelAction->setProperty("id", "panelLock");
    m_lockPanelAction->setCheckable(true);

    connect(m_lockPanelAction, &QAction::triggered, this, [this] {
        setLockPanel(m_lockPanelAction->isChecked());
    });

    updateLockPanelAction();
}

void Panel::updateLockPanelAction()
{
    if(m_lockPanelAction) {
        m_lockPanelAction->setChecked(m_lockPanel);
    }
    if(m_sizeAction) {
        m_sizeAction->setEnabled(!m_lockPanel);
    }
    if(m_positionAction) {
        m_positionAction->setEnabled(!m_lockPanel);
    }
}

void Panel::resetWorkArea(bool cut)
{
    if (cut) {
        // 为窗管添加不可显示区域
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        KX11Extras::setExtendedStrut(winId(),
                                     m_unavailableArea.leftWidth, m_unavailableArea.leftStart, m_unavailableArea.leftEnd,
                                     m_unavailableArea.rightWidth, m_unavailableArea.rightStart, m_unavailableArea.rightEnd,
                                     m_unavailableArea.topWidth, m_unavailableArea.topStart, m_unavailableArea.topEnd,
                                     m_unavailableArea.bottomWidth, m_unavailableArea.bottomStart, m_unavailableArea.bottomEnd
                                     );
#else
        KWindowSystem::setExtendedStrut(winId(),
                                        m_unavailableArea.leftWidth, m_unavailableArea.leftStart, m_unavailableArea.leftEnd,
                                        m_unavailableArea.rightWidth, m_unavailableArea.rightStart, m_unavailableArea.rightEnd,
                                        m_unavailableArea.topWidth, m_unavailableArea.topStart, m_unavailableArea.topEnd,
                                        m_unavailableArea.bottomWidth, m_unavailableArea.bottomStart, m_unavailableArea.bottomEnd
                                        );
#endif
        m_windowProxy->setPanelAutoHide(false);
    } else {
        // 为窗管移除不可显示区域
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        KX11Extras::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0
                                     );
#else
        KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0
                                        );
#endif

        m_windowProxy->setPanelAutoHide(true);
    }
}

/**
 * 设置panel是否自动隐藏
 * 如果自动隐藏，那么需要向窗管归还可用区域
 * 将任务栏调整为屏幕边缘的一个小条
 * 并且将内容隐藏
 *
 * @param autoHide
 */
void Panel::setAutoHide(bool autoHide, bool updateConfig)
{
    if (m_autoHide == autoHide) {
        return;
    }

    m_autoHide = autoHide;
    resetWorkArea(!m_autoHide);
    if (m_autoHideAction) {
        m_autoHideAction->setChecked(m_autoHide);
    }
    updateHideState();
    setGSettings(QStringLiteral("hidepanel"), m_autoHide);

    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("panelAutoHide"), m_autoHide);
    }
}

bool Panel::event(QEvent *e)
{
    switch (e->type()) {
        case QEvent::Leave: {
            m_containsMouse = false;
            updateHideState();
            break;
        }
        case QEvent::Enter: {
            m_containsMouse = true;
            updateHideState();
            break;
        }
        default:
            break;
    }

    return QQuickWindow::event(e);
}

void Panel::changeCursor(const QString &type)
{
    if (type.isEmpty()) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    Qt::CursorShape shape = Qt::CustomCursor;
    if (type == "resizeVec") {
        shape = Qt::SizeVerCursor;
    } else if (type == "resizeHor") {
        shape = Qt::SizeHorCursor;
    } else if (type == "move") {
        shape = Qt::SizeAllCursor;
    }

    if (shape == Qt::CustomCursor) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    auto cursor = QGuiApplication::overrideCursor();
    if (cursor && cursor->shape() == shape) {
        return;
    }

    QGuiApplication::setOverrideCursor({shape});
}

bool Panel::enableCustomSize() const
{
    return m_sizePolicy == GeneralConfigDefine::Custom;
}

void Panel::changePanelPos(UkuiQuick::Types::Pos pos)
{
    setPosition(pos);
}

void Panel::startResizeCustomPanelSize(const Qt::Edges &edge)
{
    startSystemResize(edge);
    m_isResizingPanel = true;
}

void Panel::endResizeCustomPanelSize()
{
    int size;
    if (mainView()->orientation() == UkuiQuick::Types::Vertical) {
        size = width() - mainView()->margin()->right() - mainView()->margin()->left();
    }
    else {
        size = height() - mainView()->margin()->bottom() - mainView()->margin()->top();
    }
    if (!mainView()->config()->contains(QStringLiteral("customPanelSize")) || mainView()->config()->getValue(QStringLiteral("customPanelSize")).toInt() != size) {
        mainView()->config()->setValue(QStringLiteral("customPanelSize"), size);
    }
    if (m_panelSize != size) {
        m_panelSize = size;
        mainView()->config()->setValue(QStringLiteral("panelSize"), m_panelSize);
        setGSettings(QStringLiteral("panelsize"), mainView()->orientation() == UkuiQuick::Types::Horizontal
                                                      ? m_windowRegion.height()
                                                      : m_windowRegion.width());
        DataCollector::setPanelSizeEvent(m_panelSize);
    }
    if (m_sizePolicy != GeneralConfigDefine::Custom) {
        setPanelPolicy(GeneralConfigDefine::Custom, true);
    }
    m_isResizingPanel = false;
}

bool Panel::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool Panel::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

bool Panel::disableMouseHover() const
{
    return m_disableMouseHover;
}

bool Panel::disableContextMenu() const
{
    return m_disableContextMenu;
}

void Panel::setLockPanel(bool locked, bool updateConfig)
{
    if (locked == m_lockPanel && !m_startUp) {
        return;
    }

    m_lockPanel = locked;

    if (!m_startUp) {
        updateLockPanelAction();
        emit lockPanelChanged();
    }
    setGSettings(QStringLiteral("lockpanel"), m_lockPanel);

    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("lockPanel"), m_lockPanel);
    }
}

bool Panel::lockPanel() const
{
    return m_lockPanel;
}

void Panel::updateHideState()
{
    if (windowSuppressed()) {
        activeHideTimer(false);
        applyWindowVisibility();
        return;
    }

    if (m_autoHide && !m_containsMouse && !mainView()->active()) {
        if (m_startUp) {
            setHidden(true);
        } else {
            activeHideTimer();
        }
    } else {
        activeHideTimer(false);
        setHidden(false);
    }
}

void Panel::updateMask()
{
    QRegion mask;
    if (m_isHidden) {
        const int maskSize = 4;
        switch (mainView()->position()) {
            case UkuiQuick::Types::Left:
                mask = QRegion(0, 0, maskSize, geometry().height());
                break;
            case UkuiQuick::Types::Top:
                mask = QRegion(0, 0, geometry().width(), maskSize);
                break;
            case UkuiQuick::Types::Right:
                mask = QRegion(geometry().width() - maskSize, 0, maskSize, geometry().height());
                break;
            case UkuiQuick::Types::Bottom:
                mask = QRegion(0, geometry().height() - maskSize, geometry().width(), maskSize);
                break;
            default:
                break;
        }

        m_windowProxy->setBlurRegion(false);

    } else {
        // TODO: region圆角
        auto blurRegion = QRegion(QRect{QPoint{mainView()->margin()->left(), mainView()->margin()->top()}, mainView()->geometry().size()});
        m_windowProxy->setBlurRegion(true, blurRegion);
    }

    setMask(mask);
}

/**
 * 进入隐藏状态，并在过程中执行过渡动画
 * 隐藏状态是指任务栏内容不可见，并在屏幕边缘保留有一个小小的条状区域用于接受鼠标事件，动态激活界面
 *
    基本逻辑分为以下两个部分
    if (hidden) {
         执行动画（由）
         关闭毛玻璃
         隐藏内容
         显示小条
    } else {
         开启毛玻璃
         显示内容
         执行显示动画
    }
 *
 * @param hidden
 */
void Panel::setHidden(bool hidden)
{
    auto panelContent = rootItem();
    if (!panelContent) {
        return;
    }

    if (m_isHidden != hidden || m_startUp) {
        m_isHidden = hidden;

        hide();
        panelContent->setVisible(!m_isHidden);
        updateMask();
    }

    applyWindowVisibility();
}

void Panel::setScreenLockHidden(bool hidden)
{
    if (m_windowSuppression.screenLockHidden() == hidden) {
        return;
    }
    m_windowSuppression.setScreenLockHidden(hidden);
    if (hidden) {
        activeHideTimer(false);
        applyWindowVisibility();
    } else {
        if (windowSuppressed()) {
            applyWindowVisibility();
        } else {
            updateHideState();
        }
    }
}

void Panel::setShellHidden(bool hidden)
{
    if (m_windowSuppression.shellHidden() == hidden) {
        return;
    }

    m_windowSuppression.setShellHidden(hidden);
    if (hidden) {
        activeHideTimer(false);
        applyWindowVisibility();
    } else {
        if (windowSuppressed()) {
            applyWindowVisibility();
        } else {
            updateHideState();
        }
    }
}

void Panel::activeHideTimer(bool active)
{
    if (!m_hideTimer) {
        return;
    }

    if (active) {
        m_hideTimer->start();
    } else {
        m_hideTimer->stop();
    }
}

void Panel::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-v11")) {
        return;
    }
    //管控中有值时，设置为管控值，否则设置为配置文件中的值
    auto config = exit? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qCDebug(PANEL_PANEL) << "Panel" << m_id << "function control config:" << config;
    //是否一直隐藏任务栏
    if (config.contains(QStringLiteral("panelHide"))) {
        setControlHidden(config.value(QStringLiteral("panelHide")).toBool());
    } else {
        setControlHidden(false);
    }
    //任务栏是否自动隐藏
    if (config.contains(QStringLiteral("panelAutoHide"))) {
        setAutoHide(config.value(QStringLiteral("panelAutoHide")).toBool(), false);
    } else {
        setAutoHide(mainView()->config()->getValue(QStringLiteral("panelAutoHide")).toBool(), false);
    }
    //是否锁定任务栏
    if (config.contains(QStringLiteral("panelLock"))) {
        setLockPanel(config.value(QStringLiteral("panelLock")).toBool(), false);
        updateLockPanelAction();
    } else {
        setLockPanel(mainView()->config()->getValue(QStringLiteral("lockPanel")).toBool(), false);
    }
    //任务栏位置 0-底部；1-左侧；2-顶部；3-右侧
    if (config.contains(QStringLiteral("panelLocation"))) {
        int pos = config.value(QStringLiteral("panelLocation")).toInt();
        switch (pos) {
            default:
            case GeneralConfigDefine::Bottom:
                pos = UkuiQuick::Types::Bottom;
                break;
            case GeneralConfigDefine::Left:
                pos = UkuiQuick::Types::Left;
                break;
            case GeneralConfigDefine::Top:
                pos = UkuiQuick::Types::Top;
                break;
            case GeneralConfigDefine::Right:
                pos = UkuiQuick::Types::Right;
                break;
        }
        setPosition(static_cast<UkuiQuick::Types::Pos>(pos), false);
    } else {
        setPosition(static_cast<UkuiQuick::Types::Pos>(mainView()->config()->getValue(QStringLiteral("position")).toInt()), false);
    }
    //任务栏尺寸 0-小；1-中；2-大；3-自定义
    if (config.contains(QStringLiteral("panelSizePolicy"))) {
        setPanelPolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(config.value(QStringLiteral("panelSizePolicy")).toInt()), false);
    } else {
        setPanelPolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(mainView()->config()->getValue(QStringLiteral("panelSizePolicy")).toInt()), false);
    }

    //任务栏主体的右键菜单中禁用的项
    QStringList disableActions;
    for (QVariant&action: config.value(QStringLiteral("contextMenuDisableActions")).toArray().toVariantList()) {
        disableActions.append(action.toString());
    }

    bool lock = m_lockPanelAction->isChecked();
    for (auto &action: mainView()->actions()) {
        if (disableActions.contains(action->property("id").toString())) {
            action->setEnabled(false);
        } else {
            if (action->property("id").toString() == QStringLiteral("panelSizePolicy")) {
                action->setEnabled(!lock && !config.contains(QStringLiteral("panelSizePolicy")));
            } else if (action->property("id").toString() == QStringLiteral("panelLocation")) {
                action->setEnabled(!lock && !config.contains(QStringLiteral("panelLocation")));
            } else if (action->property("id").toString() == QStringLiteral("panelAutoHide")) {
                action->setEnabled(!config.contains(QStringLiteral("panelAutoHide")));
            } else if (action->property("id").toString() == QStringLiteral("panelLock")) {
                action->setEnabled(!config.contains(QStringLiteral("panelLock")));
            }
        }
    }


    if (config.contains(QStringLiteral("disableWidgets"))) {
        m_controlDisableWidgets.clear();
        for (QVariant &action : config.value(QStringLiteral("disableWidgets")).toArray().toVariantList()) {
            m_controlDisableWidgets.append(action.toString());
        }
    } else {
        m_controlDisableWidgets.clear();
    }
    if (config.contains(QStringLiteral("enableWidgets"))) {
        m_controlEnableWidgets.clear();
        for (QVariant&action: config.value(QStringLiteral("enableWidgets")).toArray().toVariantList()) {
            m_controlEnableWidgets.append(action.toString());
        }
    } else {
        m_controlEnableWidgets.clear();
    }


    for (auto &action: m_widgetActions) {
        QString id = action->property("widget").toString();
        if (id == QStringLiteral("org.ukui.panel.search")
            && (disableActions.contains(QStringLiteral("show-ukui-panel-search")) || m_controlDisableWidgets.contains(id) || m_controlEnableWidgets.contains(id))) {
            action->setEnabled(false);
            continue;
        }
        if (id == QStringLiteral("org.ukui.panel.taskView")
                   && (disableActions.contains(QStringLiteral("show-ukui-panel-taskView")) || m_controlDisableWidgets.contains(id) || m_controlEnableWidgets.contains(id))) {
            action->setEnabled(false);
            continue;
        }
        action->setEnabled(true);
    }
    auto disabledWidgetsInConfig =  mainView()->config()->getValue(QStringLiteral("disabledWidgets")).toStringList();
    for (auto &widgetConfig: mainView()->config()->children("widgets")) {
        QString id = widgetConfig->getValue("id").toString();
        if (m_controlDisableWidgets.contains(id)) {
            disableWidget(id, true);
        } else if (m_controlEnableWidgets.contains(id)) {
            disableWidget(id, false);
        } else {
            disableWidget(id, disabledWidgetsInConfig.contains(id));
        }
    }

    //是否禁用左键点击（任务栏整体禁用）
    if (config.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disable = config.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (disable != m_disableMouseLeftButton) {
            m_disableMouseLeftButton = disable;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        m_disableMouseLeftButton = false;
        Q_EMIT disableMouseLeftButtonChanged();
    }
    //是否禁用右键点击（任务栏整体禁用）
    if (config.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disable = config.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (disable != m_disableMouseRightButton) {
            m_disableMouseRightButton = disable;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        m_disableMouseRightButton = false;
        Q_EMIT disableMouseRightButtonChanged();
    }
    //是否禁用任务栏鼠标悬浮效果（任务栏整体）
    if (config.contains(QStringLiteral("disableMouseHover"))) {
        bool disable = config.value(QStringLiteral("disableMouseHover")).toBool();
        if (disable != m_disableMouseHover) {
            m_disableMouseHover = disable;
            Q_EMIT disableMouseHoverChanged();
        }
    } else {
        m_disableMouseHover = false;
        Q_EMIT disableMouseHoverChanged();
    }
    //是否禁用任务栏主体的右键菜单
    if (config.contains(QStringLiteral("disableContextMenu"))) {
        bool disable = config.value(QStringLiteral("disableContextMenu")).toBool();
        if (disable != m_disableContextMenu) {
            m_disableContextMenu = disable;
            Q_EMIT disableContextMenuChanged();
        }
    } else {
        m_disableContextMenu = false;
        Q_EMIT disableContextMenuChanged();
    }
    //showSystemTrayOnAllPanels
    if (config.contains(QStringLiteral("showSystemTrayOnAllPanels"))) {
        setShowSystemTrayOnAllPanels(config.value(QStringLiteral("showSystemTrayOnAllPanels")).toBool());
    } else {
        setShowSystemTrayOnAllPanels(mainView()->config()->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool());
    }
}

void Panel::primaryScreenChanged(Screen *screen)
{
    if(m_screen == screen) {
        mainView()->addWidget(QStringLiteral("org.ukui.systemTray"), m_systemTrayInstance);
    } else {
        if(mainView()->config()->getValue("showSystemTrayOnAllPanels").toBool()) {
            mainView()->addWidget(QStringLiteral("org.ukui.systemTray"), m_systemTrayInstance);
        } else {
            mainView()->removeWidget(QStringLiteral("org.ukui.systemTray"));
        }
    }
}

void Panel::applyWindowVisibility()
{
    if (windowSuppressed()) {
        hide();
        return;
    }

    show();
}

bool Panel::windowSuppressed() const
{
    return m_windowSuppression.suppressed();
}

void Panel::setControlHidden(bool hidden)
{
    if (m_windowSuppression.controlHidden() == hidden) {
        return;
    }

    m_windowSuppression.setControlHidden(hidden);
    if (hidden) {
        activeHideTimer(false);
        applyWindowVisibility();
    } else {
        if (windowSuppressed()) {
            applyWindowVisibility();
        } else {
            updateHideState();
        }
    }
}

void Panel::setShowSystemTrayStatus()
{
    //读取托盘是否始终显示在任务栏上
    auto value = mainView()->config()->getValue(QStringLiteral("showSystemTrayOnAllPanels"));
    if (!value.isValid()) {
        value = true;
        mainView()->config()->setValue(QStringLiteral("showSystemTrayOnAllPanels"), value);
    }
    //获取系统托盘的instanceid
    if(m_systemTrayInstance == -1) {
        auto widgetsList = mainView()->config()->children(QStringLiteral("widgets"));
        for (const auto widgetConfig : widgetsList) {
            if(widgetConfig->getValue(QStringLiteral("id")).toString() == QStringLiteral("org.ukui.systemTray")) {
                m_systemTrayInstance = widgetConfig->getValue(QStringLiteral("instanceId")).toInt();
                break;
            }
        }
    }
    if(m_screen->internal() != qApp->primaryScreen() && !value.toBool()) {
        mainView()->removeWidget(QStringLiteral("org.ukui.systemTray"));
    }
}

void Panel::setGSettings(const QString& key, const QVariant &value)
{
    if (!m_settings) {
        return;
    }

    if (m_settings->keys().contains(key)) {
        m_settings->set(key, value);
    }
}

void Panel::setShowSystemTrayOnAllPanels(bool isShow)
{
    if (isShow) {
        mainView()->addWidget(QStringLiteral("org.ukui.systemTray"), m_systemTrayInstance);
    } else {
        if (m_screen->internal() != qApp->primaryScreen()) {
            mainView()->removeWidget(QStringLiteral("org.ukui.systemTray"));
        }
    }
}
} // UkuiPanel
