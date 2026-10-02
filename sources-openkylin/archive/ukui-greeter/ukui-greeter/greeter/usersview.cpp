/* usersview.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "usersview.h"
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
#include <QLightDM/UsersModel>
#include <QPropertyAnimation>
#include <unistd.h>
#include <QGraphicsOpacityEffect>
#include "xeventmonitor.h"
#include "globalv.h"
#include "securityuser.h"
#include "common/configuration.h"
#include "ktooltip.h"
#include "greeterutils.h"

/*记录五个节点的坐标*/
#define ARROW_WIDTH 120*scale
#define LEFT_ARROW_X 0
#define ITEM1_X (ARROW_WIDTH)
#define ITEM2_X (ITEM1_X + CENTER_ENTRY_WIDTH * 2 - ENTRY_WIDTH)
#define ITEM3_X (ITEM2_X + CENTER_ENTRY_WIDTH * 2 - ENTRY_WIDTH )
#define ITEM4_X (ITEM3_X + CENTER_ENTRY_WIDTH*3 - ENTRY_WIDTH*2)
#define ITEM5_X (ITEM4_X + CENTER_ENTRY_WIDTH * 2 - ENTRY_WIDTH)
#define RIGHT_ARROW_X (CENTER_ENTRY_WIDTH*9 - ENTRY_WIDTH*4 + 220*scale)
#define ITEM_Y CENTER_ENTRY_HEIGHT-ENTRY_HEIGHT
#define ITEM_CENTER_Y  0

UsersView::UsersView(QWidget *parent) :
    QWidget(parent),
    usersModel(nullptr),
    currentUser(-1),
    prevArrow(nullptr),
    nextArrow(nullptr),
    secUser(SecurityUser::instance()),
    lasttime(QTime::currentTime()),
    m_configuration(Configuration::instance()),
    mouseClickLast(QTime::currentTime())
{
    setFocusPolicy(Qt::NoFocus);
    connect(XEventMonitor::instance(), SIGNAL(keyPress(const QString &)),
            this, SLOT(onGlobalKeyRelease(const QString &)));

    setAttribute(Qt::WA_TranslucentBackground);

    QFile qss(":/ukui-greeter.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(qss.readAll());

}

UsersView::~UsersView()
{
}

void UsersView::resizeEvent(QResizeEvent *)
{
    //setCurrentRow(currentUser);
}

bool UsersView::eventFilter(QObject *obj, QEvent *event)
{
    for(int i = 0; i<nameList.count();i++){
        if(obj->objectName() == nameList.at(i)){
            if(event->type() == 110){
                QFontMetrics fontMetrics(font());
                if(fontMetrics.width(nameList.at(i)) > textlabel->width()){
                    Q_EMIT setKToolTip(nameList.at(i));
                }
            } else if (event->type() == 11){
                Q_EMIT hideKTooltip();
            }
        }
    }
    return false;
}

void UsersView::setModel(QAbstractListModel *model)
{
    usersModel = model;
    connect(usersModel, &QAbstractListModel::rowsInserted, this, &UsersView::onUserAdded);
    connect(usersModel, &QAbstractListModel::rowsRemoved, this, &UsersView::onUserRemoved);
    connect(usersModel, &QAbstractListModel::dataChanged, this, &UsersView::onUserChanged);

    for(int i = 0; i < usersModel->rowCount(); i++){
        QString name = usersModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
        if(secUser->isSecrityUser(name))
            insertUserEntry(i);
    }

}

void UsersView::setCurrentUser(const QString &userName, bool entered)
{
    int x = 0;
    for(x = 0; x < usersModel->rowCount(); x++)
    {
       QString name= usersModel->index(x).data(QLightDM::UsersModel::NameRole).toString();
       if(userName == name)
           break;
    }

    QModelIndex index = usersModel->index(x, 0);
    QMap<QString, QWidget*>::iterator itUserWidget = m_mapUserWidget.begin();
    m_configuration->getCurrentUser(userName);
    fontSize = m_configuration->getUserConfig("fontSize").toInt();
    usersMenu->setStyleSheet(QString("QMenu{background-color: rgba(255,255,255,15%);color: white;border-radius: 8px;width:250px;padding: 5px 5px 5px 5px;} \
                              QMenu::icon{padding: 2px 5px 2px 5px;}\
                              QMenu::item{width:225px;border-radius: 4px;height:36px;;padding: 2px 10px 2px 10px;} \
                              QMenu::item:selected {border-radius: 4px;background-color:rgba(255,255,255,40%);} \
                              QMenu::item:pressed {border-radius: 4px;background-color: rgba(255,255,255,40%);} \
                              QLabel {color: white; font-size:%1px}").arg(16 + fontSize));
    for( ; itUserWidget != m_mapUserWidget.end(); itUserWidget++){
        if(itUserWidget.key() == userName){
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,40%);border-radius: 6px;}");
            m_strCurSelectName = userName;
        } else {
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:transparent;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
        }
    }
    textlabel->setFontSize(16 + fontSize);
    Q_EMIT currentUserChanged(index);
    Q_EMIT userSelected(index);
    if(unList.contains(userName)){
        return ;
    }
    Q_EMIT userNotFound(userName);
}

