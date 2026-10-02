/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <glib.h>
#include <gio/gio.h>

#include "menu-main-window.h"
#include "settings.h"
#include "context-menu-manager.h"
#include "security-function-control.h"
#include "full-screen-state-helper.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQmlContext>
#include <QPoint>
#include <QPainterPath>
#include <mutex>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif

#define UKUI_PANEL_SETTING             "org.ukui.panel.settings"
#define UKUI_PANEL_POSITION_KEY        "panelposition"
#define UKUI_PANEL_SIZE_KEY            "panelsize"
#define UKUI_PANEL_TYPE_KEY            "paneltype"
#define UKUI_DATA_ISLAND_POSITION_KEY  "dataislandposition"
#define UKUI_TOPBAR_SIZE_KEY           "topbarsize"
#define UKUI_PANEL_LENGTH_KEY          "panellength"

namespace UkuiMenu {

void WindowModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    qmlRegisterRevision<QWindow, 3>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 2>(uri, versionMajor, versionMinor);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    qmlRegisterRevision<QWindow, 2>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 2>(uri, versionMajor, versionMinor);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    qmlRegisterRevision<QWindow, 2>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 1>(uri, versionMajor, versionMinor);
#else
    qmlRegisterRevision<QWindow, 1>(uri, versionMajor, versionMinor);
#endif
#endif
#endif
}

// ====== PanelSettings ====== //
// GCOVR_EXCL_START
static std::once_flag onceFlag;
static PanelGSettings* g_instance = nullptr;
static GSettings * m_settings = nullptr;
static GSettingsSchema * m_schema = nullptr;
static const char *m_panelLengthKey = "panellength";
PanelGSettings *PanelGSettings::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_instance = new PanelGSettings();
    });
    return g_instance;
}

int PanelGSettings::getPanelLength(QString screenName)
{
    if (!m_settings || !m_schema) return -1;
    if (!isKeysContain(m_panelLengthKey)) return -1;

    QMap<QString, QVariant> map = getPanelLengthMap();
    if (!map.contains(screenName)) {
        return -1;
    }
    return map.value(screenName).toInt();
}

PanelGSettings::~PanelGSettings()
{
    if (m_settings) {
        g_object_unref(m_settings);
    }
    if (m_schema) {
        g_settings_schema_unref(m_schema);
    }
    g_instance = nullptr;
}

PanelGSettings::PanelGSettings(QObject *parent) : QObject(parent)
{
    GSettingsSchemaSource *source;

    source = g_settings_schema_source_get_default();
    m_schema = g_settings_schema_source_lookup(source, "org.ukui.panel.settings", true);

    if (!m_schema) {
        m_settings = nullptr;
        return;
    }

    m_settings = g_settings_new_with_path("org.ukui.panel.settings", "/org/ukui/panel/settings/");
}

bool PanelGSettings::isKeysContain(const char *key)
{
    if (!m_settings || !m_schema) return false;
    return g_settings_schema_has_key(m_schema, key);
}

QMap<QString, QVariant> PanelGSettings::getPanelLengthMap()
{
    GVariant *gvalue = g_settings_get_value(m_settings, m_panelLengthKey);
    if (!gvalue) return {};

    GVariantIter iter;
    QMap<QString, QVariant> map;
    const gchar *key;
    GVariant *val = nullptr;

    g_variant_iter_init (&iter, gvalue);
    while (g_variant_iter_next (&iter, "{&sv}", &key, &val)) {
        if (g_variant_is_of_type(val, G_VARIANT_TYPE_UINT32)) {
            map.insert(key, static_cast<quint32>(g_variant_get_uint32(val)));
        }
        g_variant_unref(val);
    }

    g_variant_unref(gvalue);
    return map;
}
// GCOVR_EXCL_STOP

//======MenuWindow======//
MenuWindow::MenuWindow(QWindow *parent) : QQuickView(parent)
{
}

MenuWindow::MenuWindow(QQmlEngine *engine, QWindow *parent)
    : QQuickView(engine, parent)
{
}

int MenuWindow::panelPos() const
{
    return m_panelPos;
}

