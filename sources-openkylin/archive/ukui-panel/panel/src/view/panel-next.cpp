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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */


#include <config-loader.h>
#include <KWindowSystem>
#include <app-launcher.h>
#include <QGuiApplication>
#include <window-manager.h>
#include "panel-next.h"

#include "island-drag-helper.h"
#include "widget-model.h"
#include "shell.h"
#include "panel-gsettings.h"
#include <QJsonArray>
#include <QQuickView>
#include <QActionGroup>

#define DEFAULT_PANEL_SIZE       56
#define UKUI_PANEL_SETTINGS_ID   "org.ukui.panel.settings"
#define UKUI_PANEL_SETTINGS_PATH "/org/ukui/panel/settings/"

namespace UkuiPanel {
using WindowManager = UkuiQuick::WindowManager;
Q_LOGGING_CATEGORY(PANEL_PANELNEXT, "org.ukui.panel.panelNext")
void AutoHideHelper::setScreen(Screen* screen)
{
    m_screen = screen;
    update();
}

bool AutoHideHelper::needAutoHide()
{
    return m_needAutoHide;
}

void AutoHideHelper::update()
{
    auto wid = WindowManager::currentActiveWindow();
    bool maximized = WindowManager::isMaximized(wid);
    bool needAutoHide = false;
    if(maximized && m_screen->internal()->geometry().contains(WindowManager::geometry(wid).center())) {
        needAutoHide = true;
    } else {
        needAutoHide = false;
    }
    if(m_needAutoHide != needAutoHide) {
        m_needAutoHide = needAutoHide;

        Q_EMIT autoHideStateChanged(m_needAutoHide);
    }
}

AutoHideHelper::AutoHideHelper(Screen *screen, QObject* parent): QObject(parent)
{
    m_screen = screen;
    update();
    connect(WindowManager::self(), &WindowManager::activeWindowChanged, this, &AutoHideHelper::update);
    connect(WindowManager::self(), &WindowManager::maximizedChanged, this, &AutoHideHelper::update);
    connect(WindowManager::self(), &WindowManager::geometryChanged, this, &AutoHideHelper::update);
}

PanelNext::PanelNext(Screen* screen, const QString&id, QWindow* parent): UkuiQuick::IslandView(
                                                                             QStringLiteral("panel"),
                                                                             QStringLiteral("ukui-panel")),
                                                                         m_id(id)
{
    qRegisterMetaType<QList<int>>();
    qmlRegisterType<UkuiPanel::WidgetSortModel>("org.ukui.panel.impl", 1, 0, "WidgetSortModel");
    rootContext()->setContextProperty("panelNext", this);
    m_blurHelper = new BlurRegionHelper(this);
    connect(m_blurHelper, &BlurRegionHelper::regionUpdated, this, &PanelNext::onBlurRegionChanged);
    rootContext()->setContextProperty("blurRegionHelper", m_blurHelper);
    setColor(Qt::transparent);
    setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::Dock);
    m_windowProxy->setPanelAutoHide(true);
    m_autoHideHelper = new AutoHideHelper(screen, this);
    m_needHide = m_autoHideHelper->needAutoHide();
    //关闭自动隐藏是,任务栏根据当前是否有最大化窗口激活决定是否隐藏
    connect(m_autoHideHelper, &AutoHideHelper::autoHideStateChanged, this, [this](bool hide) {
        m_needHide = hide;
        updateHideState();
    });
    initMainContainer();
    setPanelScreen(screen);
    initConfig();
    initIslandsConfig();
    initPanelConfig();
    loadActions();
    updateDisableWidget();
    loadMainViewItem();
    // 用于执行隐藏的定时器
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        setHidden(true);
    });
    updateHideState();
    // 初始化gsettings,将已经初始化完成的值更新到gsettings中
    initGSettings();
    //island从任务栏拖拽移除
    connect(IslandDragHelper::self(), &IslandDragHelper::dragRemoveFromPanel, this, &PanelNext::dragRemove);
    //island从顶栏拖拽中
    connect(IslandDragHelper::self(), &IslandDragHelper::dragRemovingFromTopBar, this, &PanelNext::dragAdding);
    m_startUp = false;

    connect(this, &QQuickView::heightChanged, this, [this]() {
        if (m_isResizingPanel && (mainView()->orientation() == UkuiQuick::Types::Horizontal)) {
            calculatePanelRectFromSize(height() - mainView()->margin()->bottom() - mainView()->margin()->top());
            setGSettings(QStringLiteral("panelsize"), m_windowRegion.height());
        }
    });
    qCDebug(PANEL_PANELNEXT) << "panel next init finished with id:" << m_id;
}

PanelNext::~PanelNext()
{
    qDeleteAll(m_menus);
}

