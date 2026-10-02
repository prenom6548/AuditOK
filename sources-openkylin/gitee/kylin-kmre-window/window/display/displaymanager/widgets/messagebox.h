/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>

#include "dialog.h"

class QLabel;
class QVBoxLayout;
class QHBoxLayout;

namespace KylinUI {
namespace MessageBox {


enum MessageType{
    INFORMATION,
    QUESTION,
    WARNING,
    ABOUT
};

/// 没写静态函数，调用接直接临时变量exec，有个确认取消，写静态太麻烦
class CustomerMessageBox : public Dialog
{
    Q_OBJECT
public:
    CustomerMessageBox(QString title, QString text, QWidget *parent = nullptr, MessageType type = INFORMATION);
    ~CustomerMessageBox();

private:
    MessageType msg_type;
    QLabel  *lab_titleIcon,
            *lab_title,
            *lab_text,
            *lab_textIcon;
    QPushButton *btn_ok,
                *btn_cancel;
    QVBoxLayout *box_v;
    QHBoxLayout *box_ht,
                *box_hm,
                *box_hb;

    void setData(QString title, QString text);
    void initLayout();
};
    void information(QWidget *parent, QString title, QString text);
    void warning(QWidget *parent, QString title, QString text);
    bool question(QWidget *parent, QString title, QString text);

} //MessageBox
} //KylinUI


#endif // MESSAGEBOX_H
