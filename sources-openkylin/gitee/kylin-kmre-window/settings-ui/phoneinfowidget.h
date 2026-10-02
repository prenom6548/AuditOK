/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#ifndef PHONEINFOWIDGET_H
#define PHONEINFOWIDGET_H

#include <QWidget>
#include <QPushButton>

#include "settingscrollcontent.h"

class QRadioButton;
class QLineEdit;
class QComboBox;
class Preferences;
class QDBusInterface;
class PhoneInfoWidget : public SettingScrollContent
{
    Q_OBJECT
public:
    explicit PhoneInfoWidget(QWidget *parent = 0);
    ~PhoneInfoWidget();
    void setpresetbrandInfo(int index);
    void setpresetmodelInfo();
    void setrandimei();
    void setimeiInfo();
    void setcustombrand();
    void setcustommodel();
    Preferences *m_pref = nullptr;

signals:
    void restartEnv();

public slots:
    void setcustomvisible(bool checked);
    void setpresetvisible(bool checked);

private:
    QFrame *presetFrame = nullptr;
    QFrame *customFrame = nullptr;
    QLabel *modelLabel = nullptr;
    QLabel *IMEILabel = nullptr;
    QComboBox *presetBox1 = nullptr;
    QComboBox *presetBox2 = nullptr;
    QComboBox *presetBox3 = nullptr;
    QComboBox *presetBox4 = nullptr;
    QComboBox *presetBox5 = nullptr;
    QComboBox *presetBox6 = nullptr;
    QLabel *Label1 = nullptr;
    QLabel *Label2 = nullptr;
    QLabel *Label3 = nullptr;
    QLabel *Label4 = nullptr;
    QLineEdit *line1 = nullptr;
    QLineEdit *line2 = nullptr;
    QLineEdit *IMEIline = nullptr;
    QString IMEI;
    QString Equip;
    QString Vendor;
    QString Brand;
    QString Name;
    QString Model;
    QString Serialno;
    QString Board;
    bool custom;
    QString m_UserName;
    QString m_UserId;
    QDBusInterface *m_systemBusInterface = nullptr;
};

#endif // PHONEINFOWIDGET_H
