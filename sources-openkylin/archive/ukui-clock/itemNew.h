/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef ITEM_NEW_H
#define ITEM_NEW_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QGSettings>
#include "gsettingsubject.h"
#include "constant_class.h"
#include "kwidget.h"
#include "kswitchbutton.h"
#include "selectbtnutil.h"

using namespace kdk;



namespace Ui {
class item_new;
}
/**
 * @brief 闹钟列表子项
 */
class item_new : public QWidget
{
    Q_OBJECT

public:
    explicit item_new(QWidget *parent = nullptr,int s = 0);
    ~item_new();

//private:

    void  setupUi(QWidget *item_new);
    void  retranslateUi(QWidget *item_new);
    void  settingsStyle();
    void  blackStyle();                                                                  //黑色主题
    void  whiteStyle();                                                                  //白色主题
    void closeStyle(int size);
    void openStyle(int size);
    void commonStyle(int size);
    QString getLabelStyleSheet(int size);


    Ui::item_new *ui;

    QPushButton *deleteAlarmBtn0;
    QLabel *alarmLabel0;
    QLabel *alarmLabel1;
    QLabel *alarmLabel_w0;
    QLabel *alarmLabel_s0;

    KSwitchButton *alarm_on_off0;
    QPushButton *alarm_changed0;
    QPushButton *alarm_edit0;

    QFrame *alarm_line0;


    QString id() const;
    void setId(const QString &id);
private:
    QString m_id;
    bool m_itemClose = false;
    SelectBtnUtil * m_selectBtnUtil = nullptr;
    void updateLabelStyle();
    void updatePosition(bool onTbalet);

};

#endif // ITEM_NEW_H
