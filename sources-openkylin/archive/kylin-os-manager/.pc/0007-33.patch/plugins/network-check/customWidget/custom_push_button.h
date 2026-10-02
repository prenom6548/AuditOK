/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef CUSTOM_PUSH_BUTTON_H
#define CUSTOM_PUSH_BUTTON_H

#include <QPushButton>
#include <QString>

class CustomPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit CustomPushButton(QWidget *parent = 0);
    explicit CustomPushButton(QString &text, QWidget *parent = 0);

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