void UsersView::onGlobalKeyRelease(const QString &key)
{

}

void UsersView::showEvent(QShowEvent *event)
{

    QWidget::showEvent(event);
}

void UsersView::onShowUsermenu()
{
    usersMenu->show();
}

void UsersView::insertUserEntry(int row)
{
//    QModelIndex index = usersModel->index(row, 0);
//    QPersistentModelIndex persistentIndex(index);
//    UserEntry *entry = new UserEntry(this);
//    entry->setUserIndex(persistentIndex);
//    entry->installEventFilter(this);

//    entry->hide();

//    userlist.append(entry);
    if(!usersMenu){
        usersMenu = new QMenu(this);
        usersMenu->setWindowFlags(Qt::X11BypassWindowManagerHint);
        usersMenu->setToolTipsVisible(true);
        connect(usersMenu, &QMenu::triggered,
                       this, &UsersView::onUserMenuTrigged);

    }

    QString realName = usersModel->index(row,0).data(QLightDM::UsersModel::RealNameRole).toString();
    QString userName = usersModel->index(row,0).data(QLightDM::UsersModel::NameRole).toString();
    if(realName.isEmpty())
        realName = userName;

    QString iconPath = usersModel->index(row,0).data(QLightDM::UsersModel::ImagePathRole).toString();
    if(!QFile(iconPath).exists())
    {
        iconPath = UKUIGreeter::Utils::getDefaultFace();
    }

    QWidgetAction *action = new QWidgetAction(usersMenu);
    action->setData(userName);

    HoverWidget *widget = new HoverWidget("", this);
    widget->setStyleSheet("HoverWidget:hover{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
    widget->setObjectName(userName+ "1");
    list.append(widget);
    m_mapUserWidget[userName] = widget;
    widget->setFixedSize(240, 40);
    QLabel *iconlabel =new QLabel(widget);
    QPixmap p(iconPath);
    p = cutPixmap(p);
    p = p.scaled(32*scale,32*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    p = PixmapToRound(p, 16*scale);
    iconlabel->setPixmap(p);
    iconlabel->setGeometry(8,4,32,32);
    textlabel =new KTipLabel(widget);
    textlabel->installEventFilter(this);
    textlabel->setGeometry(48,4,190,30);
    if(getLoginStaus(userName)){
        QLabel *statusIcon = new QLabel(widget);
        QPixmap status(":/images/selected.svg");
        status = cutPixmap(status);
        status = status.scaled(14*scale,14*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        p = PixmapToRound(status, 7*scale);
        statusIcon->setPixmap(status);
        statusIcon->setGeometry(32, 5, 14, 14);
    }
    action->setDefaultWidget(widget);

    textlabel->setObjectName(realName);
    unList.append(userName);
    nameList.append(textlabel->objectName());
    textlabel->setText(realName);
    usersMenu->addAction(action);
    usersMenu->adjustSize();
}

void UsersView::onUserMenuTrigged(QAction *action)
{
    QString userName = action->data().toString();
    int x = 0;
    for(x = 0; x < usersModel->rowCount(); x++)
    {
       QString name= usersModel->index(x).data(QLightDM::UsersModel::NameRole).toString();
        if(userName == name){
            break;
        }
    }

    QMap<QString, QWidget*>::iterator itUserWidget = m_mapUserWidget.begin();
    for ( ; itUserWidget != m_mapUserWidget.end(); itUserWidget++) {
        if(itUserWidget.key() == userName){
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,40%);border-radius: 6px;}");
            m_strCurSelectName = userName;
        } else {
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:transparent;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
        }
    }
    QModelIndex index = usersModel->index(x, 0);
    Q_EMIT currentUserChanged(index);
    Q_EMIT userSelected(index);
}

void UsersView::removeUserEntry(int row)
{

}

void UsersView::onUserAdded(const QModelIndex &parent, int left, int right)
{
    Q_UNUSED(parent);

    for(int i = left; i <= right; i++){
        QString name = usersModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
        if(secUser->isSecrityUser(name))
            insertUserEntry(i);
    }
}

void UsersView::onUserRemoved(const QModelIndex &parent, int left, int right)
{
    Q_UNUSED(parent);

    for(int i = left; i <= right; i++){
        removeUserEntry(i);
    }
}

void UsersView::onUserChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int left = topLeft.row();
    int right = bottomRight.row();
    refreshTranslations(left, right);

//    for(int i = left;i<=right;i++)
//    {
//        QModelIndex index = usersModel->index(i,0);
//        uid_t uid =  index.data(QLightDM::UsersModel::UidRole).toUInt();
//        for(int j = 0;j<userlist.count();j++)
//        {
//            UserEntry *entry = userlist.at(j);
//            uid_t id = entry->userIndex().data(QLightDM::UsersModel::UidRole).toUInt();
//            if(id == uid)
//            {
//                entry->setUserName(index.data(Qt::DisplayRole).toString());
//                entry->setFace(index.data(QLightDM::UsersModel::ImagePathRole).toString());
//                entry->setLogin(index.data(QLightDM::UsersModel::LoggedInRole).toBool());
//                break;
//            }
//        }
//    }
}

