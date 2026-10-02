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

#ifndef LOG_WIDGET_H
#define LOG_WIDGET_H

#include <QWidget>

#include "settingscrollcontent.h"

class QPushButton;

class LogWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit LogWidget(const QString &userName, const QString &userId, QWidget *parent = 0);
    ~LogWidget();

    void startGetDisplayInfo();
    void startGetServiceStatus();
    void startCopyLogFils();

public slots:
    void onStart();

private:
    QPushButton* m_startButton = nullptr;
    QPushButton* m_backButton = nullptr;
    QLabel *m_loading = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_tipsLabel = nullptr;
    QLabel *log_pic = nullptr;
    QLabel *finish_pic = nullptr;
    QString m_loginUserName;
    QString m_loginUserId;
    QString m_displayInfo;
    QString m_logPath;
};

#endif // CLEANER_WIDGET_H
