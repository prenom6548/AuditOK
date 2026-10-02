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

#include "panel-top-bar.h"

#include <app-launcher.h>
#include <config-loader.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <kx11extras.h>
#else
#include <KWindowSystem>
#endif
#include <QTimer>
#include <QMenu>
#include <shell.h>
#include <QJsonArray>
#include <QActionGroup>
#include <window-manager.h>
#include "island-drag-helper.h"
#include "widget-model.h"
#include "panel-gsettings.h"

#define PANEL_TOP_BAR_SIZE 32
namespace UkuiPanel {
PanelTopBar::PanelTopBar(Screen* screen, const QString&id, QWindow* parent) : UkuiQuick::IslandView(
    QStringLiteral("panel"), QStringLiteral("ukui-panel"))
{
    qRegisterMetaType<QList<int>>();
    qmlRegisterType<UkuiPanel::WidgetSortModel>("org.ukui.panel.impl", 1, 0, "WidgetSortModel");
    rootContext()->setContextProperty("panelTopBar", this);
    setColor(Qt::transparent);
    setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::Dock);

    initMainContainer();
    setTopBarScreen(screen);
    initIslandsConfig();
    initConfig();

    for(auto island : mainView()->widgets()) {
        m_islands.insert(island->id(), dynamic_cast<UkuiQuick::WidgetContainer *>(island));
    }
    loadActions();
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        setHidden(true);
    });
    connect(mainView(), &UkuiQuick::WidgetContainer::widgetAdded, this, &PanelTopBar::onWidgetAdded);
    connect(mainView(), &UkuiQuick::WidgetContainer::widgetRemoved, this, &PanelTopBar::onWidgetRemoved);
    updateVisbility();
    updateDisableWidget();
    loadMainViewItem();
    updateIslandState();
    //island从顶栏拖拽移除
    connect(IslandDragHelper::self(), &IslandDragHelper::dragRemoveFromTopBar, this, &PanelTopBar::dragRemove);
    //island从任务栏拖拽中
    connect(IslandDragHelper::self(), &IslandDragHelper::dragRemovingFromPanel, this, &PanelTopBar::dragAdding);
    connect(this, &PanelTopBar::heightChanged, this, [this] {
        setTopBarSize(height());
    });
}

PanelTopBar::~PanelTopBar()
{
    qDeleteAll(m_menus);
}

void PanelTopBar::setTopBarScreen(Screen* screen)
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
    connect(screen, &Screen::geometryChanged, this, &PanelTopBar::onScreenGeometryChanged);
}

void PanelTopBar::addIsland(const QString& id)
{
    addContainer(id);
    updateIslandFormAction();
}

void PanelTopBar::removeIsland(const QString& id)
{
    if(m_islands.contains(id)){
        mainView()->addDisableInstance(m_islands.value(id)->instanceId());
        m_islands.remove(id);
    }
}

void PanelTopBar::dragRemove(const QString& id)
{
    QTimer::singleShot(0, this, [this, id]() {
        removeIsland(id);
        updateVisbility();
    });
}

void PanelTopBar::dragAdding(const QString& id, bool addWhenDrop)
{
    updateVisbility();
}

void PanelTopBar::onScreenGeometryChanged(const QRect&geometry)
{
    Q_UNUSED(geometry)
    updateGeometry();
}

void PanelTopBar::resetWorkArea(bool cut)
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

void PanelTopBar::setPosition(UkuiQuick::Types::Pos position)
{
    auto container = mainView();
    if (position != container->position()) {
        // 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
        int panelPosition = 0;
        switch (position) {
            case UkuiQuick::Types::Left:
                panelPosition = 2;
            break;
            case UkuiQuick::Types::Top:
                panelPosition = 1;
            break;
            case UkuiQuick::Types::Right:
                panelPosition = 3;
            break;
            default:
            case UkuiQuick::Types::Bottom:
                panelPosition = 0;
            position = UkuiQuick::Types::Pos::Bottom;
            break;
        }

        container->setPosition(position);
        updateGeometry();

        if (position == UkuiQuick::Types::Left || position == UkuiQuick::Types::Right) {
            container->setOrientation(UkuiQuick::Types::Vertical);
        } else {
            container->setOrientation(UkuiQuick::Types::Horizontal);
        }

        mainView()->config()->setValue(QStringLiteral("position"), position);
    }
}

