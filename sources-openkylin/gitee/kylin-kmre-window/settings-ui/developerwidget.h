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

#ifndef DEVELOPERWIDGET_H
#define DEVELOPERWIDGET_H

#include "settingscrollcontent.h"

#include <QStackedLayout>
class QVBoxLayout;
class QCheckBox;
class DeveloperWidget : public SettingScrollContent
{
    Q_OBJECT
public:
    explicit DeveloperWidget(QWidget * parent = 0);
    void modecheck(bool checked);
    void setClicktime();
    void initDatas();
    QString getContainerIp();
    QFrame *m_tipFrame = nullptr;
    QFrame *m_mainFrame = nullptr;
    QStackedLayout *m_stackedLayout = nullptr;
    QCheckBox *m_check = nullptr;
    QLabel *m_tip = nullptr;
    QLabel *m_tipLabel = nullptr;
//    QLabel *m_armtip = nullptr;
    QString m_ipadr;

};

#endif // DEVELOPERWIDGET_H