void PanelNext::initMainContainer()
{
    const QString defaultViewId = QStringLiteral("org.ukui.panelNext");
    // 添加panel包所在的qrc路径
    UkuiQuick::WidgetContainer::widgetLoader().addWidgetSearchPath(QStringLiteral(":/ukui-panel"));

    // TODO: 配置错误检查
    if (!loadMainViewWithoutItem(defaultViewId)) {
        // 全部使用默认配置
        auto metadata = UkuiQuick::WidgetMetadata(QStringLiteral(":/ukui-panel/org.ukui.panelNext"));
        auto cont = new UkuiQuick::WidgetContainer(metadata, this);
        cont->setConfig(UkuiQuick::ConfigLoader::getConfig(QStringLiteral("panel")));
        setMainView(cont);
    }
}

void PanelNext::initIslandsConfig()
{
    auto mainViewConfig = mainView()->config();
    mainViewConfig->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));

    //三岛默认配置
    if (mainViewConfig->numberOfChildren(QStringLiteral("widgets")) == 0) {
        QVariantList order;
        QVariantMap wData;
        //数据岛
        wData.insert(QStringLiteral("id"), "org.ukui.dataIsland");
        wData.insert(QStringLiteral("instanceId"), 0);
        wData.insert(QStringLiteral("type"), "data");
        wData.insert(QStringLiteral("minimumWidth"), 64);
        wData.insert(QStringLiteral("maximumWidth"), 208);
        wData.insert(QStringLiteral("defaultHeight"), 56);
        mainViewConfig->addChild(QStringLiteral("widgets"), wData);
        order << 0;

        //应用岛
        wData.clear();
        wData.insert(QStringLiteral("id"), "org.ukui.appIsland");
        wData.insert(QStringLiteral("instanceId"), 1);
        wData.insert(QStringLiteral("type"), "app");
        wData.insert(QStringLiteral("minimumWidth"), 64);
        wData.insert(QStringLiteral("maximumWidth"), QWINDOWSIZE_MAX);
        wData.insert(QStringLiteral("defaultHeight"), 56);
        mainViewConfig->addChild(QStringLiteral("widgets"), wData);
        order << 1;

        //配置岛
        wData.clear();
        wData.insert(QStringLiteral("id"), "org.ukui.settingsIsland");
        wData.insert(QStringLiteral("instanceId"), 2);
        wData.insert(QStringLiteral("type"), "settings");
        wData.insert(QStringLiteral("minimumWidth"), 192);
        wData.insert(QStringLiteral("maximumWidth"), QWINDOWSIZE_MAX);
        wData.insert(QStringLiteral("defaultHeight"), 56);
        mainViewConfig->addChild(QStringLiteral("widgets"), wData);
        order << 2;

        mainViewConfig->setValue(QStringLiteral("widgetsOrder"), order);

        UkuiQuick::ConfigList islandsConfig = mainView()->config()->children(QStringLiteral("widgets"));
        for (const auto config: islandsConfig) {
            config->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
            //每个岛中的widgets默认配置
            if (config->numberOfChildren(QStringLiteral("widgets")) == 0) {
                // 默认加载widgets
                QStringList defaultWidgets;
                QString type = config->getValue(QStringLiteral("type")).toString();
                if (type == "data") {
                    defaultWidgets << QStringLiteral("org.ukui.menu.starter");
                    defaultWidgets << QStringLiteral("org.ukui.panel.idm");
                    defaultWidgets << QStringLiteral("org.ukui.panel.taskView");
                }
                else if (type == "app") {
                    defaultWidgets << QStringLiteral("org.ukui.panel.taskManager");
                    defaultWidgets << QStringLiteral("org.ukui.panel.search");
                }
                else {
                    defaultWidgets << QStringLiteral("org.ukui.systemTray");
                    defaultWidgets << QStringLiteral("org.ukui.panel.calendar");
                }
                qCDebug(PANEL_PANELNEXT) << "defaultWidgets" << type << defaultWidgets;
                QVariantList order;
                for (const auto&widget: defaultWidgets) {
                    int instanceId = order.count();
                    order << instanceId;

                    QVariantMap wData;
                    wData.insert(QStringLiteral("id"), widget);
                    wData.insert(QStringLiteral("instanceId"), instanceId);

                    config->addChild(QStringLiteral("widgets"), wData);
                }

                config->setValue(QStringLiteral("widgetsOrder"), order);
                config->setValue(QStringLiteral("disabledWidgets"), QStringList());
            }

            mainView()->addWidget(config->getValue(QStringLiteral("id")).toString(),
                                  config->getValue(QStringLiteral("instanceId")).toInt());
        }
    }
    for (UkuiQuick::Widget* w: mainView()->widgets()) {
        m_islands.insert(w->id(), dynamic_cast<UkuiQuick::WidgetContainer *>(w));
    }
}

void PanelNext::setPanelScreen(Screen* screen)
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
    connect(screen, &Screen::geometryChanged, this, &PanelNext::onScreenGeometryChanged);
    m_autoHideHelper->setScreen(screen);
}

bool PanelNext::lockPanel() const
{
    return m_lockPanel;
}

