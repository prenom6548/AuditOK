/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#include "QX11Info"
#include "event-track.h"
#include "global-settings.h"
#include "hand-gesture-helper.h"
#include "notification-group-model.h"
#include "notification-model.h"
#include "popup-notification-model.h"
#include "urgency-notification-model.h"
#include "screen-monitor.h"
#include "shared-engine-view.h"
#include "settings.h"
#include "short-cut-manager.h"
#include "shortcut-model.h"
#include "sidebar-window-helper.h"
#include "window-helper.h"

#include <QGuiApplication>

namespace {

bool g_platformX11 = false;
QString g_lastCode;
QString g_lastPage;
QVariantMap g_lastMap;
QPointer<QScreen> g_currentScreen;

}

bool QX11Info::isPlatformX11()
{
    return g_platformX11;
}

__attribute__((weak)) xcb_connection_t *xcbStubConnection()
{
    return nullptr;
}

void QX11Info::setPlatformX11(bool value)
{
    g_platformX11 = value;
}

xcb_connection_t *QX11Info::connection()
{
    return xcbStubConnection();
}

namespace UkuiQuick {

static QQmlEngine *g_engine = nullptr;

QQmlEngine *SharedEngineView::sharedEngine()
{
    if (!g_engine) {
        g_engine = new QQmlEngine;
    }
    return g_engine;
}

SharedEngineView::SharedEngineView(QWindow *parent) : QQuickWindow(parent)
{
    m_context = new QQmlContext(sharedEngine()->rootContext(), this);
}

QQmlEngine *SharedEngineView::engine() const
{
    return sharedEngine();
}

QQmlContext *SharedEngineView::rootContext() const
{
    return m_context;
}

QQuickItem *SharedEngineView::rootObject() const
{
    return m_rootObject;
}

void SharedEngineView::setResizeMode(SharedEngineView::ResizeMode resizeMode)
{
    m_resizeMode = resizeMode;
}

SharedEngineView::ResizeMode SharedEngineView::resizeMode() const
{
    return m_resizeMode;
}

void SharedEngineView::setTestRootObject(QQuickItem *item)
{
    m_rootObject = item;
}

void SharedEngineView::setSource(const QUrl &source) const
{
    m_source = source;
}

WindowProxy::WindowProxy(QWindow *window, Operations operations) : QObject(window), m_window(window)
{
    Q_UNUSED(operations)
}

QScreen *WindowProxy::currentScreen()
{
    return g_currentScreen ? g_currentScreen.data() : QGuiApplication::primaryScreen();
}

void WindowProxy::setCurrentScreen(QScreen *screen)
{
    g_currentScreen = screen;
}

void WindowProxy::setPosition(const QPoint &point)
{
    m_position = point;
}

QPoint WindowProxy::position() const
{
    return m_position;
}

void WindowProxy::setWindowType(WindowType::Type type)
{
    m_windowType = type;
}

WindowType::Type WindowProxy::windowType() const
{
    return m_windowType;
}

void WindowProxy::setWindowStates(WindowState::States states, bool enable)
{
    if (enable) {
        m_states |= states;
    } else {
        m_states &= ~states;
    }
}

bool WindowProxy::windowStateEnabled(WindowState::State state) const
{
    return m_states.testFlag(state);
}

void WindowProxy::setBlurRegion(bool enable, const QRegion &region)
{
    m_blurEnabled = enable;
    m_blurRegion = region;
    ++m_blurCallCount;
}

bool WindowProxy::blurEnabled() const
{
    return m_blurEnabled;
}

QRegion WindowProxy::blurRegion() const
{
    return m_blurRegion;
}

int WindowProxy::blurCallCount() const
{
    return m_blurCallCount;
}

void WindowProxy::setBlurOpacity(double opacity)
{
    m_blurOpacity = opacity;
}

double WindowProxy::blurOpacity() const
{
    return m_blurOpacity;
}

void WindowProxy::slideWindow(WindowProxy::SlideFromEdge edge, int offset)
{
    m_lastSlideEdge = edge;
    m_lastSlideOffset = offset;
}

WindowProxy::SlideFromEdge WindowProxy::lastSlideEdge() const
{
    return m_lastSlideEdge;
}

int WindowProxy::lastSlideOffset() const
{
    return m_lastSlideOffset;
}

Settings *Settings::instance()
{
    static Settings settings;
    return &settings;
}

QString Settings::platformName() const
{
    return m_platformName;
}

void Settings::setPlatformName(const QString &name)
{
    m_platformName = name;
}

void Settings::reset()
{
    m_platformName = "xcb";
}

} // namespace UkuiQuick