double MenuWindow::transparency() const
{
    return GlobalSetting::instance()->get(GlobalSetting::Transparency).toDouble();
}

QRect MenuWindow::normalRect() const
{
    return m_normalMaskGeometry;
}

bool MenuWindow::effectEnabled() const
{
    return GlobalSetting::instance()->get(GlobalSetting::EffectEnabled).toBool();
}

bool MenuWindow::windowVisible() const
{
    return m_windowVisible;
}

bool MenuWindow::editMode() const
{
    return m_editMode;
}

void MenuWindow::setEditMode(bool mode)
{
    if (mode == m_editMode) {
            return;
        }

    m_editMode = mode;
    Q_EMIT editModeChanged();
}

bool MenuWindow::isFullScreen() const
{
    return m_isFullScreen;
}

void MenuWindow::initMenuState()
{
    connect(FullScreenStateHelper::instance(), &FullScreenStateHelper::fullScreenStateChanged, this, [this] (bool isFullScreen){
        setFullScreen(isFullScreen);
    });
    setFullScreen(FullScreenStateHelper::instance()->getFullScreenState());
}

void MenuWindow::init()
{
    initPanelSetting();

    connect(MenuSetting::instance(), &MenuSetting::changed, this, [this] (const QString& key) {
        if (key == MENU_WIDTH || key == MENU_HEIGHT) {
            updateGeometry();
        }
    });
    m_screen = QGuiApplication::primaryScreen();
    setScreen(m_screen);
    connect(m_screen, &QScreen::geometryChanged, this, &MenuWindow::updateGeometry, Qt::UniqueConnection);

    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, &MenuWindow::updateGeometry, Qt::UniqueConnection);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [&](QScreen *qScreen) {
        if(qScreen == screen()) {
            activeMenuWindow(false);
        }
    }, Qt::UniqueConnection);

    // 访问窗口api
    rootContext()->setContextProperty("mainWindow", this);

    rootContext()->setContextProperty("isLiteMode", GlobalSetting::instance()->get(GlobalSetting::IsLiteMode));
    connect(GlobalSetting::instance(), &GlobalSetting::styleChanged, this , [this] (const GlobalSetting::Key& key) {
        if (key == GlobalSetting::EffectEnabled) {
            Q_EMIT effectEnabledChanged();
        } else if (key == GlobalSetting::Transparency) {
            Q_EMIT transparencyChanged();
        } else if (key == GlobalSetting::IsLiteMode) {
            rootContext()->setContextProperty("isLiteMode", GlobalSetting::instance()->get(key));
        }
    });

    setTitle(tr("ukui-menu"));
    setResizeMode(SizeRootObjectToView);
    setColor("transparent");

    initWindowProxy();

    updateGeometry();

    ContextMenuManager::instance()->setMainWindow(this);
}

void MenuWindow::initWindowProxy()
{
    m_windowProxy = new UkuiQuick::WindowProxy(this);
}

bool MenuWindow::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        ContextMenuManager::instance()->closeMenu();
    }

    return QQuickView::event(event);
}

