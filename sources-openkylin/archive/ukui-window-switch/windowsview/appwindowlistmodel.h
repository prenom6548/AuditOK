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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */
#ifndef APPWINDOWLISTMODEL_H
#define APPWINDOWLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>

class WindowsManager;
class AppWindowListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        Id = Qt::UserRole + 1,
        Uuid,
        Caption,
        Geometry,
        Minimized,
        DesktopsId,
        ScreenId,
        Icon
    };
    Q_ENUM(Role)

    explicit AppWindowListModel(QObject *parent = nullptr);
    ~AppWindowListModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void removeAppWindow(QString wid);
    Q_INVOKABLE void moveAppWindowToDesktop(const QString &wid, const QString& srcDesktop, const QString& destDesktop);
    Q_INVOKABLE void activateAppWindow(QString wid);
    Q_INVOKABLE bool hasModalChildWindow(QString wid);

    /* for TabletView
     * qml里无法直接读取QMap<int, QVariant>，需要使用iterator，因此使用QVariant::fromValve(window)
     * TODO: TabletView 加载动画优化
     */
    Q_INVOKABLE QVariantMap getItemData(int row) const;

Q_SIGNALS:
    void appWindowRemoved(QString windowId);

private:
    void connectSignals();
    void loadWindows();
    void addItem(QString wid);
    QStringList getWindows();
    int findWindowIndex(const QString &wid);
    void notifyDataChanged(const QString &windowId, int role);

    WindowsManager *m_windowManager = nullptr;
    QStringList m_windowList;

private Q_SLOTS:
    void onWindowAdd(QString windowId);
    void onWindowRemoved(QString windowId);
    void onWindowActived(QString windowId);
    void onWindowDesktopChanged(QString windowId);
    void onWindowIconChanged(QString windowId);
    void onWindowTitleChanged(QString windowId);
    void onWindowGeometryChanged(QString windowId);
};

#endif // APPWINDOWLISTMODEL_H
