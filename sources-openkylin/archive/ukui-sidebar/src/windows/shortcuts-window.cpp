/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "shortcuts-window.h"
#include "global-settings.h"
#include "shortcut-filter-model.h"
#include "shortcut-model.h"
#include "hand-gesture-helper.h"
#include "event-track.h"
#include "short-cut-manager.h"
#include "screen-monitor.h"

#include <QScreen>
#include <QQuickItem>
#include <QQuickWindow>
#include <QResizeEvent>
#include <QRect>
#include <QVariant>
#include <QEvent>
#include <QGuiApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <KWindowEffects>
#include <KWindowSystem>
#include <QPushButton>
#include <QQmlContext>
#include <QRegion>
#include <QDBusConnection>
#include <QDBusMessage>

#define UKUI_PANEL_NAME          "org.ukui.panel"
#define UKUI_PANEL_PATH          "/panel"
#define UKUI_PANEL_INTERFACE     "org.ukui.panel"
#define UKUI_PANEL_RETURN_KEY    "org.ukui.panel.systemtray.shortcut"

ShortcutsWindow::ShortcutsWindow(QWindow *parent) : SharedEngineView(parent)
{
    // 窗口透明通道需要优先设置
    setColor(Qt::transparent);
    setResizeMode(UkuiQuick::SharedEngineView::SizeViewToRootObject);
    setTitle(tr("Shortcut"));
    // 去除标题栏等
    //setFlags(flags() | Qt::FramelessWindowHint);
    m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    m_layoutDirection = QGuiApplication::layoutDirection();

    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::SystemWindow);

    m_screen = QGuiApplication::primaryScreen();
    setScreen(m_screen);
    connect(m_screen, &QScreen::geometryChanged, this, &ShortcutsWindow::updateGeometry, Qt::UniqueConnection);

    // 加载数据
    initUI();
    initSettings();
    // 信号
    initConnections();

    // 测试切换功能
    //QPushButton *button = new QPushButton("hahahah");
    //connect(button, &QPushButton::clicked, this, [this] {
    //    onTabletModeChanged();
    //});
    //button->resize(200, 100);
    //button->show();
    //KWindowSystem::setType(button->winId(), NET::ScreenLock);
}

void ShortcutsWindow::initUI()
{
    // 上下文属性
    auto editModel = new UkuiShortcut::ShortcutModel(this);
    rootContext()->setContextProperty("editModel", editModel);
    rootContext()->setContextProperty("mainWindow", this);

    // 加载ui
    engine()->addImportPath("qrc:///qml");
    setSource(QUrl("qrc:///qml/Shortcuts.qml"));

    updateEffects();

    connect(editModel, &UkuiShortcut::ShortcutModel::requestExecAction, this, [this] (UkuiShortcut::PluginMetaType::PredefinedAction action) {
        switch (action) {
            default:
            case UkuiShortcut::PluginMetaType::NoAction:
                break;
            case UkuiShortcut::PluginMetaType::Hide:
                hide();
                break;
        }
    });
}

void ShortcutsWindow::initSettings()
{
    QStringList keys = Sidebar::GlobalSettings::globalInstance()->getKeys();
    if (keys.contains(UKUI_PANEL_POSITION_KEY)) {
        m_panelPos = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_POSITION_KEY).toInt();
    }
    if (keys.contains(UKUI_PANEL_SIZE_KEY)) {
        m_panelSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt();
    }
    if (keys.contains(UKUI_PANEL_TYPE_KEY)) {
        m_panelType = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_TYPE_KEY).toInt();
    }
    if (keys.contains(UKUI_SETTINGS_ISLAND_POSITION_KEY)) {
        m_settingsIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_SETTINGS_ISLAND_POSITION_KEY).toInt();
    }
    if (keys.contains(UKUI_TOPBAR_SIZE_KEY)) {
        m_topbarSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_TOPBAR_SIZE_KEY).toInt();
    }
}

