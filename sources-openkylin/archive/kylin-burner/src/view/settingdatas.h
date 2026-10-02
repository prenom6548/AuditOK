/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef SETTINGDATAS_H
#define SETTINGDATAS_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class SettingDatas;
}

class SettingDatas : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void change(bool);
    void title(QString);
public:
    explicit SettingDatas(QWidget *parent = nullptr);
    ~SettingDatas();
public:
    void load();
public slots:
    void accept();
    void resetText(QLabel* item);
    void updateLabelText();
    void slotChangeUIFontSize(int fontSize);
    void updateLabelWidth(QFont font);
private slots:
    void check();
private:
    Ui::SettingDatas *ui;
private:
    int       m_iDuplicate;
    bool      m_bAdd;
    bool      m_bDelete;
    bool      m_bClear;
};

#endif // SETTINGDATAS_H