bool PanelNext::enableCustomSize() const
{
    return m_sizePolicy == GeneralConfigDefine::Custom;
}

void PanelNext::addIsland(const QString&id)
{
    changeForm(id, GeneralConfigDefine::Panel);
    updateIslandFormAction();
}

void PanelNext::removeIsland(const QString&id)
{
    if (m_islands.contains(id)) {
        mainView()->addDisableInstance(m_islands.value(id)->instanceId());
        m_islands.remove(id);
    }
}

void PanelNext::dragRemove(const QString& id)
{
    QTimer::singleShot(0, this, [this, id]() {
        changeForm(id, GeneralConfigDefine::TopBar);
    });
}

void PanelNext::dragAdding(const QString& id, bool addWhenDrop)
{
    if(addWhenDrop) {
        addIsland(id);
    } else {
        removeIsland(id);
    }
}

bool PanelNext::addCustomWidget(const QString& id, int instanceId)
{
    UkuiQuick::WidgetContainer *appIsland = m_islands.value(QStringLiteral("org.ukui.appIsland"));
    if(appIsland) {
        QStringList defaultWidgets{QStringLiteral("org.ukui.panel.taskManager"), QStringLiteral("org.ukui.panel.search")};
        if(defaultWidgets.contains(id)) {
            return false;
        }
        if(instanceId >= 0) {
            //判断是否已经存在
            for(auto widget : appIsland->widgets()) {
                if(widget->instanceId() == instanceId) {
                    return false;
                }
            }
            appIsland->addWidget(id, instanceId);
        } else {
            //找到可用的instanceId
            QList<int> usedInstanceId;
            UkuiQuick::ConfigList list = appIsland->config()->children(QStringLiteral("widgets"));
            for(auto config : list) {
                usedInstanceId.append(config->id().toInt());
            }
            for(instanceId = 0;usedInstanceId.contains(instanceId);instanceId++) {}
            QVariantList widgetOrder = appIsland->config()->getValue(QStringLiteral("widgetsOrder")).value<QVariantList>();
            widgetOrder.append(instanceId);
            appIsland->config()->setValue(QStringLiteral("widgetsOrder"), widgetOrder);
            appIsland->addWidget(id, instanceId);
            Q_EMIT customWidgetAdded(id, instanceId);
        }
        return true;
    }
    return false;
}

bool PanelNext::removeCustomWidget(const QString& id, int instanceId)
{
    UkuiQuick::WidgetContainer *appIsland = m_islands.value(QStringLiteral("org.ukui.appIsland"));
    if(appIsland) {
        if(instanceId >= 0) {
            appIsland->removeWidget(id);
            return true;
        }
        QStringList defaultWidgets{QStringLiteral("org.ukui.panel.taskManager"), QStringLiteral("org.ukui.panel.search")};
        if(defaultWidgets.contains(id)) {
            return false;
        }
        appIsland->removeWidget(id);
        //移除配置文件
        UkuiQuick::ConfigList list = appIsland->config()->children(QStringLiteral("widgets"));
        QVariantList widgetOrder = appIsland->config()->getValue(QStringLiteral("widgetsOrder")).value<QVariantList>();
        for(auto config : list) {
            if(config->getValue(QStringLiteral("id")).toString() == id) {
                appIsland->config()->removeChild(QStringLiteral("widgets"), config->id());
                widgetOrder.removeOne(config->id());
                Q_EMIT customWidgetRemoved(id, config->id().toInt());
            }
        }
        appIsland->config()->setValue(QStringLiteral("widgetsOrder"), widgetOrder);
        return true;
    }
    return false;
}

void PanelNext::setPanelLength(int length)
{
    PanelGSettings::instance()->setPanelLength(m_screen->internal()->name(), length);
}