void ShortcutsWindow::initConnections()
{
    // 信号
    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == UKUI_PANEL_POSITION_KEY) {
            m_panelPos = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_POSITION_KEY).toInt();
            Q_EMIT panelPosChanged();
            updateGeometry();
        } else if (key == UKUI_PANEL_SIZE_KEY) {
            m_panelSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt();
            updateGeometry();
        } else if (key == UKUI_PANEL_TYPE_KEY) {
            m_panelType = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_PANEL_TYPE_KEY).toInt();
            updateGeometry();
        } else if (key == UKUI_SETTINGS_ISLAND_POSITION_KEY) {
            m_settingsIslandPosition = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_SETTINGS_ISLAND_POSITION_KEY).toInt();
            updateGeometry();
        } else if (key == UKUI_TOPBAR_SIZE_KEY) {
            m_topbarSize = Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_TOPBAR_SIZE_KEY).toInt();
            updateGeometry();
        } else if (key == UKUI_PANEL_LENGTH_KEY) {
            updateGeometry();
        } else if (key == TABLET_MODE) {
            onTabletModeChanged();
        }
    });

    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::screenRemoved, this, [&](QScreen *screen) {
        if(screen == m_screen) {
            activeShortcutsWindow(false);
        }
    }, Qt::UniqueConnection);

    // 失焦退出
    connect(this, &QQuickWindow::activeFocusItemChanged, this, [this] {
        if (!activeFocusItem()) {
            activeShortcutsWindow(false);
        }
    });

    connect(this, &QWindow::visibleChanged, this, [this](bool visible) {
        if (visible) {
            return;
        }

        if (m_restorePanelFocusOnHide) {
            restorePanelFocus();
        } else {
            clearPanelFocusReturn();
        }
        m_restorePanelFocusOnHide = false;
    });

    //布局方向
    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, [this] {
        m_layoutDirection = QGuiApplication::layoutDirection();
    });

    connect(this, &QWindow::widthChanged, this, &ShortcutsWindow::updateGeometry);
    connect(this, &QWindow::heightChanged, this, &ShortcutsWindow::updateGeometry);
}

void ShortcutsWindow::updateGeometry()
{
    if (!m_screen || m_screen->geometry().isEmpty()) {
        return;
    }

    QRect newGeometry;
    if (m_isTabletMode) {
        newGeometry = m_screen->geometry();
    } else {
        int margin = windowMargin();
        //QRect rect = screen()->availableGeometry();
        QRect rect = m_screen->geometry();
        QPoint point;
        UkuiQuick::WindowProxy::SlideFromEdge slideFromEdge = UkuiQuick::WindowProxy::NoEdge;

        if (m_panelType == 1) {
            //三岛任务栏
            if (m_settingsIslandPosition == 0) {

                if (m_layoutDirection == Qt::LayoutDirection::RightToLeft) {
                    point.setX(rect.left() +
                               (rect.width() - Sidebar::GlobalSettings::globalInstance()->getPanelLength(m_screen->name()))/2);
                } else {
                    point.setX(rect.right() -
                               (rect.width() - Sidebar::GlobalSettings::globalInstance()->getPanelLength(m_screen->name()))/2 - width());
                }
                point.setY(rect.bottom() - height() - margin - m_panelSize);
                slideFromEdge = UkuiQuick::WindowProxy::BottomEdge;
            } else if (m_settingsIslandPosition == 1) {

                if (m_layoutDirection == Qt::LayoutDirection::RightToLeft) {
                    point.setX(rect.left() + margin);
                } else {
                    point.setX(rect.right() - width() - margin);
                }
                point.setY(rect.top() + margin + m_topbarSize);
                slideFromEdge = UkuiQuick::WindowProxy::TopEdge;
            }
        } else {
            // 上: 1, 下: 0, 左: 2, 右: 3
            switch (m_panelPos) {
                default:
                case 0:
                    if (m_layoutDirection == Qt::LayoutDirection::RightToLeft) {
                        point.setX(rect.left() + margin);
                    } else {
                        point.setX(rect.right() - width() - margin);
                    }
                    point.setY(rect.bottom() - height() - margin - m_panelSize);
                    slideFromEdge = UkuiQuick::WindowProxy::BottomEdge;
                    break;
                case 1:
                    if (m_layoutDirection == Qt::LayoutDirection::RightToLeft) {
                        point.setX(rect.left() + margin);
                    } else {
                        point.setX(rect.right() - width() - margin);
                    }
                    point.setY(rect.top() + margin + m_panelSize);
                    slideFromEdge = UkuiQuick::WindowProxy::TopEdge;
                    break;
                case 2:
                    point.setX(rect.left() + margin + m_panelSize);
                    point.setY(rect.bottom() - height() - margin);
                    slideFromEdge = UkuiQuick::WindowProxy::LeftEdge;
                    break;
                case 3:
                    point.setX(rect.right() - width() - margin - m_panelSize);
                    point.setY(rect.bottom() - height() - margin);
                    slideFromEdge = UkuiQuick::WindowProxy::RightEdge;
                    break;
            }
        }

        QRect geometry(point, ShortcutsWindow::size());
        if (rootObject()) {
            geometry.setSize(rootObject()->size().toSize());
        }

        newGeometry = geometry;
        m_windowProxy->slideWindow(slideFromEdge, 0);
    }

    if (newGeometry.isEmpty()) return;
    setGeometry(newGeometry);

    if (m_position != newGeometry.topLeft()) {
        m_position = newGeometry.topLeft();
        m_windowProxy->setPosition(m_position);
        Q_EMIT positionChanged();
    }
}