void MenuWindow::initPanelSetting()
{
    // GCOVR_EXCL_START
    if (!MenuSetting::instance()->get(FOLLOW_UKUI_PANEL).toBool()) {
        return;
    }

    const QByteArray id(UKUI_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        m_setting = new QGSettings(id, QByteArray(), this);

        QStringList keys = m_setting->keys();
        if (keys.contains(UKUI_PANEL_POSITION_KEY)) {
            m_panelPos = m_setting->get(UKUI_PANEL_POSITION_KEY).toInt();
        }

        if (keys.contains(UKUI_PANEL_SIZE_KEY)) {
            m_panelSize = m_setting->get(UKUI_PANEL_SIZE_KEY).toInt();
        }

        if (keys.contains(UKUI_PANEL_TYPE_KEY)) {
            m_panelType = m_setting->get(UKUI_PANEL_TYPE_KEY).toInt();
        } else {
            m_panelType = 0;
        }

        if (keys.contains(UKUI_DATA_ISLAND_POSITION_KEY)) {
            m_dataIslandPosition = m_setting->get(UKUI_DATA_ISLAND_POSITION_KEY).toInt();
        }

        if (keys.contains(UKUI_TOPBAR_SIZE_KEY)) {
            m_topbarSize = m_setting->get(UKUI_TOPBAR_SIZE_KEY).toInt();
        }

        connect(m_setting, &QGSettings::changed, this, [this] (const QString& key) {
            if (key == UKUI_PANEL_POSITION_KEY) {
                m_panelPos = m_setting->get(UKUI_PANEL_POSITION_KEY).toInt();
                updateGeometry();
            } else if (key == UKUI_PANEL_SIZE_KEY) {
                m_panelSize = m_setting->get(key).toInt();
                updateGeometry();
            } else if (key == UKUI_PANEL_TYPE_KEY) {
                m_panelType = m_setting->get(UKUI_PANEL_TYPE_KEY).toInt();
                updateGeometry();
            } else if (key == UKUI_DATA_ISLAND_POSITION_KEY) {
                m_dataIslandPosition = m_setting->get(UKUI_DATA_ISLAND_POSITION_KEY).toInt();
                updateGeometry();
            } else if (key == UKUI_TOPBAR_SIZE_KEY) {
                m_topbarSize = m_setting->get(UKUI_TOPBAR_SIZE_KEY).toInt();
                updateGeometry();
            } else if (key == UKUI_PANEL_LENGTH_KEY) {
                updateGeometry();
            }
        });
    }
    // GCOVR_EXCL_STOP
}

void MenuWindow::updateCurrentScreenGeometry()
{
    if (!m_screen || m_screen->geometry().isEmpty()) {
        return;
    }

    // qt的可用区域有问题，暂时使用任务栏尺寸计算
    QRect fullRect = m_screen->geometry();
    int width = MenuSetting::instance()->get(MENU_WIDTH).toInt();
    int height = MenuSetting::instance()->get(MENU_HEIGHT).toInt();
    int margin = MenuSetting::instance()->get(MENU_MARGIN).toInt();
    UkuiQuick::WindowProxy::SlideFromEdge slideFromEdge = UkuiQuick::WindowProxy::NoEdge;
    bool isMirrored = qGuiApp->layoutDirection() == Qt::LayoutDirection::RightToLeft;

    if (m_panelType == 0) {
        //经典任务栏
        switch (m_panelPos) {
            case 0: {
                fullRect.adjust(0, 0, 0, -m_panelSize);
                slideFromEdge = UkuiQuick::WindowProxy::BottomEdge;
                break;
            }
            case 1: {
                fullRect.adjust(0, m_panelSize, 0, 0);
                slideFromEdge = UkuiQuick::WindowProxy::TopEdge;
                break;
            }
            case 2: {
                fullRect.adjust(m_panelSize, 0, 0, 0);
                slideFromEdge = UkuiQuick::WindowProxy::LeftEdge;
                break;
            }
            case 3: {
                fullRect.adjust(0, 0, -m_panelSize, 0);
                slideFromEdge = UkuiQuick::WindowProxy::RightEdge;
                break;
            }
            default: {
                fullRect.adjust(0, 0, 0, -m_panelSize);
                slideFromEdge = UkuiQuick::WindowProxy::BottomEdge;
                break;
            }
        }
    } else if (m_panelType == 1) {
        //三岛任务栏
        if (m_dataIslandPosition == 0) {
            fullRect.adjust(0, 0, 0, -m_panelSize);
            slideFromEdge = UkuiQuick::WindowProxy::BottomEdge;
        } else if (m_dataIslandPosition == 1) {
            fullRect.adjust(0, m_topbarSize, 0, 0);
            slideFromEdge = UkuiQuick::WindowProxy::TopEdge;
        }
    }

    QSize normalSize(qMin(fullRect.width() - margin * 2, width),
                           qMin(fullRect.height() - margin * 2, height));
    QRect normalMaskRect(QPoint(0, 0), normalSize);

    if (m_panelType == 0) {
        int x = margin, y = margin;
        switch (m_panelPos) {
            case 1: //Top
                if (isMirrored) x = fullRect.width() - margin - normalSize.width();
                break;
            case 2: //Left
                break;
            case 3: //Right
                x = fullRect.width() - normalSize.width() - margin;
                break;
            case 0: //Bottom
            default:
                y = fullRect.height() - normalSize.height() - margin;
                if (isMirrored) x = fullRect.width() - margin - normalSize.width();
                break;
        }
        normalMaskRect.moveTo(x, y);
    } else if (m_panelType == 1) {
        int x = margin, y = margin;
        if (m_dataIslandPosition == 0) {
            y = fullRect.height() - normalSize.height() - margin;
            int panelLen = PanelGSettings::instance()->getPanelLength(m_screen->name());
            if (panelLen > 0 && panelLen <= fullRect.width()) {
                x = isMirrored ? (fullRect.width() - (fullRect.width() - panelLen) / 2 - normalSize.width())
                               : ((fullRect.width() - panelLen) / 2);
            } else if (isMirrored) {
                x = fullRect.width() - margin - normalSize.width();
            }
        } else if (m_dataIslandPosition == 1) {
            if (isMirrored) x = fullRect.width() - margin - normalSize.width();
        }
        normalMaskRect.moveTo(x, y);
    }

    m_normalGeometry = QRect(normalMaskRect.topLeft() + fullRect.topLeft(), normalMaskRect.size());
    m_normalMaskGeometry = normalMaskRect;
    m_fullScreenGeometry = fullRect;

    m_windowProxy->slideWindow(slideFromEdge, 0);

    Q_EMIT panelPosChanged();
    Q_EMIT normalRectChanged();
}

