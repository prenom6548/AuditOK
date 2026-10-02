/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DATETIME_H
#define DATETIME_H

#include "datetimeui.h"
#include "shell/interface.h"
#include "mthread.h"
#include <QObject>
#include <QEvent>
#include "ukcccommon.h"
#include "timezonechooserdialog.h"
using namespace ukcc;

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif
#include <glib.h>
#include <gio/gio.h>

class UkccArgEvent : public QEvent
{
private:
    QString mstr;

public:
    const static Type TYPE = static_cast<Type>(QEvent::User+0x10);
    explicit UkccArgEvent(QString s) : QEvent(TYPE)
    {
        mstr = s;
    }

    QString str()
    {
        return mstr;
    }
};

class DateTime : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DateTime();
    ~DateTime();
    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const          Q_DECL_OVERRIDE;
    bool isEnable() const       Q_DECL_OVERRIDE;
    void connectToServer();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void changezoneSlot(int flag = 0);
    void ntpServerChangedSlot(QString server);
    void timezoneChooserConfirmSlot(QString timezone, QString title);
    void dataModeChangeSlot(QDBusPendingCallWatcher *call, int toggledBtn, QString mode);
    void setNtpSerAddressSlot(QDBusPendingCallWatcher *watcher);

public:
    void initContent();
    void initTimer();
    void initDate();
    void initTimezone();
    void initTimeModeSet();
    void initNtpServers();
    void initNtpServer();
    void connectUiSignals();
    void initOtherTimezone();

public Q_SLOTS:
    void dataChanged(QString key);
    void showTimezoneMap(int flag = 0);
    void setEnableBySecurity();
    void ntpServerChangedDefaultSlot();

private Q_SLOTS:
    void keyChangedSlot(const QString &key);

private:
    bool            mFirstLoad;
    QString         pluginName;
    int             pluginType;
    DatetimeUi      *datetimeUi        = nullptr;
    QDBusInterface  *datetimeInterface = nullptr;
    QTimer          *timer             = nullptr;
    int             checkTimes         = 0;
    int             timerTimes         = 0;
    QStringList     otherTimezoneList  = {};
    QString         toChangeKey = "";
    QDBusInterface  *m_pUkccSystemDbus = nullptr;
    QString m_uiMode;
    TimeZoneChooserDialog* m_timeZoneChooserDialog = nullptr;
};

#endif // DATETIME_H
