/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#ifndef BASE_KEY_H
#define BASE_KEY_H

#include <QWidget>
#include <QKeyEvent>
#include <QPushButton>

#include <QGraphicsOpacityEffect>

class KmreWindow;

class OverlayButton : public QPushButton
{
    Q_OBJECT
public:
    explicit OverlayButton(KmreWindow* window, QWidget *parent = nullptr);
    ~OverlayButton(){}

protected:
    bool eventFilter(QObject *obj, QEvent *evt) Q_DECL_OVERRIDE;

    KmreWindow* mMainWindow = nullptr;
};

class BaseKey : public QWidget
{
    Q_OBJECT
public:
    explicit BaseKey(KmreWindow* window);
    ~BaseKey();
    
    virtual void enableEdit(bool enable) = 0;
    virtual void updateSize(bool update_panel = true) = 0;
    virtual void updatePos() = 0;

    void showKey(bool show);
    void setOpacity(double opacity);
    bool isInEditing(){return m_enableEdit;}

protected:
    void checkPosOutsideScreen(QPoint pos);

protected:
    QGraphicsOpacityEffect *opacityEffect = nullptr;

    QPoint m_lastPos;//鼠标按下处坐标
    bool m_mousePressed;//鼠标按下
    bool m_enableEdit;//使能编辑（可调整大小位置及绑定key）
    bool m_outsideScreen;//移出屏幕外
    
    KmreWindow* mMainWindow = nullptr;
};

#endif // BASE_KEY_H
