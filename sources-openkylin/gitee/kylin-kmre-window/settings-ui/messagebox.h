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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>

#ifdef UKUI_WAYLAND
#include "dialog.h"

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
#endif

namespace KylinUI {
namespace MessageBox {

#ifdef UKUI_WAYLAND
#define TITLE_ICON_WIDTH 20
#define TEXT_ICON_WIDTH 22
#define WIDGET_WIDTH 380
#define WIDGET_HEIGHT 200

enum MessageType{
    INFORMATION,
    CRITICAL,
    QUESTION,
    WARNING,
    ABOUT
};

/// 没写静态函数，调用接直接临时变量exec，有个确认取消，写静态太麻烦

class MessageBox : public Dialog
{
    Q_OBJECT
public:
    MessageBox(QString title, QString text, QWidget *parent = nullptr, MessageType type = INFORMATION);
    ~MessageBox();

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
#endif
    void information(QWidget *parent, QString title, QString text, bool block = false);
    void critical(QWidget *parent, QString title, QString text, bool block = false);
    void warning(QWidget *parent, QString title, QString text, bool block = false);
    bool question(QWidget *parent, QString title, QString text);
    bool restartEnv(QWidget *parent);
} //MessageBox
} //KylinUI


#endif // MESSAGEBOX_H
