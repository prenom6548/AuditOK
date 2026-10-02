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
#include "urgency-notification-window.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QApplication>
#include <QScreen>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif

#include <KWindowEffects>
#include <KWindowSystem>
#include <kysdk/applications/windowmanager/windowmanager.h>
#include "color-helper.h"
#include "screen-monitor.h"

using namespace UkuiNotification;

const QByteArray PANEL_GSETTINGS = "org.ukui.panel.settings";
#define PANEL_TOP 1
#define PANEL_LEFT 2
#define PANEL_RIGHT 3

UrgencyNotificationWindow::UrgencyNotificationWindow(QWindow *parent)
    :SharedEngineView(parent)
{
    setResizeMode(SharedEngineView::SizeRootObjectToView);
    setWindowProperties();
    m_urgencyModel = new UrgencyNotificationModel(this);
    m_urgencyModel->setSourceModel(NotificationModel::instance());
    m_urgencyModel->sort(0, Qt::DescendingOrder);

    engine()->addImportPath("qrc:/qml");
    rootContext()->setContextProperty("urgencyNotificationModel", m_urgencyModel);
    rootContext()->setContextProperty("urgencyNotificationWindow", this);

    setSource(QUrl("qrc:/qml/UrgencyNotificationView.qml"));

    connect(m_urgencyModel, &UrgencyNotificationModel::rowsInserted, this, &UrgencyNotificationWindow::showUrgencyNotificationWindow);
    connect(m_urgencyModel, &UrgencyNotificationModel::rowsRemoved, this, &UrgencyNotificationWindow::closeUrgencyNotificationWindow);
}

void UrgencyNotificationWindow::setWindowProperties()
{
    setColor(Qt::transparent);  //背景透明
    setFlags(Qt::FramelessWindowHint);

    //跳过任务栏属性
    kdk::WindowManager::setSkipTaskBar(this, true);
    kdk::WindowManager::setSkipSwitcher(this, true);

    KWindowSystem::setType(this->winId(), NET::Notification);
    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion());  //背景模糊

    setWindowPosition();
}

void UrgencyNotificationWindow::setWindowPosition()
{
    QRect rect = QApplication::primaryScreen()->geometry();

    if(QGSettings::isSchemaInstalled(PANEL_GSETTINGS)) {
        QGSettings setting(PANEL_GSETTINGS);
        if (setting.keys().contains(QString("panelposition")) && setting.keys().contains(QString("panelsize"))) {

            int panelPosition = setting.get("panelposition").toInt();
            int panelHeight = setting.get("panelsize").toInt();

            switch (panelPosition) {
            case PANEL_TOP:
                rect = QRect(rect.x(), rect.y() + panelHeight, rect.width(), rect.height() - panelHeight);
                break;
            case PANEL_LEFT:
                rect = QRect(rect.x() + panelHeight, rect.y(), rect.width() - panelHeight, rect.height());
                break;
            case PANEL_RIGHT:
                rect = QRect(rect.x(), rect.y(), rect.width() - panelHeight, rect.height());
                break;
            default:
                rect = QRect(rect.x(), rect.y(), rect.width(), rect.height() - panelHeight);
                break;
            }
        }
    }

    if (QX11Info::isPlatformX11()) {
        this->setGeometry(rect);

    } else {
        kdk::WindowManager::setGeometry(this, rect);
    }
}

bool UrgencyNotificationWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Expose:
        if (isExposed()) {
            setWindowProperties();
        }
        break;
    default:
        break;
    }

//    qDebug() << QApplication::primaryScreen()->geometry() << geometry() << __LINE__;
    return QQuickWindow::event(event);
}

void UrgencyNotificationWindow::showUrgencyNotificationWindow()
{
    this->show();
}

void UrgencyNotificationWindow::closeUrgencyNotificationWindow()
{
    if (m_urgencyModel->rowCount() == 0) {
        this->hide();
    }
}