bool ShortcutsWindow::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Show:
            if (QX11Info::isPlatformX11()) {
                requestActivate();
            }
            break;
        default:
            break;
    }
    return SharedEngineView::event(event);
}

bool ShortcutsWindow::isTabletMode() const
{
    return m_isTabletMode;
}

int ShortcutsWindow::windowMargin() const
{
    return 8;
}

int ShortcutsWindow::panelPos() const
{
    return m_panelPos;
}

QString ShortcutsWindow::currentId() const
{
    return m_currentId;
}

void ShortcutsWindow::setCurrentId(QString currentId)
{
    if (m_currentId == currentId) return;
    m_currentId = currentId;
    Q_EMIT currentIdChanged();
}

void ShortcutsWindow::onTabletModeChanged()
{
    //bool toTabletMode = !m_isTabletMode;
    bool toTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    if (toTabletMode) {
        activeShortcutsWindow(false);
    } else {
        hide();
    }

    m_isTabletMode = toTabletMode;
    updateEffects();

    Q_EMIT tabletModeChanged();
}

void ShortcutsWindow::updateEffects()
{
    setResizeMode(m_isTabletMode ? SizeRootObjectToView : SizeViewToRootObject);
    updateGeometry();

    Sidebar::HandGestureHelper::getInstance()->disconnect(this);
    if (m_isTabletMode) {
        //KWindowEffects::slideWindow(this, KWindowEffects::NoEdge);
        connect(Sidebar::HandGestureHelper::getInstance(),&Sidebar::HandGestureHelper::controlCenterCalled,
                this, &ShortcutsWindow::moveShortcutPanel);
        connect(Sidebar::HandGestureHelper::getInstance(), &Sidebar::HandGestureHelper::right2LeftReleased,
                this, &ShortcutsWindow::onRight2LeftReleased);
    } else {
        setBlurStrength();
    }
}

void ShortcutsWindow::moveShortcutPanel(int posX)
{
    if (!rootObject()) {
        return;
    }

    if (!isVisible()) {
        QMetaObject::invokeMethod(rootObject(), "initTabletProp", Qt::DirectConnection);
        show();
    }

    int dx = (m_layoutDirection == Qt::LayoutDirection::RightToLeft)
            ? posX
            : geometry().right() - posX;
    QMetaObject::invokeMethod(rootObject(), "moveShortcutPanel", Qt::DirectConnection, Q_ARG(QVariant, (dx < 0 ? 0 : dx)));
}