void PanelNext::changeCursor(const QString& type)
{
    if (type.isEmpty()) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    Qt::CursorShape shape = Qt::CustomCursor;
    if (type == "SizeVerCursor") {
        shape = Qt::SizeVerCursor;
    } else if (type == "SizeHorCursor") {
        shape = Qt::SizeHorCursor;
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

void PanelNext::startResizeCustomPanelSize(const Qt::Edges& edge)
{
    if (m_sizePolicy == GeneralConfigDefine::Custom) {
        bool success = startSystemResize(edge);
        m_isResizingPanel = success;
    }
}

void PanelNext::endResizeCustomPanelSize()
{
    if (m_sizePolicy == GeneralConfigDefine::Custom) {
        int size;
        if (mainView()->orientation() == UkuiQuick::Types::Vertical) {
            size = width() - mainView()->margin()->right() - mainView()->margin()->left();
        } else {
            size = height() - mainView()->margin()->bottom() - mainView()->margin()->top();
        }
        if (m_panelSize != size) {
            m_panelSize = size;
            mainView()->config()->setValue(QStringLiteral("customPanelSize"), m_panelSize);
            mainView()->config()->setValue(QStringLiteral("panelSize"), m_panelSize);
            setGSettings(QStringLiteral("panelsize"), m_windowRegion.height());
        }
        m_isResizingPanel = false;
    }
}

void PanelNext::onScreenGeometryChanged(const QRect&geometry)
{
    Q_UNUSED(geometry)
    updateLimitSize();
    updateGeometry();
}

void PanelNext::updateGeometry()
{
    calculatePanelRectFromSize(m_panelSize);
    setGeometry();
    updateMask();
}

void PanelNext::initConfig()
{
    QMap<QString, int> defaultList;
    defaultList.insert(QStringLiteral("leftMargin"), 8);
    defaultList.insert(QStringLiteral("topMargin"), 0);
    defaultList.insert(QStringLiteral("rightMargin"), 8);
    defaultList.insert(QStringLiteral("bottomMargin"), 8);
    defaultList.insert(QStringLiteral("leftPadding"), 2);
    defaultList.insert(QStringLiteral("topPadding"), 2);
    defaultList.insert(QStringLiteral("rightPadding"), 2);
    defaultList.insert(QStringLiteral("bottomPadding"), 2);
    defaultList.insert(QStringLiteral("panelMinimumSize"), 48);
    defaultList.insert(QStringLiteral("panelMaximumSize"), 80);
    defaultList.insert(QStringLiteral("panelSize"), DEFAULT_PANEL_SIZE);
    defaultList.insert(QStringLiteral("panelDefaultSize"), DEFAULT_PANEL_SIZE);
    defaultList.insert(QStringLiteral("customPanelSize"), DEFAULT_PANEL_SIZE);

    auto config = mainView()->config();
    for (const auto&key: defaultList.keys()) {
        if (config->getValue(key).isNull()) {
            config->setValue(key, defaultList.value(key));
        }
    }

    int policy = config->getValue(QStringLiteral("panelSizePolicy")).isNull()
                 ? GeneralConfigDefine::Medium
                 : config->getValue(QStringLiteral("panelSizePolicy")).toInt();
    // panelSize
    if (policy == GeneralConfigDefine::Custom) {
        m_panelSize = config->getValue(QStringLiteral("customPanelSize")).isNull()
                          ? DEFAULT_PANEL_SIZE
                          : config->getValue(QStringLiteral("customPanelSize")).toInt();
    } else {
        m_panelSize = config->getValue(QStringLiteral("panelSize")).isNull()
                          ? DEFAULT_PANEL_SIZE
                          : config->getValue(QStringLiteral("panelSize")).toInt();
    }

    setPanelPolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(policy), false);

    // container
    auto cont = mainView();
    cont->setHost(UkuiQuick::WidgetMetadata::Host::Panel);

    auto margin = cont->margin();
    auto padding = cont->padding();
    margin->setLeft(config->getValue(QStringLiteral("leftMargin")).toInt());
    margin->setTop(config->getValue(QStringLiteral("topMargin")).toInt());
    margin->setRight(config->getValue(QStringLiteral("rightMargin")).toInt());
    margin->setBottom(config->getValue(QStringLiteral("bottomMargin")).toInt());
    padding->setLeft(config->getValue(QStringLiteral("leftPadding")).toInt());
    padding->setTop(config->getValue(QStringLiteral("topPadding")).toInt());
    padding->setRight(config->getValue(QStringLiteral("rightPadding")).toInt());
    padding->setBottom(config->getValue(QStringLiteral("bottomPadding")).toInt());

    // position
    int position = config->getValue(QStringLiteral("position")).isNull()
                       ? UkuiQuick::Types::Pos::Bottom
                       : config->getValue(QStringLiteral("position")).toInt();
    setPosition(static_cast<UkuiQuick::Types::Pos>(position), false);
    connect(cont, &UkuiQuick::WidgetContainer::activeChanged, this, &PanelNext::updateHideState);
}

void PanelNext::setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy, bool updateConfig)
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
                setPanelSize(56);
                break;
            case GeneralConfigDefine::Large:
                m_sizePolicy = GeneralConfigDefine::Large;
                setPanelSize(64);
                break;
            case GeneralConfigDefine::Custom:
                m_sizePolicy = GeneralConfigDefine::Custom;
                setPanelSize(mainView()->config()->getValue(QStringLiteral("customPanelSize")).toInt());
                break;
        }

        if (updateConfig) {
            mainView()->config()->setValue(QStringLiteral("panelSizePolicy"), m_sizePolicy);
        }
        emit enableCustomSizeChanged();
    }
}

void PanelNext::setPosition(UkuiQuick::Types::Pos position, bool updateConfig)
{
    auto container = PanelNext::mainView();
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
        updateGeometry();

        if (updateConfig) {
            mainView()->config()->setValue(QStringLiteral("position"), position);
            setGSettings(QStringLiteral("panelposition"), m_panelPosition);
        }
    }
}