void PanelTopBar::updateVisbility()
{
    if (windowSuppressed()) {
        activeHideTimer(false);
        applyWindowVisibility();
        return;
    }

    m_dragAdding = IslandDragHelper::self()->dragging() && !IslandDragHelper::self()->islandPendingAdd2Panel();
    if(mainView()->widgets().count() > 0 || m_dragAdding) {
        QTimer::singleShot(1, this, [&]() {
            resetWorkArea(mainView()->widgets().count() > 0);
            setHidden(false);
        });
    } else {
        QTimer::singleShot(1, this, [&]() {
            resetWorkArea(false);
            setHidden(true);
        });
    }
}

void PanelTopBar::setAutoHide(bool autoHide)
{
    if (m_autoHide == autoHide) {
        return;
    }

    m_autoHide = autoHide;

    activeHideTimer(false);
    setHidden(m_autoHide);
    resetWorkArea(!m_autoHide);
}

void PanelTopBar::setHidden(bool hidden)
{
    auto panelContent = rootItem();
    if (!panelContent) {
        return;
    }

    if (m_isHidden != hidden) {
        m_isHidden = hidden;

        hide();
        panelContent->setVisible(!m_isHidden);
        updateMask();
    }

    applyWindowVisibility();
}

void PanelTopBar::setScreenLockHidden(bool hidden)
{
    if (m_windowSuppression.screenLockHidden() == hidden) {
        return;
    }
    m_windowSuppression.setScreenLockHidden(hidden);
    if (hidden) {
        activeHideTimer(false);
        applyWindowVisibility();
    } else {
        updateVisbility();
    }
}

void PanelTopBar::activeHideTimer(bool active)
{
    if (!m_hideTimer) {
        return;
    }
    if (active) {
        m_hideTimer->start();
    }
    else {
        m_hideTimer->stop();
    }
}

void PanelTopBar::onWidgetAdded(UkuiQuick::Widget *widget)
{
    PanelGSettings::instance()->setIslandPosition(widget->id(), 1);
    updateVisbility();
}

void PanelTopBar::onWidgetRemoved(UkuiQuick::Widget *widget)
{
    PanelGSettings::instance()->setIslandPosition(widget->id(), 0);
    updateVisbility();
}

void PanelTopBar::initMainContainer()
{
    UkuiQuick::WidgetContainer::widgetLoader().addWidgetSearchPath(QStringLiteral(":/ukui-panel"));
    loadMainViewWithoutItem(QStringLiteral("org.ukui.panelTopBar"));
}

void PanelTopBar::updateGeometry()
{
    refreshRect();
    updateMask();
    resetWorkArea(true);
}

void PanelTopBar::applyWindowVisibility()
{
    if (windowSuppressed()) {
        hide();
        return;
    }

    show();
}

bool PanelTopBar::windowSuppressed() const
{
    return m_windowSuppression.suppressed();
}

void PanelTopBar::refreshRect()
{
    int panelSize = m_panelTopBarSize;
    auto container = PanelTopBar::mainView();
    const auto margin = container->margin();

    // panelRect 为实际占用的区域，包括外边距（margin）
    QRect screenGeometry = container->screen()->geometry(), panelRect;
    UnavailableArea unavailableArea;
    switch (container->position()) {
        default:
        case UkuiQuick::Types::Pos::Bottom:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, screenGeometry.height() - panelSize, 0, 0);

            unavailableArea.bottomWidth = panelRect.height();
            unavailableArea.bottomStart = panelRect.left();
            unavailableArea.bottomEnd   = panelRect.right();

            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::BottomEdge);
            break;
        case UkuiQuick::Types::Pos::Left:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(0, 0, panelSize - screenGeometry.width(), 0);

            unavailableArea.leftWidth = panelRect.width();
            unavailableArea.leftStart = panelRect.top();
            unavailableArea.leftEnd   = panelRect.bottom();

            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::LeftEdge);
            break;
        case UkuiQuick::Types::Pos::Top:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, 0, 0, panelSize - screenGeometry.height());

            unavailableArea.topWidth = panelRect.height();
            unavailableArea.topStart = panelRect.left();
            unavailableArea.topEnd   = panelRect.right();

            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::TopEdge);
            break;
        case UkuiQuick::Types::Pos::Right:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(screenGeometry.width() - panelSize, 0, 0, 0);

            unavailableArea.rightWidth = panelRect.width();
            unavailableArea.rightStart = panelRect.top();
            unavailableArea.rightEnd   = panelRect.bottom();

            m_windowProxy->slideWindow(UkuiQuick::WindowProxy::RightEdge);
            break;
    }

    QRect rect = panelRect.adjusted(margin->left(), margin->top(), -margin->right(), -margin->bottom());
    container->setGeometry(rect);

    m_windowProxy->setGeometry(panelRect);

    m_unavailableArea = unavailableArea;
}

