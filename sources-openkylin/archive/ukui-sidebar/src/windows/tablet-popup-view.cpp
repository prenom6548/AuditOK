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

#include "tablet-popup-view.h"
#include "notification-model.h"
#include "global-settings.h"
#include "date-time-utils.h"
#include "color-helper.h"
#include "notification-group-model.h"
#include "group-cache-proxy-model.h"

#include <QScreen>
#include <QQmlEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>
#include <QGuiApplication>
#include <QRegion>
#include <QPainterPath>

#include <KWindowSystem>
#include <KWindowEffects>
#include <window-helper.h>

class NotificationFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit NotificationFilterModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_isTabletMode = true;
};

NotificationFilterModel::NotificationFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE) {
            m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(key).toBool();
            invalidateFilter();
        }
    });
}

void NotificationFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (QSortFilterProxyModel::sourceModel()) {
        QSortFilterProxyModel::sourceModel()->disconnect(this);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &NotificationFilterModel::invalidateFilter);
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &NotificationFilterModel::invalidateFilter);
    }
}

bool NotificationFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!sourceModel() || !m_isTabletMode) {
        return false;
    }

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    bool isStored = idx.data(UkuiNotification::NotificationItem::IsStored).toBool();
//    bool isExpired = idx.data(UkuiNotification::NotificationItem::IsExpired).toBool();

    return !isStored;
}

class TabletNotificationModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TabletNotificationModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QSet<uint> m_readMsg;
};

TabletNotificationModel::TabletNotificationModel(QObject *parent) : QSortFilterProxyModel(parent) {}

void TabletNotificationModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (QSortFilterProxyModel::sourceModel()) {
        QSortFilterProxyModel::sourceModel()->disconnect(this);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [this] (const QModelIndex &parent, int first, int last) {
            invalidateFilter();
            for (int i = first; i <= last; ++i) {
                m_readMsg.insert(QSortFilterProxyModel::sourceModel()->index(i, 0, parent).data(UkuiNotification::NotificationItem::Id).toUInt());
            }
        });
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [this] (const QModelIndex &parent, int first, int last) {
            invalidateFilter();
            for (int i = first; i <= last; ++i) {
                m_readMsg.remove(QSortFilterProxyModel::sourceModel()->index(i, 0, parent).data(UkuiNotification::NotificationItem::Id).toUInt());
            }
        });
    }
}

bool TabletNotificationModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    uint id = sourceModel()->index(source_row, 0, source_parent).data(UkuiNotification::NotificationItem::Id).toUInt();
    return !m_readMsg.contains(id) && (source_row == (sourceModel()->rowCount() - 1));
}

// =====TabletPopupView==
UkuiNotification::TabletPopupView::TabletPopupView(QWindow *parent) : SharedEngineView(parent)
{
    setColor(Qt::transparent);
    setResizeMode(SharedEngineView::SizeRootObjectToView);
    setFlags(Qt::FramelessWindowHint);

    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::Notification);

    new UkuiQuick::WindowProxy(this, UkuiQuick::WindowProxy::SkipTaskBar | UkuiQuick::WindowProxy::SkipSwitcher);

    qmlRegisterType<GroupCacheProxyModel>("org.ukui.notification.model", 1, 0, "GroupCacheProxyModel");

    onPrimaryScreenChanged(qGuiApp->primaryScreen());
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &TabletPopupView::onPrimaryScreenChanged);

    auto filterModel = new NotificationFilterModel(this);
    filterModel->setSourceModel(NotificationModel::instance());

//    auto model = new TabletNotificationModel(this);
//    model->setSourceModel(filterModel);

    auto group = new NotificationGroupModel(this);
    group->setSourceModel(filterModel);

    setVisible(group->rowCount(QModelIndex()) > 0);
    connect(group, &TabletNotificationModel::rowsInserted, this, [this, group] {
        setVisible(group->rowCount(QModelIndex()) > 0);
    });
//    connect(group, &TabletNotificationModel::rowsRemoved, this, [this, group] {
//        qDebug() << "==rowsRemoved==" << group << group->rowCount(QModelIndex());
//        setVisible(group->rowCount(QModelIndex()) > 0);
//    });

    rootContext()->setContextProperty("tabletPopupView", this);
//    rootContext()->setContextProperty("notificationModel", model);
    rootContext()->setContextProperty("groupModel", group);
    rootContext()->setContextProperty("sourceModel", NotificationModel::instance());

    init();
}

void UkuiNotification::TabletPopupView::init()
{
    engine()->addImportPath("qrc:/qml");
    setSource(QUrl("qrc:/qml/TabletPopupView.qml"));
}

void UkuiNotification::TabletPopupView::onPrimaryScreenChanged(QScreen *newScreen)
{
    if (!newScreen) {
        return;
    }

    if (m_screen) {
        m_screen->disconnect(this);
    }

    m_screen = newScreen;
    setScreen(m_screen);
    connect(m_screen, &QScreen::geometryChanged, this, &TabletPopupView::onScreenGeometryChanged);
    onScreenGeometryChanged();
}

void UkuiNotification::TabletPopupView::onScreenGeometryChanged()
{
    int screenWidth = m_screen->geometry().width();
    m_windowWidth = static_cast<int>(screenWidth * 0.45);

    m_topLeft.setX(m_screen->geometry().x() + ((screenWidth - m_windowWidth) / 2));
    m_topLeft.setY(m_screen->geometry().y() + m_windowMargin);

    Q_EMIT windowWidthChanged();
    updateGeometry();
}

void UkuiNotification::TabletPopupView::updateGeometry()
{
    QRect geometry = QRect(m_topLeft, QSize(m_windowWidth, height()));
    if (geometry.isEmpty()) return;
    setGeometry(geometry);
    m_windowProxy->setPosition(m_topLeft);
}

bool UkuiNotification::TabletPopupView::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Show:
        case QEvent::Expose: {
            break;
        }
        default:
            break;
    }

    return SharedEngineView::event(event);
}

int UkuiNotification::TabletPopupView::windowWidth() const
{
    return m_windowWidth;
}

void UkuiNotification::TabletPopupView::enableWindowBlur(int radius, bool enable)
{
    if (m_windowProxy == nullptr) return;

    QPainterPath path;
    path.addRoundedRect(0, 0, width(), height(), radius, radius);

    m_windowProxy->setBlurRegion(enable, QRegion(path.toFillPolygon().toPolygon()));
}

int UkuiNotification::TabletPopupView::windowMaxHeight() const
{
    if (m_screen) {
        return m_screen->geometry().height() - m_windowMargin*2;
    }

    return 1;
}

void UkuiNotification::TabletPopupView::updateHeight(int height)
{
    if (height < 1) {
        height = 1;
    }
    int h = height > windowMaxHeight() ? windowMaxHeight() : height;
    if (h == TabletPopupView::height()) {
        return;
    }
    setHeight(h);
}

bool UkuiNotification::TabletPopupView::enableAnimation() const
{
    return m_enableAnimation;
}

#include "tablet-popup-view.moc"