void PanelNext::calculatePanelRectFromSize(int panelSize)
{
    auto container = PanelNext::mainView();
    const auto margin = container->margin();

    // panelRect 为实际占用的区域，包括外边距（margin）
    QRect screenGeometry = container->screen()->geometry(), panelRect;
    switch (container->position()) {
        default:
        case UkuiQuick::Types::Pos::Bottom:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, screenGeometry.height() - panelSize, 0, 0);
            break;
        case UkuiQuick::Types::Pos::Left:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(0, 0, panelSize - screenGeometry.width(), 0);
            break;
        case UkuiQuick::Types::Pos::Top:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, 0, 0, panelSize - screenGeometry.height());
            break;
        case UkuiQuick::Types::Pos::Right:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(screenGeometry.width() - panelSize, 0, 0, 0);
            break;
    }

    m_windowRegion = panelRect;
    QRect rect = panelRect.adjusted(margin->left(), margin->top(), -margin->right(), -margin->bottom());
    container->setGeometry(rect);
}

void PanelNext::setGeometry()
{
    switch (PanelNext::mainView()->position()) {
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

    m_windowProxy->setGeometry(m_windowRegion);
}

void PanelNext::updateMask()
{
    QRegion maskForHover, mask;
    const int maskSize = 4;
    switch (mainView()->position()) {
        case UkuiQuick::Types::Left:
            maskForHover = QRegion(0, 0, maskSize, geometry().height());
        break;
        case UkuiQuick::Types::Top:
            maskForHover = QRegion(0, 0, geometry().width(), maskSize);
        break;
        case UkuiQuick::Types::Right:
            maskForHover = QRegion(geometry().width() - maskSize, 0, maskSize, geometry().height());
        break;
        case UkuiQuick::Types::Bottom:
            maskForHover = QRegion(0, geometry().height() - maskSize, geometry().width(), maskSize);
        break;
        default:
            break;
    }

    if (m_isHidden) {
        m_windowProxy->setBlurRegion(false);
        mask = maskForHover;
    } else {
        auto blurRegion = m_blurHelper->region();
        m_windowProxy->setBlurRegion(true, blurRegion);
        mask = blurRegion.united(maskForHover);
    }
    setMask(mask);
}

void PanelNext::setPanelSize(int size, bool updateConfig)
{
    if (size == m_panelSize) {
        return;
    }

    int minSize = mainView()->config()->getValue(QStringLiteral("panelMinimumSize")).toInt();
    int maxSize = mainView()->config()->getValue(QStringLiteral("panelMaximumSize")).toInt();
    size = std::max(std::min(size, maxSize), minSize);

    m_panelSize = size;
    updateGeometry();
    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("panelSize"), m_panelSize);
        setGSettings(QStringLiteral("panelsize"), m_windowRegion.height());
    }
}

void PanelNext::initPanelConfig()
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

    connect(config, &UkuiQuick::Config::configChanged, this, &PanelNext::onConfigChanged);
}

void PanelNext::onConfigChanged(const QString&key)
{
    auto configValue = mainView()->config()->getValue(key);

    if (key == QStringLiteral("position")) {
        setPosition(UkuiQuick::Types::Pos(configValue.toInt()), false);
    }
    else if (key == QStringLiteral("panelSize")) {
        auto panelSize = configValue.value<int>();
        setPanelSize(panelSize, false);
    }
    else if (key == QStringLiteral("panelSizePolicy")) {
        auto policy = configValue.value<GeneralConfigDefine::PanelSizePolicy>();
        setPanelPolicy(policy, false);
    }
    else if (key == QStringLiteral("panelAutoHide")) {
        setAutoHide(configValue.toBool(), false);
    }
    else if (key == QStringLiteral("lockPanel")) {
        setLockPanel(configValue.toBool(), false);
    }
    else if (key == QStringLiteral("disabledWidgets")) {
        QStringList disabledWidgets = mainView()->config()->getValue(key).toStringList();
        if (disabledWidgets == m_disabledWidgets) {
            return;
        }

        QStringList tmp;
        tmp.append(disabledWidgets);
        tmp.append(m_disabledWidgets);
        tmp.removeDuplicates();

        for (const auto&item : tmp) {
            disableWidget(item, disabledWidgets.contains(item));
        }
    }
}