void PanelTopBar::updateMask()
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
        m_windowProxy->setBlurRegion(true);
    }
    setMask(mask);
}

void PanelTopBar::initConfig()
{
    QMap<QString, int> defaultList;
    defaultList.insert(QStringLiteral("leftMargin"), 0);
    defaultList.insert(QStringLiteral("topMargin"), 0);
    defaultList.insert(QStringLiteral("rightMargin"), 0);
    defaultList.insert(QStringLiteral("bottomMargin"), 0);
    defaultList.insert(QStringLiteral("leftPadding"), 0);
    defaultList.insert(QStringLiteral("topPadding"), 0);
    defaultList.insert(QStringLiteral("rightPadding"), 0);
    defaultList.insert(QStringLiteral("bottomPadding"), 0);
    defaultList.insert(QStringLiteral("panelTopBarSize"), PANEL_TOP_BAR_SIZE);

    auto config = mainView()->config();
    for (const auto &key : defaultList.keys()) {
        if(config->getValue(key).isNull()) {
            config->setValue(key, defaultList.value(key));
        }
    }

    // panelSize
    m_panelTopBarSize = config->getValue(QStringLiteral("panelTopBarSize")).isNull()? PANEL_TOP_BAR_SIZE : config->getValue(QStringLiteral("panelTopBarSize")).toInt();
    // m_panelTopBarSize = 300;

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
    int position = config->getValue(QStringLiteral("position")).isNull()? UkuiQuick::Types::Pos::Top : config->getValue(QStringLiteral("position")).toInt();
    setPosition(UkuiQuick::Types::Pos(position));
    // connect(cont, &UkuiQuick::WidgetContainer::activeChanged, this, &PanelTopBar::onContainerActiveChanged);
}

void PanelTopBar::initIslandsConfig()
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
        mainViewConfig->addChild(QStringLiteral("widgets"), wData);
        order << 0;

        //配置岛
        wData.clear();
        wData.insert(QStringLiteral("id"), "org.ukui.settingsIsland");
        wData.insert(QStringLiteral("instanceId"), 1);
        wData.insert(QStringLiteral("type"), "settings");
        wData.insert(QStringLiteral("minimumWidth"), 192);
        wData.insert(QStringLiteral("maximumWidth"), QWINDOWSIZE_MAX);
        mainViewConfig->addChild(QStringLiteral("widgets"), wData);
        order << 1;

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
                } else {
                    defaultWidgets << QStringLiteral("org.ukui.systemTray");
                    defaultWidgets << QStringLiteral("org.ukui.panel.calendar");
                }
                qDebug() << "defaultWidgets" << type << defaultWidgets;
                QVariantList order;
                for (const auto&widget: defaultWidgets) {
                    int instanceId = order.count();
                    order << instanceId;

                    QVariantMap wData;
                    wData.insert(QStringLiteral("id"), widget);
                    wData.insert(QStringLiteral("instanceId"), instanceId);

                    config->addChild(QStringLiteral("widgets"), wData);
                    config->setValue(QStringLiteral("widgetsOrder"), order);
                }
            }
        }
        mainView()->setDisableInstances(QList<int>{0, 1});
    }
}

