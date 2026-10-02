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

#include "notification-window.h"
#include "notification-model.h"
#include "sidebar-window-helper.h"
#include "hand-gesture-helper.h"
#include "global-settings.h"
#include "event-track.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QScreen>
#include <QGuiApplication>
#include <QQuickItem>
#include <QSortFilterProxyModel>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif

#include <mutex>

static std::once_flag flag;

using namespace UkuiNotification;

class IsStoredFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit IsStoredFilterModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

IsStoredFilterModel::IsStoredFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

bool IsStoredFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    return idx.data(UkuiNotification::NotificationItem::IsStored).toBool();
}

NotificationGroupModel *NotificationCenterWindow::globalGroupModel()
{
    static NotificationGroupModel groupModel;
    std::call_once(flag, [&] {
        auto filterModel = new IsStoredFilterModel(&groupModel);
        filterModel->setSourceModel(NotificationModel::instance());

        groupModel.setSourceModel(filterModel);
    });

    return &groupModel;
}

NotificationCenterWindow::NotificationCenterWindow(QWindow *parent) : SharedEngineView(parent)
{
    setColor(Qt::transparent);
    setResizeMode(SharedEngineView::SizeRootObjectToView);
    setFlags(Qt::FramelessWindowHint);

    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::SystemWindow);

    rootContext()->setContextProperty("groupModel", NotificationCenterWindow::globalGroupModel());
    rootContext()->setContextProperty("sourceModel", NotificationModel::instance());
    rootContext()->setContextProperty("notificationWindow", this);

    onPrimaryScreenChanged(qGuiApp->primaryScreen());
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &NotificationCenterWindow::onPrimaryScreenChanged);

    m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE) {
            m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(key).toBool();
            if (!m_isTabletMode) {
                callNotificationCenterEnd(0, 0);
            }
        }
    });

    connect(Sidebar::HandGestureHelper::getInstance(), &Sidebar::HandGestureHelper::notificationCenterCalled,
            this, &NotificationCenterWindow::callNotificationCenter);

    connect(Sidebar::HandGestureHelper::getInstance(), &Sidebar::HandGestureHelper::top2BottomReleased,
            this, &NotificationCenterWindow::callNotificationCenterEnd);

}

void NotificationCenterWindow::init()
{
    engine()->addImportPath("qrc:/qml");
    setSource(QUrl("qrc:/qml/NotificationView.qml"));
}

bool NotificationCenterWindow::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Expose: {
            if (QX11Info::isPlatformX11()) {
                requestActivate();
            }
            break;
        }
        default:
            break;
    }

    return SharedEngineView::event(event);
}

void NotificationCenterWindow::updateGeometry()
{
    if (!m_screen || m_screen->geometry().isEmpty()) {
        return;
    }

    setGeometry(m_screen->geometry());
    m_windowProxy->setPosition(m_screen->geometry().topLeft());
}

void NotificationCenterWindow::onPrimaryScreenChanged(QScreen *newScreen)
{
    if (!newScreen) {
        return;
    }

    if (m_screen) {
        m_screen->disconnect(this);
    }

    m_screen = newScreen;
    setScreen(m_screen);
    updateGeometry();

    connect(m_screen, &QScreen::geometryChanged, this, &NotificationCenterWindow::updateGeometry);
}

void NotificationCenterWindow::callNotificationCenter(int posY)
{
    if (!rootObject() || !m_isTabletMode || m_contentVisible) {
        return;
    }

    if (!isVisible()) {
        QMetaObject::invokeMethod(rootObject(), "updateContentY", Qt::DirectConnection, Q_ARG(QVariant, -height()));
        setVisible(true);
    }

    if (posY > height()) {
        posY = height();

    } else if (posY < 0) {
        posY = 0;
    }

    posY -= height();

    QMetaObject::invokeMethod(rootObject(), "updateContentY", Qt::DirectConnection, Q_ARG(QVariant, posY));
}

void NotificationCenterWindow::callNotificationCenterEnd(int posX, int posY)
{
    Q_UNUSED(posX)
    if (!isVisible()) {
        return;
    }

    bool isShow = posY > (height() * 0.25);
    QMetaObject::invokeMethod(rootObject(), "startAnimation", Qt::DirectConnection, Q_ARG(QVariant, isShow));
    QVariantMap map;
    map.insert("type", "slide");
    Sidebar::EventTrack().sendSlideEvent("open_notification_center", "notificationCenter", map);
}

bool NotificationCenterWindow::contentVisible() const
{
    return m_contentVisible;
}

void NotificationCenterWindow::setContentVisible(bool contentVisible)
{
    m_contentVisible = contentVisible;
    Q_EMIT contentVisibleChanged();
}

void NotificationCenterWindow::activeNotificationCenter(bool active)
{
    if (active) {
        callNotificationCenter(0);
        callNotificationCenterEnd(0, height());

    } else {
        callNotificationCenterEnd(0, 0);
    }
}

#include "notification-window.moc"
