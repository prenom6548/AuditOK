/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef STATUSBUTTON_H
#define STATUSBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStylePainter>
#include <QStyleOption>

enum BUTTONSTATUS
{
    NORMAL,
    HOVER,
    SELECT,
    CLICKED,
};

enum BUTTONTYPE
{
    BOTBUTTON,
    BIOBUTTON,
    POWERBUTTON,
    NORMALBTN,
    PWDTIPBTN,
    SIDEBARBTN,
};

class StatusButton : public QPushButton
{
    Q_OBJECT
public:
    explicit StatusButton(QWidget *parent = nullptr, int type = BOTBUTTON);
    /**
     * @brief 设置按钮当前状态
     * @param selected 是否选中
     * @return
     */
    void setClickedStatus(int status);

    void setDrivedId(int drivedId);

    inline int getDrivedId()
    {
        return m_drivedId;
    }
    inline int getButtonType()
    {
        return buttonType;
    }
    void setToolTip(const QString &text);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

    bool event(QEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

Q_SIGNALS:
    /**
     * @brief 响应点击事件
     * @return
     */
    void clicked();
    void buttonClicked(int drivedId);

private:
    int curStatus = NORMAL;
    int buttonType;
    int m_drivedId;

    bool m_isHovered = false;
    QPoint m_lastMousePos;
};

#endif // STATUSBUTTON_H
