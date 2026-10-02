/* sessionsview.cpp
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
#include "sessionsview.h"
#include "hoverwidget.h"
#include <QListWidget>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QAction>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QWidgetAction>
#include <QLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QDateTime>
#include <QCoreApplication>
#include <QLightDM/SessionsModel>
#include <unistd.h>
#include "globalv.h"
#include "ktooltip.h"

SessionsView::SessionsView(QWidget *parent) :
    QWidget(parent),
    m_sessionsModel(nullptr),
    m_currentSession(-1)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground);

    QFile qss(":/ukui-greeter.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(qss.readAll());

}

SessionsView::~SessionsView()
{
}

bool SessionsView::eventFilter(QObject *obj, QEvent *event)
{
    for(int i = 0; i<m_nameList.count();i++){
        if(obj->objectName() == m_nameList.at(i)){
            if(event->type() == 110){
                QFontMetrics fontMetrics(font());
                if(fontMetrics.width(m_nameList.at(i)) > m_textlabel->width()){
                    Q_EMIT setKToolTip(m_nameList.at(i));
                }
            } else if (event->type() == 11){
                Q_EMIT hideKTooltip();
            }
        }
    }
    return false;
}

void SessionsView::setModel(QAbstractItemModel *model)
{
    m_sessionsModel = model;

    for(int i = 0; i < m_sessionsModel->rowCount(); i++){
        insertSessionEntry(i);
    }

}

void SessionsView::setCurrentSession(const QString &sessionName, bool entered)
{
    QString selectSessionName = "";
    for(int x = 0; x < m_sessionsModel->rowCount(); x++)
    {
       QString name= m_sessionsModel->index(x, 0).data(QLightDM::SessionsModel::KeyRole).toString();
       if(sessionName == name) {
           selectSessionName = sessionName;
           break;
       }
    }

    QMap<QString, QWidget*>::iterator itUserWidget = m_mapSessionWidget.begin();
    for( ; itUserWidget != m_mapSessionWidget.end(); itUserWidget++){
        if(itUserWidget.key() == sessionName){
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:rgb(255,255,255,40%);border-radius: 6px;}"
                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,40%);border-radius: 6px;}");
            m_strCurSelectName = sessionName;
        } else {
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:transparent;}"
                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
        }
    }
}

void SessionsView::onShowSessionmenu()
{
    m_sessionsMenu->show();
}

void SessionsView::insertSessionEntry(int row)
{
    if(!m_sessionsMenu){
        m_sessionsMenu = new QMenu(this);
        m_sessionsMenu->setWindowFlags(Qt::X11BypassWindowManagerHint);
        m_sessionsMenu->setToolTipsVisible(true);
        connect(m_sessionsMenu, &QMenu::triggered,
                       this, &SessionsView::onSessionMenuTrigged);
    }

    QString sessionName = m_sessionsModel->index(row, 0).data().toString();
    QString sessionKey = m_sessionsModel->index(row,0).data(QLightDM::SessionsModel::KeyRole).toString();
    QString sessionIcon = getSessionIcon(sessionKey);

    QWidgetAction *action = new QWidgetAction(m_sessionsMenu);
    action->setData(sessionKey);

    HoverWidget *widget = new HoverWidget("", this);
    widget->setStyleSheet("HoverWidget:hover{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
    widget->setObjectName(sessionKey+ "1");
    m_listWidgets.append(widget);
    m_mapSessionWidget[sessionKey] = widget;
    widget->setFixedSize(240, 40);
    QLabel *iconlabel =new QLabel(widget);
    QPixmap p(sessionIcon);
    p = cutPixmap(p);
    p = p.scaled(32*scale,32*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    p = PixmapToRound(p, 16*scale);
    iconlabel->setPixmap(p);
    iconlabel->setGeometry(8,4,32,32);
    m_textlabel =new KTipLabel(widget);
    m_textlabel->installEventFilter(this);
    m_textlabel->setStyleSheet("QToolTip{border-radius:4px;background-color:#FFFFFF;color:black;}");
    m_textlabel->setGeometry(48,8,190,24);
    action->setDefaultWidget(widget);

    m_textlabel->setObjectName(sessionName);
    m_nameList.append(m_textlabel->objectName());
    m_textlabel->setText(sessionName);
    m_sessionsMenu->addAction(action);
    m_sessionsMenu->adjustSize();
}

void SessionsView::onSessionMenuTrigged(QAction *action)
{
    QString selectSessionName = "";
    QString ssKey = action->data().toString();
    for(int x = 0; x < m_sessionsModel->rowCount(); x++)
    {
       QString sessionKey= m_sessionsModel->index(x, 0).data(QLightDM::SessionsModel::KeyRole).toString();
        if(ssKey == sessionKey){
            selectSessionName = ssKey;
            break;
        }
    }

    QMap<QString, QWidget*>::iterator itUserWidget = m_mapSessionWidget.begin();
    for ( ; itUserWidget != m_mapSessionWidget.end(); itUserWidget++) {
        if(itUserWidget.key() == ssKey){
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:rgb(255,255,255,40%);border-radius: 6px;}"
                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,40%);border-radius: 6px;}");
            m_strCurSelectName = ssKey;
        } else {
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:transparent;}"
                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
        }
    }
    Q_EMIT currentSessionChanged(selectSessionName);
}

void SessionsView::keyBdRelease(int key)
{
    if(key == Qt::Key_Up){
        for (int i = 0; i < m_listWidgets.count(); i++) {
            if(m_strCurSelectName+"1" == m_listWidgets[i]->objectName()){
                if(i == 0)
                    return ;
                m_listWidgets.at(i)->setStyleSheet("HoverWidget{background-color:transparent;}"
                                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
                m_listWidgets.at(i-1)->setStyleSheet("HoverWidget{background-color:rgb(255,255,255,40%);border-radius: 6px;}"
                                          "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
                m_strCurSelectName = m_listWidgets[i-1]->objectName().mid(0, m_listWidgets[i-1]->objectName().size()-1);

            }
        }
    } else if(key == Qt::Key_Down) {
        for(int i = m_listWidgets.count()-1; i >= 0; i--){
            if(m_strCurSelectName+"1" == m_listWidgets[i]->objectName()){
                if(i == m_listWidgets.count() - 1)
                    return ;
                m_listWidgets.at(i)->setStyleSheet("HoverWidget{background-color:transparent;}"
                                                      "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
                m_listWidgets.at(i+1)->setStyleSheet("HoverWidget{background-color:rgb(255,255,255,40%);border-radius: 6px;}"
                                          "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
                m_strCurSelectName = m_listWidgets[i+1]->objectName().mid(0, m_listWidgets[i+1]->objectName().size()-1);
            }
        }
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        for(int i = 0; i < m_sessionsModel->rowCount(); i++){
            QString name = m_sessionsModel->index(i, 0).data(QLightDM::SessionsModel::KeyRole).toString();
            if(name == m_strCurSelectName) {
                Q_EMIT currentSessionChanged(m_strCurSelectName);
                if (m_mapSessionWidget.contains(m_strCurSelectName)) {
                    m_mapSessionWidget[m_strCurSelectName]->setStyleSheet("HoverWidget{background-color:rgb(255,255,255,40%);border-radius: 6px;}"
                                          "HoverWidget:hover:!pressed{background-color:rgb(255,255,255,15%);border-radius: 6px;}");
                }
            }
        }
    }
}

QString SessionsView::getSessionIcon(const QString &session)
{
    QString sessionPrefix = session.left(session.indexOf('-'));
    if(session == "ukui-wayland")
        sessionPrefix = "ukui_wayland";

    QString sessionIcon = IMAGE_DIR + QString("badges/%1_badge.svg").arg(sessionPrefix.toLower());
    QFile iconFile(sessionIcon);
    if(!iconFile.exists()){
        sessionIcon = IMAGE_DIR + QString("badges/unknown_badge.svg");
    }
    return sessionIcon;
}
