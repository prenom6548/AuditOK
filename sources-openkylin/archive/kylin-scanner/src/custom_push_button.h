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

#ifndef CUSTOM_PUSH_BUTTON_H
#define CUSTOM_PUSH_BUTTON_H

#include <kpushbutton.h>
#include <QString>

class CustomPushButton : public kdk::KPushButton
{
    Q_OBJECT
public:
    explicit CustomPushButton(QWidget *parent=0);
    explicit CustomPushButton(QString &text, QWidget *parent=0);

    void setText(const QString &text);
    void setFullText(const QString &text);
    void setTextLimitShrink(const QString &text, int width);
    void setTextLimitExpand(const QString &text);
    QString fullText() const;
protected:
    void paintEvent(QPaintEvent *);

private:
    void elideText();
private:
    QString m_fullText;
};
#endif // CUSTOM_PUSH_BUTTON_H
