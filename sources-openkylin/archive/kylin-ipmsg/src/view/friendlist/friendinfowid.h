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

#ifndef FRIENDINFOWID_H
#define FRIENDINFOWID_H


#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyleOption>

#include "view/titlebar/titlebar.h"
#include "global/database/friendlistdb.h"

class FriendInfoWid : public QWidget
{
    Q_OBJECT
public:
    explicit FriendInfoWid(QWidget *parent = nullptr);
    ~FriendInfoWid();
    void changeFontSize(); // 改变字号大小

    // 好友头像
    QLabel *m_avatarLab;

    // 好友用户名
    QLabel *m_usernameLab;
    QLabel *m_usernameInfo;

    // 好友IP地址
    QLabel *m_ipAddrLab;
    QLabel *m_ipAddrInfo;

    // 好友备注
    QLabel *m_nicknameLab;
    QLabel *m_nicknameInfo;


    // 单例，初始化返回指针
    static FriendInfoWid *getInstance(QWidget *parent = nullptr);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 填充好友信息
    void fillFriendInfo(int friendId);

    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

    // 按Esc关闭弹窗
    virtual void keyPressEvent(QKeyEvent *event);
private:
    int m_friendId = 0;

    // 标题栏
    QPushButton *m_titleIcon;
    QLabel *m_titleLable;
    QPushButton *m_titleClose;

    // 修改好友备注按钮
    QPushButton *m_changeNicknameBtn;

    // 界面总体布局
    QVBoxLayout *m_mainLayout;

    // 好友信息界面网格布局
    QGridLayout *m_infoLayout;

    QHBoxLayout *m_avatarLayout;
    QHBoxLayout *m_nicknameLayout;
    QHBoxLayout *m_titleHLayout;

    void initGsetting();                 //监听主题变化
    void transChange();                  //控制面板变化
    void paintEvent(QPaintEvent *event); // 解决毛玻璃特效的问题

    // 更新好友信息
    void slotUpdateFriendInfo(int friendId);

    // 修改好友备注
    void changeNickname();
    void setBackgroundColor(); // 高亮色的切换

private slots:
    void slotChangeFont();  //控件的字体进行更换；

    friend class TestFriendInfoWid;
};




#endif // FRIENDINFOWID_H
