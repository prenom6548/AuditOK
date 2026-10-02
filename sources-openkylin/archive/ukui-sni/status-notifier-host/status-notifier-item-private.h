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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUI_SNI_STATUS_NOTIFIER_ITEM_PRIVATE_H
#define UKUI_SNI_STATUS_NOTIFIER_ITEM_PRIVATE_H
#include <QIcon>
#include <QMenu>
#include "dbusmenu-importer.h"
#include "dbus-types.h"
#include "statusnotifieritem_interface.h"

namespace UkuiSni {

class UkuiDBusMenuImporter : public DBusMenuImporter
{
public:
    UkuiDBusMenuImporter(const QString &service, const QString &path, QObject *parent = 0);
protected:
    QIcon iconForName(const QString & iconName) override;
};

class StatusNotifierItemPrivate : public QObject
{
    friend class StatusNotifierItem;
    Q_OBJECT
public:
    explicit StatusNotifierItemPrivate(const QString &service, StatusNotifierItem *q, QObject *parent = nullptr);
    void init();
    void contextMenu(int x, int y);
    void activate(int x, int y);
    void secondaryActivate(int x, int y);
    void scroll(int delta, const QString &direction);
    void provideXdgActivationToken(const QString &token);

private Q_SLOTS:
    void refresh();
    void performRefresh();
    void syncStatus(const QString &status);
    void refreshMenu();
    void refreshCallback(QDBusPendingCallWatcher *call);
    void activateCallback(QDBusPendingCallWatcher *call);

private:
    QIcon loadIcon(const QVariantMap &properties, const QString &iconName, const QString &pixmapKey);
    QPixmap DbusImageStructToPixmap(const DbusImageStruct &image) const;
    QIcon imageVectorToPixmap(const DbusImageVector &vector) const;
    static QString displayFormPid(int pid);

    QTimer m_refreshTimer;
    bool m_refreshing : 1;
    bool m_needsReRefreshing : 1;
    bool m_valid = false;
    QString m_serviceNamePath;
    QString m_serviceName;
    UkuiDBusMenuImporter *m_menuImporter = nullptr;
    int m_pendingMenuToShow = 0;
    org::kde::StatusNotifierItem *m_statusNotifierItemInterface = nullptr;
    QIcon m_attentionIcon;
    QString m_attentionIconName;
    QString m_attentionMovieName;
    QString m_category;
    QIcon m_icon;
    QString m_iconName;
    QString m_iconThemePath;
    QString m_id;
    bool m_itemIsMenu{};
    QIcon m_overlayIcon;
    QString m_overlayIconName;
    QString m_status;
    QString m_title;
    QIcon m_toolTipIcon;
    QString m_toolTipSubTitle;
    QString m_toolTipTitle;
    QString m_windowId;
    QString m_display;
    uint m_pid = 0;
    StatusNotifierItem *q;
};
}
#endif //UKUI_SNI_STATUS_NOTIFIER_ITEM_PRIVATE_H