void PanelNext::loadActions()
{
    auto settings = new QAction(tr("Panel Settings"), this);
    connect(settings, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchAppWithArguments(
            QStringLiteral("/usr/share/applications/ukui-control-center.desktop"), {"-m", "Panel"});
    });
    m_dataIslandActions << settings;
    m_appIslandActions << settings;
    m_settingsIslandActions << settings;

    auto showDesktop = new QAction(tr("Show Desktop"), this);
    connect(showDesktop, &QAction::triggered, this, [] {
        UkuiQuick::WindowManager::setShowingDesktop(!UkuiQuick::WindowManager::showingDesktop());
    });
    m_dataIslandActions << showDesktop;
    m_appIslandActions << showDesktop;
    m_settingsIslandActions << showDesktop;

    auto systemMonitor = new QAction(tr("System Monitor"), this);
    connect(systemMonitor, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchApp(
            QStringLiteral("/usr/share/applications/ukui-system-monitor.desktop"));
    });
    m_dataIslandActions << systemMonitor;
    m_appIslandActions << systemMonitor;
    m_settingsIslandActions << systemMonitor;

    // 分割线
    auto* separator1 = new QAction(this);
    separator1->setSeparator(true);
    m_dataIslandActions << separator1;
    m_appIslandActions << separator1;
    m_settingsIslandActions << separator1;

    // actions
    initSizeAction();
    // initPositionAction();
    // 调整大小
    m_dataIslandActions << m_sizeAction;
    m_appIslandActions << m_sizeAction;
    m_settingsIslandActions << m_sizeAction;

    initIslandFormAction();
    m_dataIslandActions << m_dataIslandFormAction;
    m_settingsIslandActions << m_settingsIslandFormAction;

    // 自动隐藏
    m_autoHideAction = new QAction(tr("Auto Hide"), this);
    m_autoHideAction->setCheckable(true);
    m_autoHideAction->setChecked(m_autoHide);
    connect(m_autoHideAction, &QAction::triggered, this, [this] {
        setAutoHide(m_autoHideAction->isChecked());
    });

    m_dataIslandActions << m_autoHideAction;
    m_appIslandActions << m_autoHideAction;
    m_settingsIslandActions << m_autoHideAction;

    m_lockPanelAction = new QAction(tr("Lock Panel"), this);
    m_lockPanelAction->setCheckable(true);
    m_lockPanelAction->setChecked(m_lockPanel);
    connect(m_lockPanelAction, &QAction::triggered, this, [this] {
        setLockPanel(m_lockPanelAction->isChecked());
    });
    m_dataIslandActions << m_lockPanelAction;
    m_appIslandActions << m_lockPanelAction;
    m_settingsIslandActions << m_lockPanelAction;

    auto switchPanel = new QAction(tr("Switch to Classical Panel"), this);
    connect(switchPanel, &QAction::triggered, this, [] {
        QTimer::singleShot(1, []() {
            Shell::self()->switchTo(Shell::PanelType::Version4);
        });
    });
    m_dataIslandActions << switchPanel;
    m_appIslandActions << switchPanel;
    m_settingsIslandActions << switchPanel;

    for (auto island: m_islands) {
        if (island->id() == QStringLiteral("org.ukui.dataIsland")) {
            island->setActions(m_dataIslandActions);
        } else if (island->id() == QStringLiteral("org.ukui.appIsland")) {
            island->setActions(m_appIslandActions);
        } else if (island->id() == QStringLiteral("org.ukui.settingsIsland")) {
            island->setActions(m_settingsIslandActions);
        }
    }
}

void PanelNext::initSizeAction()
{
    auto* sizeAction = new QAction(tr("Panel Size"), this);
    sizeAction->setMenu(new QMenu());
    m_menus.append(sizeAction->menu());
    auto group = new QActionGroup(sizeAction);

    QAction* action = sizeAction->menu()->addAction(tr("Large"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Large);
    group->addAction(action);

    action = sizeAction->menu()->addAction(tr("Medium"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Medium);
    group->addAction(action);

    action = sizeAction->menu()->addAction(tr("Small"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Small);
    group->addAction(action);

    action = sizeAction->menu()->addAction(tr("Custom"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Custom);
    group->addAction(action);

    connect(group, &QActionGroup::triggered, this, [this](QAction* action) {
        auto sizePolicy = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        setPanelPolicy(sizePolicy);
    });

    m_sizeAction = sizeAction;
    updateSizeAction();
}

void PanelNext::updateSizeAction()
{
    for (const auto&action: m_sizeAction->menu()->actions()) {
        action->setCheckable(true);
        auto sp = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        action->setChecked(sp == m_sizePolicy);
    }
}

void PanelNext::setLockPanel(bool locked, bool updateConfig)
{
    if (locked == m_lockPanel && !m_startUp) {
        return;
    }

    m_lockPanel = locked;
    if (!m_startUp) {
        updatePositionAction();
        emit lockPanelChanged();
    }
    setGSettings(QStringLiteral("lockpanel"), m_lockPanel);

    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("lockPanel"), m_lockPanel);
    }
}

void PanelNext::initPositionAction()
{
    m_lockPanelAction = new QAction(tr("Lock Panel"), this);
    m_lockPanelAction->setCheckable(true);
    m_lockPanelAction->setChecked(m_lockPanel);
    connect(m_lockPanelAction, &QAction::triggered, this, [this] {
        setLockPanel(m_lockPanelAction->isChecked());
    });

    updatePositionAction();
}

void PanelNext::updatePositionAction()
{
    if(m_lockPanelAction) {
        m_lockPanelAction->setChecked(m_lockPanel);
    }
    if(m_sizeAction) {
        m_sizeAction->setEnabled(!m_lockPanel);
    }
}

void PanelNext::initIslandFormAction()
{
    //数据岛特有aciton
    m_dataIslandFormAction = new QAction(tr("Data Island Form"), this);
    m_dataIslandFormAction->setMenu(new QMenu());
    m_menus.append(m_dataIslandFormAction->menu());

    auto group0 = new QActionGroup(m_dataIslandFormAction);
    QAction* action = m_dataIslandFormAction->menu()->addAction(tr("Islands"));
    action->setCheckable(true);
    group0->addAction(action);
    action->setProperty("form", GeneralConfigDefine::IslandsForm::Panel);

    action = m_dataIslandFormAction->menu()->addAction(tr("Top Bar"));
    action->setCheckable(true);
    group0->addAction(action);
    action->setProperty("form", GeneralConfigDefine::IslandsForm::TopBar);

    connect(group0, &QActionGroup::triggered, this, [this](QAction* action) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        changeForm(QStringLiteral("org.ukui.dataIsland"), form);
    });

    //配置岛特有action
    m_settingsIslandFormAction = new QAction(tr("Settings Island Form"), this);
    m_settingsIslandFormAction->setMenu(new QMenu());
    m_menus.append(m_settingsIslandFormAction->menu());

    auto group1 = new QActionGroup(m_dataIslandFormAction);
    action = m_settingsIslandFormAction->menu()->addAction(tr("Islands"));
    action->setCheckable(true);
    group1->addAction(action);
    action->setProperty("form", GeneralConfigDefine::IslandsForm::Panel);

    action = m_settingsIslandFormAction->menu()->addAction(tr("Top Bar"));
    action->setCheckable(true);
    group1->addAction(action);
    action->setProperty("form", GeneralConfigDefine::IslandsForm::TopBar);

    connect(group1, &QActionGroup::triggered, this, [this](QAction* action) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        changeForm(QStringLiteral("org.ukui.settingsIsland"), form);
    });
    updateIslandFormAction();
}