void ShortcutsWindow::onRight2LeftReleased(int posX, int posY)
{
    Q_UNUSED(posY)
    if (rootObject()) {
        int dx = (m_layoutDirection == Qt::LayoutDirection::RightToLeft)
                ? posX : geometry().right() - posX;
        bool active = (dx >= geometry().width() * 0.05);
        QMetaObject::invokeMethod(rootObject(), "activePanelOnTablet", Qt::DirectConnection, Q_ARG(QVariant, active));

        if (active) {
            QVariantMap map;
            map.insert("type", "slide");
            Sidebar::EventTrack::instance()->sendSlideEvent("open_sidebar", "sidebar", map);
        }
    }
}

void ShortcutsWindow::setBlurStrength()
{
    if (m_windowProxy == nullptr) return;
    m_windowProxy->setBlurRegion(true);
}

void ShortcutsWindow::requestPanelFocusRestore()
{
    if (isVisible()) {
        m_restorePanelFocusOnHide = true;
    }
}

void ShortcutsWindow::callPanelMethod(const char *method)
{
    QDBusMessage message = QDBusMessage::createMethodCall(UKUI_PANEL_NAME,
                                                           UKUI_PANEL_PATH,
                                                           UKUI_PANEL_INTERFACE,
                                                           method);
    message << UKUI_PANEL_RETURN_KEY;

    if (QDBusConnection::sessionBus().isConnected()) {
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void ShortcutsWindow::restorePanelFocus()
{
    callPanelMethod("restoreFocusByExternalKey");
}

void ShortcutsWindow::clearPanelFocusReturn()
{
    callPanelMethod("clearFocusReturnByExternalKey");
}

void ShortcutsWindow::activeShortcutsWindow(bool active)
{
    if (!rootObject()) {
        hide();
        return;
    }

    if (active == isVisible()) {
        return;
    }

    if (active) {
        m_restorePanelFocusOnHide = false;
    }

    if (m_isTabletMode) {
        if (active) {
            QMetaObject::invokeMethod(rootObject(), "initTabletProp", Qt::DirectConnection);
            show();
        }
        QMetaObject::invokeMethod(rootObject(), "activePanelOnTablet", Qt::DirectConnection, Q_ARG(QVariant, active));

    } else {
        if (active) {
            if (m_screen != UkuiQuick::WindowProxy::currentScreen()) {
                if (m_screen) {
                    m_screen->disconnect(this);
                }
                m_screen = UkuiQuick::WindowProxy::currentScreen();
                this->setScreen(m_screen);
                connect(m_screen, &QScreen::geometryChanged, this, &ShortcutsWindow::updateGeometry, Qt::UniqueConnection);
                updateGeometry();
            }
        } else {
            backToShortcuts();
        }
        setVisible(active);
    }
}

bool ShortcutsWindow::requestMenuWidget(QString widgetId, bool showReturnButton)
{
    QPair<bool, QString> pair = UkuiShortcut::ShortcutManager::getInstance()->getWidgetInfo(widgetId);
    if (pair.first) {
        Q_EMIT showMenuWidget(widgetId, showReturnButton, pair.second);
        return true;
    }
    return false;
}

void ShortcutsWindow::backToShortcuts()
{
    Q_EMIT hideMenuWidget();
}

bool ShortcutsWindow::inTargetCircle(int x, int y, int cx, int cy, int r)
{
    int dx = x - cx;
    int dy = y - cy;
    return (dx * dx + dy * dy) <= (r * r);
}

bool ShortcutsWindow::inTargetRect(int x, int y, QRectF rect)
{
    return rect.contains(x, y);
}
