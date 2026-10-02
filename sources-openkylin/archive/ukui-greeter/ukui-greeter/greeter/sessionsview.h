/* sessionsview.h
 * Copyright (C) 2023 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef SESSIONSVIEW_H
#define SESSIONSVIEW_H

#include <QWidget>
#include <QTime>
#include <QPair>
#include <QSize>
#include <QMenu>
#include "hoverwidget.h"
#include "ktiplabel.h"
#include "ktooltip.h"
class QPushButton;
class QAbstractItemModel;
class QAction;
class SessionsView : public QWidget
{
    Q_OBJECT

public:
    explicit SessionsView(QWidget *parent = 0);
    ~SessionsView();
    void setModel(QAbstractItemModel *);
    void setCurrentSession(const QString &sessionName, bool entered = false);
    void onShowSessionmenu();
    void keyBdRelease(int key);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void onSessionMenuTrigged(QAction *action);

signals:
    void currentSessionChanged(const QString &session);
    void setKToolTip(const QString text);
    void hideKTooltip();

private:
    void insertSessionEntry(int row);
    QString getSessionIcon(const QString &session);

private:
    QAbstractItemModel *m_sessionsModel;
    int m_currentSession;
    QList<QWidget *> m_listWidgets;
    QMap<QString, QWidget *> m_mapSessionWidget;
    KTipLabel *m_textlabel = nullptr;
    QStringList m_nameList;
    QMenu *m_sessionsMenu = nullptr;
    QString m_strCurSelectName = "";
};

#endif // SESSIONSVIEW_H