void UsersView::refreshTranslations(int nBegin, int nEnd)
{
    for(int i = nBegin; i <= nEnd; i++) {
        QModelIndex index = usersModel->index(i, 0);
        QString strName =  index.data(QLightDM::UsersModel::NameRole).toString();
        QString strRealName =  index.data(QLightDM::UsersModel::RealNameRole).toString();
        if (strName == "*login") {
            if (usersMenu) {
                for (auto action : usersMenu->actions()) {
                    QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(action);
                    if (widgetAction && widgetAction->data().toString() == strName) {
                        QWidget *subWidget = widgetAction->defaultWidget();
                        if (subWidget) {
                            for (auto widget : subWidget->children()) {
                                if (QString(widget->metaObject()->className()) == "KLabel") {
                                    KLabel *label = qobject_cast<KLabel*>(widget);
                                    if (label) {
                                        label->setText(strRealName);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool UsersView::getIsUser(QString username)
{
    for(int i = 0; i < usersModel->rowCount(); i++){
        QString name = usersModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
    	if(name == username)
	    return true;
    }
    return false;
}

bool UsersView::getLoginStaus(QString username)
{
    for(int i = 0; i < usersModel->rowCount(); i++){
        QString name = usersModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
        if(name == username){
 	    return usersModel->index(i).data(QLightDM::UsersModel::LoggedInRole).toBool();	
	}
    }
}

int UsersView::getLoginNum()
{
    int num = 0;
    for(int i =0; i < usersModel->rowCount(); i++) {
        if(usersModel->index(i).data(QLightDM::UsersModel::LoggedInRole).toBool())
            num++;
    }
    return num;
}

void UsersView::setCurrentRow(int user)
{
    if(usersMenu->actions().count() == 0)
        return ;
    if(user < 0 || user >= usersMenu->actions().count())
        user = 0;

    currentUser = user;

    QModelIndex index = usersModel->index(user, 0);
    QString strCurUserName = index.data(QLightDM::UsersModel::NameRole).toString();
    QMap<QString, QWidget*>::iterator itUserWidget = m_mapUserWidget.begin();
    for ( ; itUserWidget != m_mapUserWidget.end(); itUserWidget++) {
        if(itUserWidget.key() == strCurUserName){
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,40%);border-radius: 6px;}");
            m_strCurSelectName = strCurUserName;
        } else {
            itUserWidget.value()->setStyleSheet("HoverWidget{background-color:transparent;}"
                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
        }
    }

    usersMenu->adjustSize();

    Q_EMIT currentUserChanged(index);
    Q_EMIT userSelected(index);
}

void UsersView::keyBdRelease(int key)
{
    QString strLastSelectName = m_strCurSelectName;
    if(key == Qt::Key_Up){
        for (int i = 0; i < list.count(); i++) {
            if(m_strCurSelectName+"1" == list[i]->objectName()){
                if(i == 0)
                    return ;
                list.at(i)->setStyleSheet("HoverWidget{background-color:transparent;}"
                                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
                list.at(i-1)->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                          "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
                m_strCurSelectName = list[i-1]->objectName().mid(0, list[i-1]->objectName().size()-1);

            }
        }
    } else if(key == Qt::Key_Down) {
        for(int i = list.count()-1; i >= 0; i--){
            if(m_strCurSelectName+"1" == list[i]->objectName()){
                if(i == list.count() - 1)
                    return ;
                list.at(i)->setStyleSheet("HoverWidget{background-color:transparent;}"
                                                      "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
                list.at(i+1)->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                          "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
                m_strCurSelectName = list[i+1]->objectName().mid(0, list[i+1]->objectName().size()-1);
            }
        }
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        //if (strLastSelectName != m_strCurSelectName) {
            for(int i = 0; i < usersModel->rowCount(); i++){
                QString name = usersModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
                if(name == m_strCurSelectName) {
                    Q_EMIT currentUserChanged(usersModel->index(i));
                    Q_EMIT userSelected(usersModel->index(i));
                    if (m_mapUserWidget.contains(m_strCurSelectName)) {
                        m_mapUserWidget[m_strCurSelectName]->setStyleSheet("HoverWidget{background-color:rgba(255,255,255,40%);border-radius: 6px;}"
                                              "HoverWidget:hover:!pressed{background-color:rgba(255,255,255,15%);border-radius: 6px;}");
                    }
                }
            }
        //}
    }
}

