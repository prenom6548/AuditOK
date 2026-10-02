/* powerwindow.h
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

#ifndef POWERMANAGER_H
#define POWERMANAGER_H
#include <QWidget>
#include <QListWidget>
#include <QLightDM/Power>
#include <QSize>
#include <QLabel>
#include <QPushButton>
#include <QTime>
#include <QDBusArgument>

#include "common/xeventmonitor.h"

#define ITEM_WIDTH 204
#define ITEM_HEIGHT 200
#define ITEM_SPACING 30

enum stateType {
    SHOTDOWN,
    REBOOT,
    SLEEP,
    HIBERNATE,
    NOTHING
};

class KLabel;
class QListWidget;
class QListWidgetItem;
class QDBusInterface;
class Configuration;
class PowerManager:public QListWidget
{
    Q_OBJECT

public:
    PowerManager( int num, QWidget *parent = 0);
    QSize windowSize();
    void doEvent(int type);
    void keyBdRelease(int key);
    void clearStatus();
    void cancelClicked();

private:
    void initUI();
    QList<QPushButton *>     listLabel;
    int nowAt = -1;
    QListWidget *list;
    QWidget *lockWidget;
    QWidget *switchWidget = nullptr;
    QWidget *suspendWidget;
    QWidget *rebootWidget;
    QWidget *shutdownWidget;
    QWidget *hibernateWidget;
    KLabel *switchLabel = nullptr;
    KLabel *rebootLabel;
    KLabel *shutdownLabel;
    QLightDM::PowerInterface *m_power;
    QTime lasttime;
    QDBusInterface      *actService;
    int loginNum = 0;
    int blockInhibited = NOTHING;
    Configuration *m_configuration;
    double curFontSize;
private:
    void lockWidgetClicked();
    void switchWidgetClicked();
    void suspendWidgetClicked();
    void rebootWidgetClicked();
    void shutdownWidgetClicked();
    void hibernateWidgetClicked();
    void refreshTranslate();
    void keyBdRelease(QString key);
    void setButtonStyle(QString Style);
    QStringList getLoginedUsers();

private Q_SLOTS:
    void powerClicked(QListWidgetItem *item);
    bool eventFilter(QObject *obj, QEvent *event);
    void onGlobalKeyPress(const QString &key);
    void onGlobalkeyRelease(const QString &key);
    void onHibitedWatcherMessage(void);
    //void keyboardPressed(QListWidgetItem *item,QListWidgetItem*);
Q_SIGNALS:
    void switchToUser();
    void lock();
    void is_blockInhibited(int stateType, int lockType);
    void setKToolTip(const QString text);
    void hideKTooltip();
};

#endif // POWERMANAGER_H