void PanelNext::updateIslandFormAction()
{
    bool containDataIsland = false;
    bool containSettingsIsland = false;
    for (auto island: m_islands) {
        if (island->id() == QStringLiteral("org.ukui.dataIsland")) {
            containDataIsland = true;
        } else if (island->id() == QStringLiteral("org.ukui.settingsIsland")) {
            containSettingsIsland = true;
        }
    }
    for (auto action: m_dataIslandFormAction->menu()->actions()) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        action->setChecked(form == GeneralConfigDefine::IslandsForm::Panel && containDataIsland);
    }
    for (auto action: m_settingsIslandFormAction->menu()->actions()) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        action->setChecked(form == GeneralConfigDefine::IslandsForm::Panel && containSettingsIsland);
    }
}

void PanelNext::setAutoHide(bool autoHide, bool updateConfig)
{
    if (m_autoHide == autoHide) {
        return;
    }
    m_autoHide = autoHide;
    if (m_autoHideAction) {
        m_autoHideAction->setChecked(m_autoHide);
    }
    updateHideState();
    setGSettings(QStringLiteral("hidepanel"), m_autoHide);

    if (updateConfig) {
        mainView()->config()->setValue(QStringLiteral("panelAutoHide"), m_autoHide);
    }
}

void PanelNext::setHidden(bool hidden)
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

void PanelNext::setScreenLockHidden(bool hidden)
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

void PanelNext::setShellHidden(bool hidden)
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

void PanelNext::activeHideTimer(bool active)
{
    if (!m_hideTimer) {
        return;
    }

    if (active) {
        if(m_hideTimer->isActive()) {
            return;
        }
        m_hideTimer->start();
    }
    else {
        m_hideTimer->stop();
    }
}

bool PanelNext::event(QEvent* e)
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

    return UkuiQuick::IslandView::event(e);
}

void PanelNext::onBlurRegionChanged(QRegion region)
{
    if (!m_isHidden) {
        updateMask();
    }
}