void PanelTopBar::addContainer(const QString &id)
{
    const QStringList validId{"org.ukui.dataIsland", "org.ukui.settingsIsland"};
    if(validId.contains(id)) {
        qDebug() << mainView()->widgets();
        for(auto w : mainView()->widgets()) {
            if(w->id() == id) {
                qWarning() << id << "has already been added!";
                return;
            }
        }
        auto mainViewConfig = mainView()->config();
        UkuiQuick::ConfigList islandsConfig = mainViewConfig->children(QStringLiteral("widgets"));
        for(auto child : islandsConfig) {
            if(child->getValue(QStringLiteral("id")) == id) {
                mainView()->removeDisableInstance(child->id().toInt());
                child->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
                if(child->children(QStringLiteral("widgets")).size() != 0) {
                    QList<int> disableInstances;
                    for(auto disabledWidget : mainViewConfig->getValue("disabledWidgets").toStringList()) {
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

                mainView()->addWidget(id, child->id().toInt());
                for (UkuiQuick::Widget* w: mainView()->widgets()) {
                    if(w->id() == id) {
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

void PanelTopBar::loadActions()
{
     auto settings = new QAction(tr("Panel Settings"), this);
    connect(settings, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchAppWithArguments(
            QStringLiteral("/usr/share/applications/ukui-control-center.desktop"), {"-m", "Panel"});
    });
    m_dataIslandActions << settings;
    m_settingsIslandActions << settings;

    auto showDesktop = new QAction(tr("Show Desktop"), this);
    connect(showDesktop, &QAction::triggered, this, [] {
        UkuiQuick::WindowManager::setShowingDesktop(!UkuiQuick::WindowManager::showingDesktop());
    });
    m_dataIslandActions << showDesktop;
    m_settingsIslandActions << showDesktop;

    auto systemMonitor = new QAction(tr("System Monitor"), this);
    connect(systemMonitor, &QAction::triggered, this, [] {
        UkuiQuick::AppLauncher::instance()->launchApp(
            QStringLiteral("/usr/share/applications/ukui-system-monitor.desktop"));
    });
    m_dataIslandActions << systemMonitor;
    m_settingsIslandActions << systemMonitor;

    initIslandFormAction();
    m_dataIslandActions << m_dataIslandFormAction;
    m_settingsIslandActions << m_settingsIslandFormAction;

    auto switchPanel = new QAction(tr("Switch to Classical Panel"), this);
    connect(switchPanel, &QAction::triggered, this, [] {
        QTimer::singleShot(1, []() {
            Shell::self()->switchTo(Shell::PanelType::Version4);
        });
    });
    m_dataIslandActions << switchPanel;
    m_settingsIslandActions << switchPanel;

    for (auto island: m_islands) {
        if (island->id() == QStringLiteral("org.ukui.dataIsland")) {
            island->setActions(m_dataIslandActions);
        } else if (island->id() == QStringLiteral("org.ukui.settingsIsland")) {
            island->setActions(m_settingsIslandActions);
        }
    }
}

void PanelTopBar::changeForm(const QString& id, GeneralConfigDefine::IslandsForm form)
{
    if(form == GeneralConfigDefine::IslandsForm::TopBar) {
        addContainer(id);
    } else {
        if(m_islands.contains(id)) {
            mainView()->addDisableInstance(m_islands.value(id)->config()->id().toInt());
            m_islands.remove(id);
            Q_EMIT islandRemoved(id);
        }
    }
}

void PanelTopBar::initIslandFormAction()
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

void PanelTopBar::updateIslandFormAction()
{
    bool containDataIsland = false;
    bool containSettingsIsland = false;
    for(auto island : m_islands) {
        if(island->id() == QStringLiteral("org.ukui.dataIsland")) {
            containDataIsland = true;
        } else if(island->id() == QStringLiteral("org.ukui.settingsIsland")) {
            containSettingsIsland = true;
        }
    }
    for(auto action : m_dataIslandFormAction->menu()->actions()) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        action->setChecked(form == GeneralConfigDefine::IslandsForm::TopBar && containDataIsland);
    }
    for(auto action : m_settingsIslandFormAction->menu()->actions()) {
        auto form = action->property("form").value<GeneralConfigDefine::IslandsForm>();
        action->setChecked(form == GeneralConfigDefine::IslandsForm::TopBar && containSettingsIsland);
    }
}

void PanelTopBar::updateDisableWidget()
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

    connect(mainView()->config(), &UkuiQuick::Config::configChanged, this, [this] (const QString &key) {
        if (key == QStringLiteral("disabledWidgets")) {
            QStringList disabledWidgets = mainView()->config()->getValue(key).toStringList();
            if (disabledWidgets == m_disabledWidgets) {
                return;
            }

            QStringList tmp;
            tmp.append(disabledWidgets);
            tmp.append(m_disabledWidgets);
            tmp.removeDuplicates();

            for (const auto &item : tmp) {
                disableWidget(item, disabledWidgets.contains(item));
            }
        }
    });
}

void PanelTopBar::disableWidget(const QString &id, bool disable)
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

void PanelTopBar::updateIslandState()
{
    int dataIslandPosition = 0, settingsIslandPosition = 0;

    for (UkuiQuick::Widget* widget : mainView()->widgets()) {
        if (widget->id() == "org.ukui.dataIsland") {
            dataIslandPosition = 1;
        }
        if (widget->id() == "org.ukui.settingsIsland") {
            settingsIslandPosition = 1;
        }
    }

    PanelGSettings::instance()->setIslandPosition("org.ukui.dataIsland", dataIslandPosition);
    PanelGSettings::instance()->setIslandPosition("org.ukui.settingsIsland", settingsIslandPosition);
}

void PanelTopBar::setTopBarSize(int size)
{
    PanelGSettings::instance()->setTopBarSize(size);
}

}