namespace Sidebar {

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
}

GlobalSettings *GlobalSettings::globalInstance(QObject *parent)
{
    static GlobalSettings settings(parent);
    return &settings;
}

QVariant GlobalSettings::getValue(const QString &key) const
{
    return m_values.value(key);
}

QStringList GlobalSettings::getKeys() const
{
    return m_values.keys();
}

int GlobalSettings::getPanelLength(const QString &screenName) const
{
    return m_panelLengths.value(screenName, 0);
}

void GlobalSettings::setValue(const QString &key, const QVariant &value)
{
    if (m_values.value(key) == value) {
        return;
    }
    m_values.insert(key, value);
    Q_EMIT valueChanged(key);
}

void GlobalSettings::setPanelLength(const QString &screenName, int length)
{
    m_panelLengths.insert(screenName, length);
    Q_EMIT valueChanged(UKUI_PANEL_LENGTH_KEY);
}

void GlobalSettings::reset()
{
    m_values.clear();
    m_panelLengths.clear();
}

ScreenMonitor::ScreenMonitor(QObject *parent) : QObject(parent)
{
}

ScreenMonitor *ScreenMonitor::getInstance()
{
    static ScreenMonitor monitor;
    return &monitor;
}

bool ScreenMonitor::getScreenLockState() const
{
    return m_screenLockState;
}

QScreen *ScreenMonitor::getPrimaryScreen() const
{
    return m_primaryScreen;
}

QRect ScreenMonitor::getGeometry() const
{
    return m_primaryScreen ? m_primaryScreen->geometry() : QRect();
}

Qt::LayoutDirection ScreenMonitor::getLayoutDirection() const
{
    return m_direction;
}

int ScreenMonitor::getPanelPosition() const
{
    return m_panelPosition;
}

int ScreenMonitor::getPanelSize() const
{
    return m_panelSize;
}

void ScreenMonitor::setScreenLockState(bool state)
{
    if (m_screenLockState == state) {
        return;
    }
    m_screenLockState = state;
    Q_EMIT screenLockStateChanged(state);
}

void ScreenMonitor::setPrimaryScreen(QScreen *screen)
{
    m_primaryScreen = screen;
    Q_EMIT primaryScreenChanged();
    Q_EMIT geometryChanged(getGeometry());
}

void ScreenMonitor::setLayoutDirection(Qt::LayoutDirection direction)
{
    m_direction = direction;
    Q_EMIT layoutDirectionChanged();
}

void ScreenMonitor::setPanelPosition(int position)
{
    m_panelPosition = position;
}

void ScreenMonitor::setPanelSize(int size)
{
    m_panelSize = size;
}

void ScreenMonitor::emitPanelPropertyChanged()
{
    Q_EMIT panelPropertyChanged();
}

void ScreenMonitor::reset()
{
    m_screenLockState = false;
    m_primaryScreen = nullptr;
    m_direction = Qt::LeftToRight;
    m_panelPosition = 0;
    m_panelSize = 0;
}

HandGestureHelper::HandGestureHelper(QObject *parent) : QObject(parent)
{
}

HandGestureHelper *HandGestureHelper::getInstance()
{
    static HandGestureHelper helper;
    return &helper;
}

void HandGestureHelper::reset()
{
}

void HandGestureHelper::callNotificationCenter(int posY)
{
    Q_EMIT notificationCenterCalled(posY);
}

void HandGestureHelper::top2BottomRelease(int posX, int posY)
{
    Q_EMIT top2BottomReleased(posX, posY);
}

void HandGestureHelper::callControlCenter(int posX)
{
    Q_EMIT controlCenterCalled(posX);
}

void HandGestureHelper::right2LeftRelease(int posX, int posY)
{
    Q_EMIT right2LeftReleased(posX, posY);
}

SidebarWindowHelper::SidebarWindowHelper(QObject *parent) : QObject(parent)
{
}

SidebarWindowHelper *SidebarWindowHelper::instance(QObject *parent)
{
    Q_UNUSED(parent)
    static SidebarWindowHelper helper;
    return &helper;
}

QRect SidebarWindowHelper::getWindowGeometry(SidebarWindowType::Type type)
{
    return m_geometries.value(static_cast<int>(type));
}

