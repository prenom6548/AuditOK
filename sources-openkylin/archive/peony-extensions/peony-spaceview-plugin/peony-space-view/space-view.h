/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#ifndef SPACEVIEW_H
#define SPACEVIEW_H
#include <QWidget>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
class Spaceview: public QWidget{
    Q_OBJECT
public:
    explicit Spaceview(QWidget *parent = nullptr,QString uri="",QString Spacename="");
    void setDirectoryUri(QString uri);
public Q_SLOTS:
    void handleLinkActivated(const QString &link);
    void userAddFilesToSpaceBySpaceplugin(const QStringList &filePath,const QString &spaceName,bool isUserAdd);
    void editSpaceConditionBySpaceplugin(const QString &uri);
private:
    QString m_uri;
    QString m_spaceName;
    QDBusInterface* m_signalTransInterface;
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};
#endif //
