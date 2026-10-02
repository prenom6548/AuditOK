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

#ifndef TITLEBAR_H
#define TITLEBAR_H


#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "ukuiwindowhelper.h"

#include "menumodule.h"
#include "view/common/globalsizedata.h"

class TitleBar : public QWidget
{
    Q_OBJECT
    friend class TestTitleBar;
public:
    explicit TitleBar(QWidget *parent = nullptr);
    explicit TitleBar(bool isMenu, bool isClose, bool isBlur = true, QWidget *parent = nullptr);
    ~TitleBar();

    // 初始化数据
    void setInitData(bool isMenu = true, bool isClose = true, bool isBlur = true);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 读取配置文件
    void initGsetting();

    void paintEvent(QPaintEvent *event);

    bool event(QEvent *event);

    // 设置标题栏文字
    void setTitleText(QString titleStr);

    // 跟随主题深浅模式
    void changeTheme();
    
    void slotChangeHScreenMode();   // 平板横屏
    void slotChangeVScreenMode();   // 平板竖屏
    void slotChangePCMode();   // 平板竖屏

    QPushButton *m_pIconBtn;        // 应用图标
    QLabel *m_pFuncLabel;           // 界面标识
    menuModule *m_menuBar;          // 菜单模块
    QPushButton *m_menuButton;      // 菜单按钮
    QPushButton *m_pMinimizeButton; // 最小化按钮
    QPushButton *m_pCloseButton;    // 关闭按钮
    UkuiWindowHelper *m_windowHelper;
    QColor m_backColor;

private:
    // 整体界面布局
    QHBoxLayout *m_pLayout;

    bool m_isMenu;
    bool m_isClose;
    bool m_isBlur;
    bool m_hasFocus = true;
    void setFocusFade(bool hasFocus);
protected:
#ifdef __V10__
    // 进行鼠标界面的拖动
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
#endif

private slots:

    // 进行置顶、最小化、关闭操作
    void onClicked();
    // 菜单按钮点击处理
    void onMenuButtonClicked();
};


#endif // TITLEBAR_H