Qt::LayoutDirection SidebarWindowHelper::getLayoutDirection()
{
    return m_direction;
}

int SidebarWindowHelper::getPanelPos()
{
    return 0;
}

void SidebarWindowHelper::setWindowGeometry(SidebarWindowType::Type type, const QRect &geometry)
{
    m_geometries.insert(static_cast<int>(type), geometry);
    Q_EMIT geometryChanged();
}

void SidebarWindowHelper::setLayoutDirection(Qt::LayoutDirection direction)
{
    m_direction = direction;
    Q_EMIT geometryChanged();
}

void SidebarWindowHelper::reset()
{
    m_geometries.clear();
    m_direction = Qt::LeftToRight;
}

EventTrack::EventTrack(QObject *parent) : QObject(parent)
{
}

EventTrack *EventTrack::instance()
{
    static EventTrack eventTrack;
    return &eventTrack;
}

EventTrack *EventTrack::qmlAttachedProperties(QObject *object)
{
    return new EventTrack(object);
}

void EventTrack::reset()
{
    g_lastCode.clear();
    g_lastPage.clear();
    g_lastMap.clear();
}

QString EventTrack::lastCode()
{
    return g_lastCode;
}

QString EventTrack::lastPage()
{
    return g_lastPage;
}

QVariantMap EventTrack::lastMap()
{
    return g_lastMap;
}

void EventTrack::sendSlideEvent(const QString &code, const QString &page, const QVariantMap &map)
{
    g_lastCode = code;
    g_lastPage = page;
    g_lastMap = map;
}

} // namespace Sidebar

namespace UkuiShortcut {

ShortcutManager *ShortcutManager::getInstance()
{
    static ShortcutManager manager;
    return &manager;
}

QList<Shortcut *> ShortcutManager::getShortcuts()
{
    return {};
}

QPair<bool, QString> ShortcutManager::getWidgetInfo(const QString &widgetId)
{
    if (!m_widgetInfo.contains(widgetId)) {
        return {false, QString()};
    }
    return {true, m_widgetInfo.value(widgetId)};
}

void ShortcutManager::setWidgetInfo(const QString &widgetId, const QString &returnButtonName)
{
    m_widgetInfo.insert(widgetId, returnButtonName);
}

void ShortcutManager::reset()
{
    m_widgetInfo.clear();
}

ShortcutModel::ShortcutModel(QObject *parent) : QAbstractListModel(parent)
{
}

int ShortcutModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 0;
}

QVariant ShortcutModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return {};
}

} // namespace UkuiShortcut

namespace UkuiNotification {

NotificationGroupModel::NotificationGroupModel(QObject *parent) : QAbstractProxyModel(parent)
{
}

void NotificationGroupModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    beginResetModel();
    if (this->sourceModel()) {
        this->sourceModel()->disconnect(this);
    }
    QAbstractProxyModel::setSourceModel(sourceModel);
    endResetModel();

    if (!sourceModel) {
        return;
    }

    connect(sourceModel, &QAbstractItemModel::rowsInserted, this,
            [this] (const QModelIndex &, int first, int last) {
        beginInsertRows(QModelIndex(), first, last);
        endInsertRows();
    });
    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this,
            [this] (const QModelIndex &, int first, int last) {
        beginRemoveRows(QModelIndex(), first, last);
        endRemoveRows();
    });
    connect(sourceModel, &QAbstractItemModel::modelReset, this, [this] {
        beginResetModel();
        endResetModel();
    });
}

QModelIndex NotificationGroupModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return {};
    }
    return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), QModelIndex());
}

QModelIndex NotificationGroupModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return {};
    }
    return index(sourceIndex.row(), sourceIndex.column(), QModelIndex());
}

QModelIndex NotificationGroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || column != 0 || row >= rowCount(QModelIndex())) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex NotificationGroupModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return {};
}

int NotificationGroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !sourceModel()) {
        return 0;
    }

    if (qobject_cast<QAbstractProxyModel *>(sourceModel())) {
        return sourceModel()->rowCount();
    }

    int count = 0;
    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        if (sourceModel()->index(i, 0).data(NotificationItem::IsStored).toBool()) {
            ++count;
        }
    }
    return count;
}

int NotificationGroupModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant NotificationGroupModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return {};
    }

    if (qobject_cast<QAbstractProxyModel *>(sourceModel())) {
        QModelIndex sourceIndex = sourceModel()->index(proxyIndex.row(), 0);
        if (role == NotificationItem::GroupName) {
            return sourceIndex.data(NotificationItem::AppName);
        }
        if (role == NotificationItem::GroupCount) {
            return 1;
        }
        return sourceIndex.data(role);
    }

    int storedRow = -1;
    for (int i = 0, current = 0; i < sourceModel()->rowCount(); ++i) {
        QModelIndex index = sourceModel()->index(i, 0);
        if (!index.data(NotificationItem::IsStored).toBool()) {
            continue;
        }
        if (current == proxyIndex.row()) {
            storedRow = i;
            break;
        }
        ++current;
    }

    if (storedRow < 0) {
        return {};
    }

    QModelIndex sourceIndex = sourceModel()->index(storedRow, 0);
    if (role == NotificationItem::GroupName) {
        return sourceIndex.data(NotificationItem::AppName);
    }
    if (role == NotificationItem::GroupCount) {
        return 1;
    }
    return sourceIndex.data(role);
}

PopupNotificationModel::PopupNotificationModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool PopupNotificationModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return !index.data(NotificationItem::IsStored).toBool()
           && !Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool()
           && (!Sidebar::ScreenMonitor::getInstance()->getScreenLockState()
               || index.data(NotificationItem::ShowOnLockScreen).toBool());
}

NotificationModel::NotificationModel(QObject *parent) : QAbstractListModel(parent)
{
}

NotificationModel *NotificationModel::instance()
{
    static NotificationModel model;
    return &model;
}

int NotificationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant NotificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    const auto &item = m_items.at(index.row());
    switch (role) {
    case NotificationItem::Id:
        return item.id;
    case NotificationItem::AppName:
        return item.appName;
    case NotificationItem::NoFold:
        return item.noFold;
    case NotificationItem::IsStored:
        return item.isStored;
    case NotificationItem::ShowOnLockScreen:
        return item.showOnLockScreen;
    case NotificationItem::Summary:
        return item.summary;
    case NotificationItem::Body:
        return item.body;
    case NotificationItem::Urgency:
        return item.urgency;
    case NotificationItem::CreateTime:
        return item.createTime;
    default:
        return {};
    }
}

QHash<int, QByteArray> NotificationModel::roleNames() const
{
    return {
        {NotificationItem::Id, "id"},
        {NotificationItem::AppName, "appName"},
        {NotificationItem::NoFold, "noFold"},
        {NotificationItem::IsStored, "isStored"},
        {NotificationItem::ShowOnLockScreen, "showOnLockScreen"},
        {NotificationItem::Summary, "summary"},
        {NotificationItem::Body, "body"},
        {NotificationItem::Urgency, "urgency"},
        {NotificationItem::CreateTime, "createTime"}
    };
}

void NotificationModel::removeNotification(uint id)
{
    m_removedIds.append(id);
}

void NotificationModel::setItems(const QList<TestNotificationData> &items)
{
    if (!m_items.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
        m_items.clear();
        endRemoveRows();
    }

    if (!items.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, items.size() - 1);
        m_items = items;
        endInsertRows();
    }
    Q_EMIT unreadMessageCountChanged();
}

void NotificationModel::reset()
{
    if (!m_items.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
        m_items.clear();
        endRemoveRows();
    }
    m_removedIds.clear();
    Q_EMIT unreadMessageCountChanged();
}

QList<uint> NotificationModel::removedIds() const
{
    return m_removedIds;
}

int NotificationModel::getUnreadMessageCount() const
{
    return m_items.size();
}

void NotificationModel::storePopupNotification(bool store)
{
    Q_UNUSED(store)
}

} // namespace UkuiNotification

UrgencyNotificationModel::UrgencyNotificationModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void UrgencyNotificationModel::closeNotification(uint id)
{
    UkuiNotification::NotificationModel::instance()->removeNotification(id);
}

void UrgencyNotificationModel::execAction(uint id, QString action)
{
    Q_UNUSED(id)
    Q_UNUSED(action)
}

bool UrgencyNotificationModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return false;
    }
    const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceIndex.data(UkuiNotification::NotificationItem::Urgency).toInt() == 2;
}

bool UrgencyNotificationModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    return sourceLeft.data(UkuiNotification::NotificationItem::CreateTime).toDateTime()
           < sourceRight.data(UkuiNotification::NotificationItem::CreateTime).toDateTime();
}
