/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLEANER_WIDGET_H
#define CLEANER_WIDGET_H

#include "cleaneritem.h"
#include "settingscrollcontent.h"

#include <QWidget>
#include <QMap>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVBoxLayout>
#include <QDBusPendingCallWatcher>
#include <QDBusInterface>

class QStackedLayout;
class QListWidget;
struct ContainerMeta {
    QString m_name;
    QString m_repo;
    QString m_tag;
};

Q_DECLARE_METATYPE(ContainerMeta)

typedef QMap<QString, QString> Dict;
Q_DECLARE_METATYPE(Dict)

class CleanerWidget : public QWidget/*SettingScrollContent*/
{
    Q_OBJECT
public:
    explicit CleanerWidget(QWidget *parent = 0);
    ~CleanerWidget();

public slots:
    void onGetAllImagesInfoFinished(QDBusPendingCallWatcher *w);
    void onResponseAddImage(const QJsonObject &value, const QString &currentImage, const QString &container = "");
    void onResponseRemoveImage(const QString &id, const QString &containerName, const QString &imageName);
    void onRemoveAllImages();
    void onImageRemoved(const QString &imageName, bool success);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel *tiplabel = nullptr;
    QLabel *title = nullptr;
    CleanerItem *m_item = nullptr;
    QPushButton *m_clearButton = nullptr;
    QMap<CleanerItem*, QJsonObject> m_itemList;
    QDBusInterface *m_dbusInterface = nullptr;
    QVBoxLayout *m_itemsLayout = nullptr;
    QLabel *m_null = nullptr;
    QScrollArea *scrollarea = nullptr;
    QStackedLayout *m_stackedLayout = nullptr;
    QFrame *m_tipFrame = nullptr;
    QFrame *m_mainFrame = nullptr;
    QListWidget *m_list = nullptr;
    QListWidgetItem *remove_item = nullptr;
};

#endif // CLEANER_WIDGET_H
