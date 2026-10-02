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

#ifndef TITLESETING_H
#define TITLESETING_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "titlebar.h"

class TitleSeting : public QWidget
{
    Q_OBJECT
    friend class TestTitleSeting;
public:
    explicit TitleSeting(QWidget *parent = nullptr);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    QLabel *m_labelCatalogue;      //目录名称
    QLabel *m_labelTip;            //提示
    QLineEdit *m_lineRoute;        //路径
    QPushButton *m_chooseRouteBtn; //选择路径
    QPushButton *m_clearChatBtn;   //清空聊天记录
    QPushButton *m_clearCacheBtn;  //清空缓存

    QDialog *m_tipWidget; //提示框
    QLabel *m_tipWidText; //提示框内的信息
    QLabel *m_tipWidIcon; //提示图标

    TitleBar *m_titleBar;

    QVBoxLayout *m_mainLayout;     // 设置页面标题布局
    QVBoxLayout *m_vControlLayout; // 总体控件纵向布局
    QHBoxLayout *m_hControlLayout; // 总体控件横向布局
    QVBoxLayout *m_vTipWidLayout;  // 提示弹窗纵向布局
    QHBoxLayout *m_hTipWidLayout;  // 提示弹窗横向布局

    void setTipWid(QString text); // 初始化弹窗
    virtual void keyPressEvent(QKeyEvent *event); // 按Esc关闭弹窗
private:
    /* 2021-12-15 添加 , 用于处理 bashrc 安全漏洞问题 - 吉圣杰 */
    /* 限定目录只能在 家目录下的一个目录下边 */
    bool checkSavePath(QString path);

signals:

private slots:
    void updateRoute();          /* 修改目录 */
    void clearChat();            /* 清空聊天信息 */
    void clearCache();           /* 清空缓存 */
    bool getPower(QString path); /* 获取目录权限 */
    void timeDestroy();          /* 销毁弹窗 */
    void slotChangeFont();
};


#endif // TITLESETING_H