void PanelNext::changeForm(const QString&id, GeneralConfigDefine::IslandsForm form)
{
    if (form == GeneralConfigDefine::IslandsForm::TopBar) {
        if (m_islands.contains(id)) {
            mainView()->addDisableInstance(m_islands.value(id)->config()->id().toInt());
            m_islands.remove(id);
            Q_EMIT islandRemoved(id);
        }
    } else {
        if (!m_islands.contains(id)) {
            for (auto config: mainView()->config()->children(QStringLiteral("widgets"))) {
                if (config->getValue(QStringLiteral("id")).toString() == id) {
                    mainView()->removeDisableInstance(config->id().toInt());
                    config->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
                    if(config->children(QStringLiteral("widgets")).size() != 0) {
                        QList<int> disableInstances;
                        for(auto disabledWidget : mainView()->config()->getValue("disabledWidgets").toStringList()) {
                            for (const auto &widget: config->children(QStringLiteral("widgets"))) {
                                if (widget->getValue(QStringLiteral("id")).toString() == disabledWidget) {
                                    disableInstances.append(widget->getValue(QStringLiteral("instanceId")).toInt());
                                }
                            }
                        }
                        QJsonArray jsonArray;
                        for (int value : disableInstances) {
                            jsonArray.append(value);
                        }
                        config->setValue(QStringLiteral("disabledInstances"), jsonArray);
                    }

                    mainView()->addWidget(id, config->id().toInt());
                    for (UkuiQuick::Widget* w: mainView()->widgets()) {
                        if (w->id() == id) {
                            if (id == QStringLiteral("org.ukui.dataIsland")) {
                                w->setActions(m_dataIslandActions);
                            } else if (id == QStringLiteral("org.ukui.settingsIsland")) {
                                w->setActions(m_settingsIslandActions);
                            }
                            m_islands.insert(w->id(), dynamic_cast<UkuiQuick::WidgetContainer *>(w));
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
}

void PanelNext::updateHideState()
{
    if (windowSuppressed()) {
        activeHideTimer(false);
        applyWindowVisibility();
        return;
    }

    if ((m_autoHide || m_needHide) && !m_containsMouse && !mainView()->active()) {
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

void PanelNext::applyWindowVisibility()
{
    if (windowSuppressed()) {
        hide();
        return;
    }

    show();
}

bool PanelNext::windowSuppressed() const
{
    return m_windowSuppression.suppressed();
}

void PanelNext::disableWidget(const QString &id, bool disable)
{
    if (disable) {
        // 禁用
        if (m_disabledWidgets.contains(id)) {
            return;
        }
        m_disabledWidgets.append(id);

        // 卸载全部widget
        for(auto island : m_islands) {
            QVector<UkuiQuick::Widget *> widgets = island->widgets();
            QList<int> instances;
            for (const auto &item: widgets) {
                if (item->id() == id) {
                    instances.append(item->instanceId());
                }
            }
            instances.append(island->disableInstances());
            island->setDisableInstances(instances);
        }
    } else {
        // 启用
        if (m_disabledWidgets.removeAll(id) == 0) {
            return;
        }

        // 重新激活全部widget
        for(auto island : m_islands) {
            UkuiQuick::ConfigList children = island->config()->children(QStringLiteral("widgets"));
            for (const auto &child : children) {
                const QString widgetId = child->getValue(QStringLiteral("id")).toString();
                if (widgetId == id) {
                    island->removeDisableInstance(child->id().toInt());
                    island->addWidget(widgetId, child->id().toInt());
                }
            }
        }
    }
}

void PanelNext::updateDisableWidget()
{
    //初始化disableWidgets
    auto mainViewConfig = mainView()->config();
    if(!mainViewConfig->getValue(QStringLiteral("disabledWidgets")).isValid()) {
        mainViewConfig->setValue(QStringLiteral("disabledWidgets"), QStringList());
    }
    m_disabledWidgets = mainViewConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();

    UkuiQuick::ConfigList islandsConfig = mainViewConfig->children(QStringLiteral("widgets"));
    for(auto child : islandsConfig) {
        child->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
        if(child->children(QStringLiteral("widgets")).size() == 0) {
            continue;
        }
        QList<int> disableInstances;
        for(auto disabledWidget : m_disabledWidgets) {
            for (const auto &widget: child->children(QStringLiteral("widgets"))) {
                if (widget->getValue(QStringLiteral("id")).toString() == disabledWidget) {
                    disableInstances.append(widget->getValue(QStringLiteral("instanceId")).toInt());
                }
            }
        }
        QJsonArray jsonArray;
        for (int value : disableInstances) {
            jsonArray.append(value);
        }
        child->setValue(QStringLiteral("disabledInstances"), jsonArray);
    }
}

void PanelNext::updateLimitSize()
{
    auto container = mainView();
    const auto margin = container->margin();
    int minSize = mainView()->config()->getValue(QStringLiteral("panelMinimumSize")).toInt();
    int maxSize = mainView()->config()->getValue(QStringLiteral("panelMaximumSize")).toInt();
    setMaximumHeight(maxSize + (margin->top() + margin->bottom()));
    setMinimumHeight(minSize + (margin->top() + margin->bottom()));
}

void PanelNext::initGSettings()
{
    if (QGSettings::isSchemaInstalled(UKUI_PANEL_SETTINGS_ID)) {
        m_settings = new QGSettings(UKUI_PANEL_SETTINGS_ID, UKUI_PANEL_SETTINGS_PATH, this);

        setGSettings(QStringLiteral("panelposition"), m_panelPosition);
        setGSettings(QStringLiteral("panelsize"), m_windowRegion.height());
        setGSettings(QStringLiteral("hidepanel"), m_autoHide);
        setGSettings(QStringLiteral("lockpanel"), m_lockPanel);
    }
}

void PanelNext::setGSettings(const QString& key, const QVariant& value)
{
    if (!m_settings) {
        return;
    }

    if (m_settings->keys().contains(key)) {
        m_settings->set(key, value);
    }
}
} // UkuiPanel