// GCOVR_EXCL_START
//======MenuWindowWayland======//
MenuWindowWayland::MenuWindowWayland(QWindow *parent) : MenuWindow(parent)
{
    MenuWindow::init();
}

MenuWindowWayland::MenuWindowWayland(QQmlEngine *engine, QWindow *parent)
    : MenuWindow(engine, parent)
{
    MenuWindow::init();

    //规避wlcom逻辑缺陷，监听windowState变化之后再切换ui
    connect(this, &MenuWindowWayland::windowStateChanged, this, [this] (Qt::WindowState state) {
        if (state == Qt::WindowMaximized || state == Qt::WindowNoState) {
            bool isFullScreen = (state == Qt::WindowMaximized);
            if (m_isFullScreen == isFullScreen) return;
            m_isFullScreen = isFullScreen;
            Q_EMIT fullScreenChanged();
        }
    });
    MenuWindow::initMenuState();
}

void MenuWindowWayland::setFullScreen(bool isFullScreen)
{
    if (m_isFullScreen == isFullScreen) {
        return;
    }

    FullScreenStateHelper::instance()->setFullScreenState(isFullScreen);
    setMenuState(isFullScreen);
}

void MenuWindowWayland::activeMenuWindow(bool active)
{
    if (active == windowVisible()) {
        return;
    }
    m_windowVisible = active;

    if (active) {
        if (SecurityFunctionControl::instance()->disable()) return;
        if (m_screen != UkuiQuick::WindowProxy::currentScreen()) {
            if (m_screen) {
                m_screen->disconnect(this);
            }
            m_screen = UkuiQuick::WindowProxy::currentScreen();
            this->setScreen(m_screen);
            connect(m_screen, &QScreen::geometryChanged, this, &MenuWindowWayland::updateGeometry, Qt::UniqueConnection);
        }
        updateGeometry();
        setMenuState(m_isFullScreen);
    }

    setVisible(active);

    Q_EMIT windowVisibleChanged();
}

void MenuWindowWayland::initWindowProxy()
{
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::SystemWindow);
    m_windowProxy->setWindowStates(UkuiQuick::WindowState::Maximizable);
}

void MenuWindowWayland::updateGeometry()
{
    updateCurrentScreenGeometry();
    m_windowProxy->setBlurRegion(true);
    if (!m_isFullScreen) {
        m_windowProxy->setGeometry(m_normalGeometry);
    } else {
        m_windowProxy->setGeometry(m_fullScreenGeometry);
    }
}

bool MenuWindowWayland::event(QEvent *event)
{
    return MenuWindow::event(event);
}

