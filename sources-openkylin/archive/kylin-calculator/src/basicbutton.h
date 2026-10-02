/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef FUNCBUTTON_H
#define FUNCBUTTON_H

#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPushButton>

class BasicButton : public QPushButton
{
    Q_OBJECT
public:
    explicit BasicButton(QWidget *parent = 0);
    ~BasicButton() {}

    // 按钮所代表的意义
    QString btnText;

    // 设置按钮内容
    void setText(const QString &text);

    // 获取按钮内容
    QString getText();

    // 获取按钮内容
    QString text();

};

class IntelModeButton : public QPushButton
{
    Q_OBJECT

public:
    IntelModeButton(QWidget *parent = 0);
    ~IntelModeButton();

    void init(QString text);
    void select(void);
    void deselect(void);
    bool getStatus();
    void setTextColor(QString color);

    QLabel *m_text;
    QLabel *m_icon;
    QHBoxLayout *m_hlayout;

private:
    bool m_status;
};

class IntelModeList : public QWidget
{
    Q_OBJECT

public:
    IntelModeList(QWidget *parent = 0);
    ~IntelModeList();

    IntelModeButton *m_standard;
    IntelModeButton *m_science;

    QVBoxLayout *m_vlayout;

    void init(void);
    void establishInterrupt(void);
    void setWidgetStyle(void);

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void slotStandardClick(void);
    void slotScienceClick(void);
};

#endif // FUNCBUTTON_H
