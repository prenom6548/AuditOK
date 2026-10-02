/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#pragma once

#include "settingscrollcontent.h"
#include "preferences.h"
#include <QProcess>
#include <QProgressDialog>

class AdvancedWidget : public SettingScrollContent
{
    Q_OBJECT
public:
    explicit AdvancedWidget(QWidget * parent = 0);
    void init();

private slots:
    void slotCloseKmre();
    void slotUninstallKmre();
    void slotReadProgress();
    void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Preferences *mPref;
    bool mClearUserData;
    QProcess *mUninstallProcess;
    QProgressDialog *mUninstallProgress;
};