void MenuWindowWayland::setMenuState(bool isFullScreen)
{
    if (isFullScreen) {
        m_windowProxy->setBlurType(UkuiQuick::WindowProxy::BlurWallpaper);
        setWindowStates(Qt::WindowMaximized);
    } else {
        m_windowProxy->setBlurType(UkuiQuick::WindowProxy::BlurRealtime);
        m_windowProxy->setRestoreGeometry(QRect());
        m_windowProxy->setRestoreGeometry(m_normalGeometry);
        setWindowStates(Qt::WindowNoState);
    }
}

//======MenuWindowXcb======//
UkuiMenu::MenuWindowXcb::MenuWindowXcb(QWindow *parent) : MenuWindow(parent)
{
    MenuWindow::init();
}

UkuiMenu::MenuWindowXcb::MenuWindowXcb(QQmlEngine *engine, QWindow *parent)
    : MenuWindow(engine, parent)
{
    MenuWindow::init();
    MenuWindow::initMenuState();
}

//beforeFullScreenChanged -> (qml)onWidthChanged -> fullScreenChanged
void UkuiMenu::MenuWindowXcb::setFullScreen(bool isFullScreen)
{
    if (m_isFullScreen == isFullScreen) {
        return;
    }

    Q_EMIT beforeFullScreenChanged();

    m_isFullScreen = isFullScreen;
    FullScreenStateHelper::instance()->setFullScreenState(isFullScreen);

    if (m_isFullScreen) {
        updateGeometryOfMask();
        Q_EMIT fullScreenChanged();
    } else {
        Q_EMIT beforeFullScreenExited();
    }

    // 更新contentItem尺寸
    QEvent event(QEvent::Resize);
    QCoreApplication::sendEvent(this, &event);
}

void UkuiMenu::MenuWindowXcb::changeWindowBlurRegion(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(x, y, w, h, radius, radius);
    m_windowProxy->setBlurRegion(true, QRegion(path.toFillPolygon().toPolygon()));
}

void UkuiMenu::MenuWindowXcb::activeMenuWindow(bool active)
{
    if (active == windowVisible()) {
        return;
    }
    m_windowVisible = active;

    if (active) {
        if (SecurityFunctionControl::instance()->disable()) return;
        if (m_screen != UkuiQuick::WindowProxy::currentScreen()) {
            if (m_screen) {
                m_screen->disconnect(this);
            }
            m_screen = UkuiQuick::WindowProxy::currentScreen();
            this->setScreen(m_screen);
            connect(m_screen, &QScreen::geometryChanged, this, &MenuWindowXcb::updateGeometry, Qt::UniqueConnection);
            updateGeometry();
        }
    }

    setVisible(active);
    Q_EMIT windowVisibleChanged();
}

void MenuWindowXcb::finalizeExitFullScreen()
{
    updateGeometryOfMask();
    Q_EMIT fullScreenChanged();
}

void MenuWindowXcb::initWindowProxy()
{
    setFlags(flags() | Qt::Window | Qt::FramelessWindowHint);
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::SystemWindow);
}

void UkuiMenu::MenuWindowXcb::updateGeometry()
{
    updateCurrentScreenGeometry();

    if (m_fullScreenGeometry.isEmpty()) return;
    setMinimumSize(m_fullScreenGeometry.size());
    setMaximumSize(m_fullScreenGeometry.size());
    setGeometry(m_fullScreenGeometry);
    m_windowProxy->setPosition(m_fullScreenGeometry.topLeft());

    updateGeometryOfMask();
    changeWindowBlurRegion( m_maskGeometry.x(), m_maskGeometry.y(), m_maskGeometry.width(), m_maskGeometry.height(), 16);
}

bool MenuWindowXcb::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        requestActivate();
    }

    return MenuWindow::event(event);
}

void UkuiMenu::MenuWindowXcb::updateGeometryOfMask()
{
    if (m_isFullScreen) {
        m_maskGeometry = QRect(0, 0, m_fullScreenGeometry.width(), m_fullScreenGeometry.height());
    } else {
        m_maskGeometry = m_normalMaskGeometry;
    }

    setMask(m_maskGeometry);
}
// GCOVR_EXCL_STOP

} // UkuiMenu
